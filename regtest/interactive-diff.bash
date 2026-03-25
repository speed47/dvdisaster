# Handle interactive diffs that were deferred from subshells.
# Called after all tests have finished, in a process with tty access.
# Requires: INTERACTIVE_DIR, LOGDIR
#
# Usage: _handle_interactive_diffs
# Returns: number of accepted diffs (via return code, capped at 255)

function _handle_interactive_diffs()
{
    local accepted=0

    for ifile in "$INTERACTIVE_DIR"/*.interactive; do
	[ -f "$ifile" ] || continue
	local symbol reflog newlog difflog
	symbol=$(sed -n '1p' "$ifile")
	reflog=$(sed -n '2p' "$ifile")
	newlog=$(sed -n '3p' "$ifile")
	difflog=$(sed -n '4p' "$ifile")

	echo ""
	echo "Interactive diff for: $symbol"
	if [ -f "$difflog" ]; then
	    cat "$difflog"
	fi

	while true; do
	    read -n 1 -p ">> Press 'a' to accept this diff; 'i' to ignore; 'v' to vimdiff; any other key to fail this test:" -e answer
	    if test "$answer" == "a"; then
		cp "$reflog" "$LOGDIR"
		head -n 2 "$LOGDIR/$(basename "$reflog")" >"$reflog"
		cat "$newlog" >>"$reflog"
		accepted=$((accepted + 1))
	    elif test "$answer" == "v"; then
		vimdiff "$reflog" "$newlog"
		continue
	    fi
	    break
	done
	rm -f "$ifile" "${ifile%.interactive}.newlog" "${ifile%.interactive}.difflog"
    done

    [ $accepted -ge 256 ] && accepted=255
    return $accepted
}
