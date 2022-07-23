CONFIGFILE="./config.txt"
CONFIGFILE_WIN="./config-win.txt"
NEWVER=../dvdisaster
SETVERSION="0.80"

DATABASE=./database
RNDSEQ="./fixed-random-sequence"

NON_EXISTENT_DEVICE=/dev/sdz

# directory for permanently storing test files
ISODIR=/var/tmp/regtest
if ! test -d $ISODIR; then
    echo "$ISODIR does not exist."
    echo "Please create it manually, or edit common.bash"
    exit 1
fi

# directory for temporary files; e.g. to keep them away from SSDs
TMPDIR=/dev/shm
if ! test -d $TMPDIR; then
    echo "$TMPDIR does not exist."
    echo "Please create it manually, or edit common.bash"
    exit 0
fi

LOGDIR="/dev/shm"
if ! test -d $LOGDIR; then
    LOGDIR=/tmp
fi
LOGFILE="$LOGDIR/log.txt"
DIFFLOG="$LOGDIR/difflog.txt"
NEWLOG="$LOGDIR/newlog.txt"
TMPLOG="$LOGDIR/tmplog.txt"

MD5SUM=md5sum
if ! $MD5SUM $RNDSEQ >/dev/null 2>&1; then
    MD5SUM=../simple-md5sum
fi

nbfailed=0

# For MSYS2

if [ -n "$ORIGINAL_TEMP" ]; then
    ISODIR="$ORIGINAL_TEMP"
    # /c/ => C:/
    NON_EXISTENT_DEVICE=V:
fi

# Usage

if test "$1" == "--help" || test "$1" == "-h"; then
    echo "Usage: $0 [gui] [all|[cont|only] <test case>]"
    exit 1;
fi

doall="no"
cont_at="false"
only_this_one="false"
gui_mode="false"

param=($*)

case "${param[0]}" in
    gui)
	gui_mode="true"
	param[0]="${param[1]}"
	param[1]="${param[2]}"
	param[2]="${param[3]}"
	;;
esac
	
case "${param[0]}" in
    all)
	doall="yes"
	;;
    cont)
	cont_at="${param[1]}"
	;;
    only)
	only_this_one="${param[1]}"
	;;
esac

# Sanity check

echo -n "Checking for $NEWVER: "
if test -x $NEWVER; then
    echo "OK"
else
    echo "missing."
    exit 1
fi

### Helper functions

# See if file needs to be created

function file_exists()
{
    if test -f $1; then
	return 0
    fi

    if test -n "${FILE_MSG}"; then
	FILE_MSG="$FILE_MSG\n  (file $1 was created)"
    else
	FILE_MSG="  (file $1 was created)"
    fi
    return 1
}

# See if a specific test should be performed

function try()
{  local doit=$(grep "${CODEC_PREFIX}_$2 " $CONFIGFILE)
   if echo "$OS" | grep -q Windows && test -e "$CONFIGFILE_WIN" && grep -q "${CODEC_PREFIX}_$2 " "$CONFIGFILE_WIN"; then
       doit=$(grep "${CODEC_PREFIX}_$2 " "$CONFIGFILE_WIN")
   fi

   if test -z "$doit"; then
       echo "Config for ${CODEC_PREFIX}_$2 missing"
       exit 1
   fi

   if test "$cont_at" != "false" && test "$cont_at" != "${CODEC_PREFIX}_$2"; then
       return 1
   else
       cont_at="false"
   fi

   if test "$only_this_one" != "false"; then
       if test "$only_this_one" != "${CODEC_PREFIX}_$2"; then
           return 1
       elif test "$only_this_one" == "done_please_exit"; then
           exit $nbfailed
       else
           only_this_one="done_please_exit"
       fi
   fi

   doit=$(echo $doit | cut -d\  -f 2) 

   if test $doall = "yes" || test $doit = "yes" || test $only_this_one != "false"; then
       # Clean up temporary files
       if test -n "$TMPISO" && test -f "$TMPISO"; then
	   rm -f $TMPISO
       fi
       if test -n "$TMPECC" && test -f "$TMPECC"; then
	   rm -f $TMPECC
       fi
       if test -n "$SIMISO" && test -f "$SIMISO"; then
	   rm -f $SIMISO
       fi
       
       if test -z "$REGTEST_SECTION"; then
	   REGTEST_SECTION="Test"
       fi

       if [ "$REGTEST_NO_UTF8" != 1 ]; then
           echo -n "[ ] "
       fi
       echo -n "${CODEC_PREFIX} - ${REGTEST_SECTION} - $1 - "
       return 0
   else
       if [ "$REGTEST_NO_UTF8" != 1 ]; then
           echo -n "[-] "
       fi
       echo "${CODEC_PREFIX} - ${REGTEST_SECTION} - $1 - SKIPPED ($doit, ${CODEC_PREFIX}_$2)"
       return 1
   fi
}

