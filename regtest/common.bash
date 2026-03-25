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
    TMPDIR=/var/tmp
fi

LOGDIR="/dev/shm"
if ! test -d $LOGDIR; then
    LOGDIR=/tmp
fi

# These are defaults; try() overrides them per-test for isolation
LOGFILE="$LOGDIR/log.txt"
DIFFLOG="$LOGDIR/difflog.txt"
NEWLOG="$LOGDIR/newlog.txt"
TMPLOG="$LOGDIR/tmplog.txt"

UNAME="$(uname -s)"

if [ "$UNAME" = Darwin ]; then
    MD5SUM="md5 -r"
else
    MD5SUM=md5sum
fi

if ! $MD5SUM $RNDSEQ >/dev/null 2>&1; then
    MD5SUM=../simple-md5sum
fi

nbfailed=0

# Parallel execution settings
MAX_JOBS=${MAX_JOBS:-$(nproc 2>/dev/null || echo 4)}
RESULTS_DIR=$(mktemp -d "${TMPDIR}/regtest_results_XXXXXX")
OUTPUT_LOCK="$RESULTS_DIR/.output.lock"
touch "$OUTPUT_LOCK"

# Shared directory for interactive diff data (survives per-codec cleanup)
INTERACTIVE_DIR="${TMPDIR}/regtest_interactive"
mkdir -p "$INTERACTIVE_DIR"

# Clean up on exit (handles interrupted runs leaving leftover dirs)
# Uses CODEC_PREFIX which is set after sourcing, so each script only
# cleans up its own temp dirs (no interference between parallel scripts)
function _regtest_cleanup() {
    # Wait for any background jobs to finish
    wait 2>/dev/null
    rm -rf "$RESULTS_DIR" 2>/dev/null
    if [ -n "$CODEC_PREFIX" ]; then
	rm -rf "$TMPDIR/regtest_${CODEC_PREFIX}" 2>/dev/null
    fi
}
trap _regtest_cleanup EXIT

# Call after setting CODEC_PREFIX to clean up leftovers from interrupted runs
function init_regtest_cleanup() {
    rm -rf "$TMPDIR/regtest_${CODEC_PREFIX}" 2>/dev/null
}

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

# Locked output: serialize printing across parallel subshells

function locked_print()
{
    {
	flock -x 200
	printf "%s\n" "$*"
    } 200>>"$OUTPUT_LOCK"
}

function locked_print_n()
{
    {
	flock -x 200
	printf "%s" "$*"
    } 200>>"$OUTPUT_LOCK"
}

# Print a block of output atomically (reads from stdin)

function locked_cat()
{
    {
	flock -x 200
	cat
    } 200>>"$OUTPUT_LOCK"
}

# Job control for parallel execution

function limit_jobs()
{
    while [ "$(jobs -rp | wc -l)" -ge "$MAX_JOBS" ]; do
	wait -n 2>/dev/null || sleep 0.1
    done
}

# Collect results from all parallel tests and tally failures

function collect_results()
{
    wait

    nbfailed=0
    for f in "$RESULTS_DIR"/${CODEC_PREFIX}_*.status; do
	[ -f "$f" ] || continue
	status=$(cat "$f")
	if [ "$status" != "0" ]; then
	    nbfailed=$((nbfailed + 1))
	fi
    done

    # Handle interactive diffs when running a codec script directly.
    # When run from runtests.sh, REGTEST_RUNNER is set and interactive
    # diffs are deferred to runtests.sh after all codec scripts finish.
    if test -z "$REGTEST_RUNNER"; then
	. "$(dirname "${BASH_SOURCE[0]}")/interactive-diff.bash"
	_handle_interactive_diffs
	local accepted=$?
	nbfailed=$((nbfailed - accepted))
	[ $nbfailed -lt 0 ] && nbfailed=0
	rm -rf "$INTERACTIVE_DIR"
    fi

    [ $nbfailed -ge 256 ] && nbfailed=255

    # Warn about out-of-space issues that may have caused bogus failures
    if [ -f "$RESULTS_DIR/.oom_space" ]; then
	local oom_count
	oom_count=$(wc -l < "$RESULTS_DIR/.oom_space")
	echo ""
	echo "*** WARNING: $oom_count test(s) failed while the temp filesystem was full:"
	sed 's/^/***   /' "$RESULTS_DIR/.oom_space"
	echo "*** These failures are likely caused by out-of-space, not actual bugs."
	echo "*** Try re-running with a lower MAX_JOBS (current: $MAX_JOBS), e.g.:"
	echo "***   MAX_JOBS=1 $0 $*"
	echo ""
    fi

    # Clean up per-test temp directories (only this codec's)
    rm -rf "$TMPDIR/regtest_${CODEC_PREFIX}" 2>/dev/null
    rm -rf "$RESULTS_DIR"
}

