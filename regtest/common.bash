CONFIGFILE="./config.txt"
NEWVER=../dvdisaster
SETVERSION="0.80"

DATABASE=./database
RNDSEQ="./fixed-random-sequence"

ISODIR=/var/tmp/regtest
if ! test -d $ISODIR; then
    echo "$ISODIR does not exist."
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

# Assemble sed expressions for removal of variable output contents

SED_REMOVE_ISO_DIR=$(echo "${ISODIR}/" | sed -e "s/\//\\\\\//g")
SED_REMOVE_DEV_SHM=$(echo "/dev/shm/"  | sed -e "s/\//\\\\\//g")

# Usage

if test "$1" == "--help" || test "$1" == "-h"; then
    echo "Usage: $0 [gui] [all|cont <test case>]"
    exit 0;
fi

doall="no"
cont_at="false"
gui_mode="false"

param=($*)

case "${param[0]}" in
    gui)
	gui_mode="true"
	param[0]="${param[1]}"
	param[1]="${param[2]}"
	;;
esac
	
case "${param[0]}" in
    all)
	doall="yes"
	;;
    cont)
	doall="yes"
	cont_at="${param[1]}"
	;;
esac

# Sanity check

echo -n "Checking for $NEWVER: "
if test -x $NEWVER; then
    echo "OK"
else
    echo "missing."
    exit 0
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

   if test -z "$doit"; then
       echo "Config for ${CODEC_PREFIX}_$2 missing"
       exit 1
   fi

   if test "$cont_at" != "false" && test "$cont_at" != "${CODEC_PREFIX}_$2"; then
       return 1
   else
       cont_at="false"
   fi
   
   doit=$(echo $doit | cut -d\  -f 2) 

   if test $doall = "yes" || test $doit = "yes"; then
       echo -n "Test case: $1 - "
       return 0
   else
#       echo "Skipping: $1 ($doit, ${CODEC_PREFIX}_$2)"
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
   local testeccopt=""
   local image_md5=""
   local ecc_md5=""
   local pass="true"

   local fail_on_bad=$(grep "FAIL_ON_BAD" $CONFIGFILE)
   fail_on_bad=$(echo $fail_on_bad | cut -d\  -f 2) 

   local spawn_log_window=$(grep "SPAWN_LOG_WINDOW" $CONFIGFILE)
   spawn_log_window=$(echo $spawn_log_window | cut -d\  -f 2) 

   if test -n "${testecc}"; then
       testeccopt="-e ${testecc}"
   fi

   REFLOG=${DATABASE}/${CODEC_PREFIX}_${testsymbol}

   if test "$gui_mode" == "false"; then
     rm -f $NEWLOG

     echo "LANG=en_EN.UTF-8 $NEWVER -i${testiso} ${testeccopt} ${extra_args} ${testparms}" >>$LOGFILE 
     LANG=en_EN.UTF-8 $NEWVER -i${testiso} ${testeccopt} ${extra_args} ${testparms} 2>&1 | tail -n +3  >>$NEWLOG 

     if ! test -r $REFLOG; then
	 echo -e "FAIL\n$REFLOG missing in log file database"
	 return
     fi

     # ignore the memory tracker line when no memory leaks
     # have been found
     
     grep -v "dvdisaster: No memory leaks found." $NEWLOG >$TMPLOG
     mv $TMPLOG $NEWLOG
     
     # ignore log lines specified by user
     
     if test -n "$IGNORE_LOG_LINE"; then
	 egrep -v "$IGNORE_LOG_LINE" $NEWLOG >$TMPLOG
	 mv $TMPLOG $NEWLOG
     fi
       
     if ! diff <(tail -n +3 $REFLOG) <(sed -e "s/${SED_REMOVE_ISO_DIR}//g" $NEWLOG | sed -e "s/${SED_REMOVE_DEV_SHM}//g") >${DIFFLOG}; then
	 echo "BAD; diffs found:"
	 cat ${DIFFLOG}
	 pass="false"
     fi
   else  # gui mode
       replace_config last-image "$testiso"
       if test -n "${testecc}"; then
           replace_config last-ecc "$testecc"
       fi
       
       if test "$spawn_log_window" == "yes"; then
	   echo LANG=en_EN.UTF-8 $NEWVER $extra_args --resource-file $LOGDIR/.dvdisaster-regtest >$NEWLOG
	   xterm -geometry +0+0 -e tail -n 50 -f $NEWLOG &
	   xterm_pid=$!
       fi

       LANG=en_EN.UTF-8 $NEWVER $extra_args --resource-file $LOGDIR/.dvdisaster-regtest >>$NEWLOG 2>&1
       rm -f $LOGDIR/.dvdisaster-regtest
   fi

   unset extra_args
     
   image_md5=$(head -n 1 $REFLOG)
   ecc_md5=$(head -n 2 $REFLOG | tail -n 1)

   if test ${image_md5} != "ignore"; then
       md5=$($MD5SUM ${testiso} | cut -d\  -f 1)
       if test "$image_md5" != "$md5"; then
	   echo "BAD; md5 sum mismatch in image file:"
	   echo "... expected  image: $image_md5"
	   echo "... generated image: $md5"
	   pass="false"
       fi
   fi	   

   if test ${ecc_md5} != "ignore"; then
       md5=$($MD5SUM ${testecc} | cut -d\  -f 1)
       if test "$ecc_md5" != "$md5"; then
	   echo "BAD; md5 sum mismatch in ecc file:"
	   echo "... expected  ecc: $ecc_md5"
	   echo "... generated ecc: $md5"
	   pass="false"
       fi
   fi	   
   
   if test ${pass} == "true"; then
      echo "GOOD"
   else
      echo "test symbol for config: $testsymbol"
      if test "$fail_on_bad" == "yes"; then
	next=$(grep -A 1  ${CODEC_PREFIX}_$testsymbol config.txt | tail -n 1 | cut -d\  -f 1)
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
   fi

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
