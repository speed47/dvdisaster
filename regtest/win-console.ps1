# Run GUI launches in a separate console, verify that the GUI is no longer
# attached to that console, then check it survives the probe process's exit.
param(
    [Parameter(Mandatory=$true)][string]$Dist,
    [switch]$Probe,
    [switch]$Direct,
    [switch]$WithImage,
    [switch]$Capture,
    [string]$Result
)
$ErrorActionPreference = 'Stop'
$Dist = (Resolve-Path $Dist).Path

if ($Probe) {
    $state = @{ LauncherPid = 0; ChildPid = 0; ConsolePids = @(); Ready = $false; Error = '' }
    try {
        # Exercise each output stream redirected independently in a real console.
        $stdout = "$Result.stdout"
        $stderr = "$Result.stderr"
        & "$Dist/dvdisaster.com" --version > $stdout
        if ($LASTEXITCODE -ne 0 -or (Get-Content -Raw $stdout) -notmatch 'dvdisaster') {
            throw 'Redirected version output was lost'
        }
        # Windows PowerShell treats native stderr as ErrorRecords. The error
        # here is intentional, so inspect it instead of terminating the probe.
        $ErrorActionPreference = 'Continue'
        & "$Dist/dvdisaster.com" --no-such-option 2> $stderr
        $optionExitCode = $LASTEXITCODE
        $ErrorActionPreference = 'Stop'
        if ($optionExitCode -ne 1 -or (Get-Content -Raw $stderr) -notmatch 'no-such-option') {
            throw 'Redirected option error was lost'
        }

        $name = if ($Direct) { 'dvdisaster.exe' } else { 'dvdisaster.com' }
        # Windows PowerShell's Start-Process -NoNewWindow can lose the process
        # handle and return a null ExitCode. Start through .NET to retain it,
        # inheriting this console and waiting only for the launcher itself.
        $launch = New-Object System.Diagnostics.ProcessStartInfo
        $launch.FileName = "$Dist/$name"
        $launch.UseShellExecute = $false
        $launch.CreateNoWindow = $false
        $launch.WorkingDirectory = $PWD.Path
        $launch.RedirectStandardOutput = $Capture.IsPresent
        $launch.RedirectStandardError = $Capture.IsPresent
        if ($WithImage) { $launch.Arguments = '-i handoff-test.iso' }
        $process = [System.Diagnostics.Process]::Start($launch)
        if ($Capture) {
            # Read concurrently, as a capturing shell does, but bound the EOF
            # wait so a GUI retaining pipe writers produces a useful failure.
            $capturedOutput = $process.StandardOutput.ReadToEndAsync()
            $capturedError = $process.StandardError.ReadToEndAsync()
        }
        if ($Direct) { $state.ChildPid = $process.Id }
        else { $state.LauncherPid = $process.Id }
        $state | ConvertTo-Json | Set-Content $Result

        if (!$Direct) {
            if (!$process.WaitForExit(30000)) { throw 'Launcher kept waiting for the GUI' }
            $launcherExitCode = $process.ExitCode
            if ($null -eq $launcherExitCode) { throw 'Launcher exit code is unavailable' }
            if ($launcherExitCode -ne 0) { throw "Launcher exited with $launcherExitCode" }
            if ($Capture) {
                if (!$capturedOutput.Wait(5000) -or !$capturedError.Wait(5000)) {
                    throw 'GUI kept inherited output pipes open after launcher exit'
                }
                if ($capturedOutput.Result -notmatch 'GUI detached from console') {
                    throw 'Captured output is missing the GUI detachment message'
                }
            }
            $children = @(Get-CimInstance Win32_Process -Filter "ParentProcessId = $($process.Id) AND Name = 'dvdisaster.exe'")
            if ($children.Count -ne 1) { throw "Expected one GUI child, got $($children.Count)" }
            $state.ChildPid = $children[0].ProcessId
            $state | ConvertTo-Json | Set-Content $Result
        }

        $gui = Get-Process -Id $state.ChildPid
        $deadline = (Get-Date).AddSeconds(30)
        do {
            $gui.Refresh()
            if ($gui.HasExited) { throw 'GUI exited during startup' }
            if ($gui.MainWindowHandle -ne 0) { break }
            Start-Sleep -Milliseconds 100
        } while ((Get-Date) -lt $deadline)
        if ($gui.MainWindowHandle -eq 0) { throw 'GUI did not create a window' }
        Add-Type -Name ConsoleProbe -Namespace Dvdisaster -MemberDefinition @'
        [System.Runtime.InteropServices.DllImport("kernel32.dll", SetLastError=true)]
        public static extern uint GetConsoleProcessList(
            [System.Runtime.InteropServices.Out] uint[] processList, uint processCount);
'@
        # A console window handle can be a message-only window in CI; posting
        # WM_CLOSE to it need not close the console. Check actual membership
        # instead, with this probe's PID as a positive control for the query.
        $consolePids = New-Object 'uint32[]' 16
        while ($true) {
            $count = [Dvdisaster.ConsoleProbe]::GetConsoleProcessList($consolePids, $consolePids.Length)
            if ($count -eq 0) {
                throw "Cannot query probe console (Windows error $([Runtime.InteropServices.Marshal]::GetLastWin32Error()))"
            }
            if ($count -le $consolePids.Length) { break }
            $consolePids = New-Object 'uint32[]' $count
        }
        $state.ConsolePids = @($consolePids[0..($count - 1)])
        if ($state.ConsolePids -notcontains $PID) { throw 'Probe is missing from its own console process list' }
        if ($state.ConsolePids -contains $state.ChildPid) { throw 'GUI is still attached to the probe console' }
        $state.Ready = $true
    } catch {
        $state.Error = $_.ToString()
    }
    $state | ConvertTo-Json | Set-Content $Result
    # Membership was checked before exiting, so a GUI accidentally keeping
    # this console alive cannot turn the survival check into a false pass.
    exit 0
}