# See if a specific test should be performed

function try()
{  local doit=$(grep "${CODEC_PREFIX}_$2 " $CONFIGFILE)
   if [[ $OS =~ Windows ]] && test -e "$CONFIGFILE_WIN" && grep -q "${CODEC_PREFIX}_$2 " "$CONFIGFILE_WIN"; then
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
           collect_results
           exit $nbfailed
       else
           only_this_one="done_please_exit"
       fi
   fi

   doit=$(echo $doit | cut -d\  -f 2)

   if test $doall = "yes" || test $doit = "yes" || test $only_this_one != "false"; then
       # Create per-test temp directory for isolation
       # Each codec script uses its own subdir to avoid interference
       TEST_TMPDIR="$TMPDIR/regtest_${CODEC_PREFIX}/$2"
       mkdir -p "$TEST_TMPDIR"

       # Set per-test file paths (preserve original basenames)
       local orig_tmpiso="$TMPISO"
       local orig_tmpecc="$TMPECC"
       local orig_simiso="$SIMISO"
       TMPISO="$TEST_TMPDIR/$(basename "$orig_tmpiso")"
       TMPECC="$TEST_TMPDIR/$(basename "$orig_tmpecc")"
       SIMISO="$TEST_TMPDIR/$(basename "$orig_simiso")"

       # Per-test log files
       LOGFILE="$TEST_TMPDIR/log.txt"
       NEWLOG="$TEST_TMPDIR/newlog.txt"
       DIFFLOG="$TEST_TMPDIR/difflog.txt"
       TMPLOG="$TEST_TMPDIR/tmplog.txt"

       if test -z "$REGTEST_SECTION"; then
	   REGTEST_SECTION="Test"
       fi

       # Save test header for run_regtest output
       TEST_HEADER="${CODEC_PREFIX} - ${REGTEST_SECTION} - $1"

       # Print started notification
       locked_print ">> ${TEST_HEADER}"

       return 0
   else
       if [ "$REGTEST_NO_UTF8" != 1 ]; then
           locked_print_n "[-] "
       fi
       locked_print "${CODEC_PREFIX} - ${REGTEST_SECTION} - $1 - SKIPPED ($doit, ${CODEC_PREFIX}_$2)"
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
   local output=""

   local fail_on_bad=$(grep "FAIL_ON_BAD" $CONFIGFILE)
   fail_on_bad=$(echo $fail_on_bad | cut -d\  -f 2)

   local spawn_log_window=$(grep "SPAWN_LOG_WINDOW" $CONFIGFILE)
   spawn_log_window=$(echo $spawn_log_window | cut -d\  -f 2)

   local interactive_diff
   interactive_diff=$(grep "INTERACTIVE_DIFF" $CONFIGFILE)
   interactive_diff=$(echo $interactive_diff | cut -d\  -f 2)

   if test -z "$testecc"; then
       echo -e "broken test case $1\n--> run_regtest: 4 arguments required to ensure deterministic test behaviour."
       exit 1
   fi

   if test -n "${testecc}"; then
       testeccopt="-e ${testecc}"
   fi

   REFLOG="${DATABASE}/${CODEC_PREFIX}_${testsymbol}"
   if [ "$UNAME" = "Darwin" ] && [ -f "$REFLOG.darwin" ]; then
      REFLOG="$REFLOG.darwin"
   elif [[ $OS =~ Windows ]] && [ -f "$REFLOG.win" ]; then
      REFLOG="$REFLOG.win"
   fi

   if test "$gui_mode" == "false"; then
      rm -f $NEWLOG

      echo "LANG=en_EN.UTF-8 $NEWVER --regtest --no-progress -i${testiso} ${testeccopt} ${extra_args} ${testparms}" >>$LOGFILE
      LANG=en_EN.UTF-8 $NEWVER --regtest --no-progress -i${testiso} ${testeccopt} ${extra_args} ${testparms} 2>&1 | tail -n +4  >>$NEWLOG

      if ! test -r $REFLOG; then
          pass="false"
          output="BAD; '$REFLOG' is missing in log file database"
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

         # for Windows, just remove any path we find:
         sed -i -re "s=[A-Z]:/[A-Za-z0-9_/-]+/==g" $NEWLOG

         # remove per-test temp dir, then general temp/iso dirs for reproducible output:
         if test -n "$TEST_TMPDIR"; then
             sed -i -re "s=${TEST_TMPDIR}/==g" $NEWLOG
         fi
         sed -i -re "s=$TMPDIR/*==g;s=$ISODIR/*==g;s=regtest/==g" $NEWLOG

         # remote tmp path of github actions
         sed -i -re "s=[-A-Za-z0-9_~]+/AppData/Local/Temp/==g" $NEWLOG

         if ! diff <(tail -n +3 $REFLOG | $filter) <(cat $NEWLOG | $filter) >${DIFFLOG}; then
            output="BAD; diffs found (<expected; >created):"
            pass="false"

            if test "$interactive_diff" == "yes"; then
               # Copy logs into INTERACTIVE_DIR so they survive all cleanup
               local saved_newlog="$INTERACTIVE_DIR/${CODEC_PREFIX}_${testsymbol}.newlog"
               local saved_difflog="$INTERACTIVE_DIR/${CODEC_PREFIX}_${testsymbol}.difflog"
               cp "$NEWLOG" "$saved_newlog"
               cp "$DIFFLOG" "$saved_difflog"
               printf "%s\n%s\n%s\n%s\n" \
                  "${CODEC_PREFIX}_${testsymbol}" "$REFLOG" "$saved_newlog" "$saved_difflog" \
                  > "$INTERACTIVE_DIR/${CODEC_PREFIX}_${testsymbol}.interactive"
               pass="interactive"
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
	   output="${output:+${output}
}BAD; md5 sum mismatch in image file:
... expected  image: $image_md5
... generated image: $md5"
	   pass="false"
       fi
   fi

   if test "${ecc_md5}" != "ignore"; then
       md5=$($MD5SUM ${testecc} | cut -d\  -f 1)
       if test "$ecc_md5" != "$md5"; then
	   output="${output:+${output}
}BAD; md5 sum mismatch in ecc file:
... expected  ecc: $ecc_md5
... generated ecc: $md5"
	   pass="false"
       fi
   fi

   # Print result atomically
   case "${pass}" in
     true)
      if [ "$REGTEST_NO_UTF8" = 1 ]; then
        locked_print "${TEST_HEADER} - GOOD"
      else
        locked_print "[✓] ${TEST_HEADER} - GOOD"
      fi
      echo "0" > "$RESULTS_DIR/${CODEC_PREFIX}_${testsymbol}.status"
      ;;

     skip)
      echo "0" > "$RESULTS_DIR/${CODEC_PREFIX}_${testsymbol}.status"
      ;;

     interactive)
      # Deferred to main shell via .interactive file; mark as failed for now
      {
	flock -x 200
	printf "[\e[33m?\e[0m] %s - PENDING INTERACTIVE REVIEW\n" "${TEST_HEADER}"
	if test -n "$output"; then
	    echo "$output"
	fi
	if test -f "${DIFFLOG}" && test -s "${DIFFLOG}"; then
	    cat "${DIFFLOG}"
	fi
      } 200>>"$OUTPUT_LOCK"
      echo "1" > "$RESULTS_DIR/${CODEC_PREFIX}_${testsymbol}.status"
      ;;

     *)
      {
	flock -x 200
	if [ "$REGTEST_NO_UTF8" = 1 ]; then
	    echo "[✘] ${TEST_HEADER} - FAILED"
	else
	    printf "[\e[31m✘\e[0m] %s - FAILED\n" "${TEST_HEADER}"
	fi
	if test -n "$output"; then
	    echo "$output"
	fi
	if test -f "${DIFFLOG}" && test -s "${DIFFLOG}"; then
	    cat "${DIFFLOG}"
	fi
	echo "test symbol for config: $testsymbol"
      } 200>>"$OUTPUT_LOCK"
      echo "1" > "$RESULTS_DIR/${CODEC_PREFIX}_${testsymbol}.status"
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
     locked_print "$FILE_MSG"
     FILE_MSG=""
   fi

   # Check for out-of-space before cleaning up, so we can warn the user
   # that failures may be caused by disk exhaustion rather than real bugs.
   if test "$pass" != "true" && test "$pass" != "skip" \
       && test -n "$TEST_TMPDIR" && test -d "$TEST_TMPDIR"; then
       local avail_kb
       avail_kb=$(df -k "$TEST_TMPDIR" 2>/dev/null | awk 'NR==2 {print $4}')
       if test -n "$avail_kb" && test "$avail_kb" -lt 1024 2>/dev/null; then
           echo "${CODEC_PREFIX}_${testsymbol}" >> "$RESULTS_DIR/.oom_space"
           locked_print "  *** OUT OF SPACE on $(df "$TEST_TMPDIR" 2>/dev/null | awk 'NR==2 {print $6}') — this failure is likely bogus"
       fi
   fi

   # Clean up per-test temp files (ISOs, ecc files) to avoid filling up tmpdir.
   # Results are already stored in $RESULTS_DIR, not $TEST_TMPDIR.
   if test -n "$TEST_TMPDIR" && test -d "$TEST_TMPDIR"; then
       rm -rf "$TEST_TMPDIR"
   fi
}