# Change the configuration file for the GUI mode

function replace_config()
{  local attribute="$1"
   local value=$(echo "$2" | sed -e "s/\//\\\\\//g")

   if test "$gui_mode" == "false"; then
       return
   fi

   if ! test -f $LOGDIR/.dvdisaster-regtest; then
      cp .dvdisaster-default $LOGDIR/.dvdisaster-regtest
   fi
   
   cp $LOGDIR/.dvdisaster-regtest $LOGDIR/.dvdisaster-old
   sed -e "s/${attribute}:[-_ a-zA-Z0-9\/\.]*/${attribute}: $value/" <$LOGDIR/.dvdisaster-old >$LOGDIR/.dvdisaster-regtest
}

# Perform test and compare results with database

function run_regtest()
{  local testsymbol="$1"
   local testparms="$2"
   local testiso="$3"
   local testecc="$4"
   local options="$5"
   local testeccopt=""
   local image_md5=""
   local ecc_md5=""
   local pass="false"

   local fail_on_bad=$(grep "FAIL_ON_BAD" $CONFIGFILE)
   fail_on_bad=$(echo $fail_on_bad | cut -d\  -f 2) 

   local spawn_log_window=$(grep "SPAWN_LOG_WINDOW" $CONFIGFILE)
   spawn_log_window=$(echo $spawn_log_window | cut -d\  -f 2) 

   local interactive_diff=$(grep "INTERACTIVE_DIFF" $CONFIGFILE)
   interactive_diff=$(echo $interactive_diff | cut -d\  -f 2) 

   if test -z "$testecc"; then
       echo -e "broken test case $1\n--> run_regtest: 4 arguments required to ensure deterministic test behaviour."
       exit 1
   fi
   
   if test -n "${testecc}"; then
       testeccopt="-e ${testecc}"
   fi

   REFLOG=${DATABASE}/${CODEC_PREFIX}_${testsymbol}

   if test "$gui_mode" == "false"; then
      rm -f $NEWLOG

      echo "LANG=en_EN.UTF-8 $NEWVER --regtest --no-progress -i${testiso} ${testeccopt} ${extra_args} ${testparms}" >>$LOGFILE 
      LANG=en_EN.UTF-8 $NEWVER --regtest --no-progress -i${testiso} ${testeccopt} ${extra_args} ${testparms} 2>&1 | tail -n +4  >>$NEWLOG 

      if ! test -r $REFLOG; then
          pass="false"
          if [ "$REGTEST_NO_UTF8" = 1 ]; then
             echo "BAD; '$REFLOG' is missing in log file database"
          else
             printf "%b\r%b\n" "BAD; '$REFLOG' is missing in log file database" "[\e[31m✘\e[0m]"
          fi
      else
         # ignore the memory tracker line when no memory leaks
         # have been found

         grep -va "dvdisaster: No memory leaks found." $NEWLOG >$TMPLOG
         mv $TMPLOG $NEWLOG

         # ignore log lines specified by user

         if test -n "$IGNORE_LOG_LINE"; then
            grep -Eva "$IGNORE_LOG_LINE" $NEWLOG >$TMPLOG
            mv $TMPLOG $NEWLOG
         fi

         filter=cat
         echo "$options" | grep -qw SORTED && filter=sort

         # for Windows
         if [[ $testsymbol =~ _no_device$ ]]; then
            sed -i -re "s=device $NON_EXISTENT_DEVICE\.=/dev/sdz: No such file or directory=" $NEWLOG
         fi

         # for Windows, just remove any path we find:
         sed -i -re "s=[A-Z]:/[A-Za-z0-9_/-]+/==g" $NEWLOG

         # remove all paths to get reproducible output:
         sed -i -re "s=$TMPDIR/*==g;s=$ISODIR/*==g" $NEWLOG

         # remote tmp path of github actions
         sed -i -re "s=[-A-Za-z0-9_~]+/AppData/Local/Temp/==g" $NEWLOG

         if ! diff <(tail -n +3 $REFLOG | $filter) <(cat $NEWLOG | $filter) >${DIFFLOG}; then
            if [ "$REGTEST_NO_UTF8" = 1 ]; then
               echo "BAD; diffs found (<expected; >created):"
            else
               printf "%b\r%b\n" "BAD; diffs found (<expected; >created):" "[\e[31m✘\e[0m]"
            fi
            cat ${DIFFLOG}

            if test "$interactive_diff" == "yes"; then
               while true; do
                  read -n 1 -p ">> Press 'a' to accept this diff; 'i' to ignore; 'v' to vimdiff; any other key to fail this test:" -e answer
                  if test "$answer" == "a"; then
                     cp $REFLOG $LOGDIR
                     head -n 2 $LOGDIR/${CODEC_PREFIX}_${testsymbol} >$REFLOG 
                     cat $NEWLOG >>$REFLOG
                     pass="skip"
                  elif test "$answer" == "v"; then
                     vimdiff $REFLOG $NEWLOG
                     continue
                  else
		     if test "$answer" == "i"; then
			   pass="skip"
		     else
			   pass="false"
		     fi
                  fi
                  break
               done
            fi
         else
            pass="true"
         fi
      fi
   else  # gui mode
       replace_config last-image "$testiso"
       if test -n "${testecc}"; then
           replace_config last-ecc "$testecc"
       fi
       
       if test "$spawn_log_window" == "yes"; then
	   echo LANG=en_EN.UTF-8 $NEWVER --regtest $extra_args --resource-file $LOGDIR/.dvdisaster-regtest >$NEWLOG
	   xterm -geometry +0+0 -e tail -n 50 -f $NEWLOG &
	   xterm_pid=$!
       fi

       LANG=en_EN.UTF-8 timeout 15 $NEWVER --regtest $extra_args --resource-file $LOGDIR/.dvdisaster-regtest >>$NEWLOG 2>&1
       rm -f $LOGDIR/.dvdisaster-regtest
   fi

   unset extra_args
     
   if test -r "$REFLOG"; then
      image_md5=$(head -n 1 $REFLOG)
      ecc_md5=$(head -n 2 $REFLOG | tail -n 1)
   else
      image_md5=ignore
      ecc_md5=ignore
   fi

   if test "${image_md5}" != "ignore"; then
       md5=$($MD5SUM ${testiso} | cut -d\  -f 1)
       if test "$image_md5" != "$md5"; then
	   if [ "$REGTEST_NO_UTF8" = 1 ]; then
	      echo "BAD; md5 sum mismatch in image file:"
	   else
	      printf "%b\r%b\n" "BAD; md5 sum mismatch in image file:" "[\e[31m✘\e[0m]"
	   fi
	   echo "... expected  image: $image_md5"
	   echo "... generated image: $md5"
	   pass="false"
       fi
   fi	   

   if test "${ecc_md5}" != "ignore"; then
       md5=$($MD5SUM ${testecc} | cut -d\  -f 1)
       if test "$ecc_md5" != "$md5"; then
	   if [ "$pass" = false ] || [ "$REGTEST_NO_UTF8" = 1 ]; then
	      echo "BAD; md5 sum mismatch in ecc file:"
	   else
	      printf "%b\r%b\n" "BAD; md5 sum mismatch in ecc file:" "[\e[31m✘\e[0m]"
	   fi
	   echo "... expected  ecc: $ecc_md5"
	   echo "... generated ecc: $md5"
	   pass="false"
       fi
   fi	   

   case "${pass}" in
     true)
      if [ "$REGTEST_NO_UTF8" = 1 ]; then
        echo GOOD
      else
        printf "%b\r%b\n" "GOOD" "[\e[32m✓\e[0m]"
      fi
      ;;
     
     skip)
      ;;
     
     *)
      nbfailed=$((nbfailed + 1))
      [ $nbfailed -ge 256 ] && nbfailed=255
      echo "test symbol for config: $testsymbol"
      if test "$fail_on_bad" == "yes"; then
	next=$(grep -A 1 "${CODEC_PREFIX}_$testsymbol" config.txt | tail -n 1 | cut -d\  -f 1)
	echo "FAIL_ON_BAD set to yes -- exiting"
	if test "$gui_mode" == "true"; then
	    guiarg="gui"
	fi
	echo "$0 $guiarg cont ${CODEC_PREFIX}_$testsymbol resumes tests here;"
	if test -n "$next"; then
	    echo "$0 $guiarg cont $next skips failed test."
	fi
	exit 1
      fi
      ;;
   esac

   if test "$gui_mode" == "true" && test "$spawn_log_window" == "yes"; then
       read -n 1 -p "Press q to quit; any other key to continue." -e answer
       kill $xterm_pid
       if test "$answer" == "q"; then
	   echo "$0 gui cont ${CODEC_PREFIX}_$testsymbol resumes this test."
	   exit 1
       fi
   fi
   
   if test -n "$FILE_MSG"; then
     echo -e "$FILE_MSG"
     FILE_MSG=""
   fi
}