$temporary = Join-Path ([IO.Path]::GetTempPath()) ([IO.Path]::GetRandomFileName())
New-Item -ItemType Directory $temporary | Out-Null
try {
    $cases = @(
        @{ Direct = $false; Image = $false; Capture = $false },
        @{ Direct = $false; Image = $true;  Capture = $false },
        @{ Direct = $false; Image = $false; Capture = $true },
        @{ Direct = $false; Image = $true;  Capture = $true },
        @{ Direct = $true;  Image = $false; Capture = $false },
        @{ Direct = $true;  Image = $true;  Capture = $false }
    )
    foreach ($case in $cases) {
        $caseDescription = "direct=$($case.Direct) image=$($case.Image) capture=$($case.Capture)"
        Write-Host "Checking GUI console independence: $caseDescription"
        $resultFile = Join-Path $temporary 'result.json'
        Remove-Item $resultFile -ErrorAction SilentlyContinue
        $arguments = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`"",
                       '-Probe', '-Dist', "`"$Dist`"", '-Result', "`"$resultFile`"")
        if ($case.Direct) { $arguments += '-Direct' }
        if ($case.Image) { $arguments += '-WithImage' }
        if ($case.Capture) { $arguments += '-Capture' }
        $hostProcess = Start-Process powershell.exe -ArgumentList $arguments -PassThru
        try {
            if (!$hostProcess.WaitForExit(90000)) { throw 'Console probe timed out' }
            $state = Get-Content -Raw $resultFile | ConvertFrom-Json
            if (!$state.Ready) { throw "Console probe failed ($caseDescription): $($state.Error)" }
            # Allow the console host to finish processing the probe's exit.
            Start-Sleep -Milliseconds 500
            $gui = Get-Process -Id $state.ChildPid
            if ($gui.HasExited) { throw 'Exiting the console probe terminated the GUI' }
            Write-Host "GUI detached and survived probe exit: $caseDescription (console PIDs: $($state.ConsolePids -join ', '))"
        } finally {
            # Only clean up processes created by this probe.
            if (!$hostProcess.HasExited) { $hostProcess.Kill(); $hostProcess.WaitForExit() }
            if (Test-Path $resultFile) {
                $state = Get-Content -Raw $resultFile | ConvertFrom-Json
                if ($state.LauncherPid) {
                    Get-CimInstance Win32_Process -Filter "ParentProcessId = $($state.LauncherPid) AND Name = 'dvdisaster.exe'" |
                        ForEach-Object { Stop-Process -Id $_.ProcessId -ErrorAction SilentlyContinue }
                    Stop-Process -Id $state.LauncherPid -ErrorAction SilentlyContinue
                }
                if ($state.ChildPid) { Stop-Process -Id $state.ChildPid -ErrorAction SilentlyContinue }
            }
        }
    }
} finally {
    Remove-Item -Recurse -Force $temporary
}
