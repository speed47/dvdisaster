# Run GUI launches in a separate console, then verify that the GUI survives
# both the launcher's exit and the console host's exit.
param(
    [Parameter(Mandatory=$true)][string]$Dist,
    [switch]$Probe,
    [switch]$Direct,
    [switch]$WithImage,
    [string]$Result
)
$ErrorActionPreference = 'Stop'
$Dist = (Resolve-Path $Dist).Path

if ($Probe) {
    $state = @{ LauncherPid = 0; ChildPid = 0; Ready = $false; Error = '' }
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
        if ($WithImage) { $launch.Arguments = '-i handoff-test.iso' }
        $process = [System.Diagnostics.Process]::Start($launch)
        if ($Direct) { $state.ChildPid = $process.Id }
        else { $state.LauncherPid = $process.Id }
        $state | ConvertTo-Json | Set-Content $Result

        if (!$Direct) {
            if (!$process.WaitForExit(30000)) { throw 'Launcher kept waiting for the GUI' }
            $launcherExitCode = $process.ExitCode
            if ($null -eq $launcherExitCode) { throw 'Launcher exit code is unavailable' }
            if ($launcherExitCode -ne 0) { throw "Launcher exited with $launcherExitCode" }
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
        [System.Runtime.InteropServices.DllImport("kernel32.dll")]
        public static extern System.IntPtr GetConsoleWindow();
        [System.Runtime.InteropServices.DllImport("user32.dll", EntryPoint="PostMessageW")]
        public static extern bool PostMessage(System.IntPtr window, uint message, System.IntPtr wparam, System.IntPtr lparam);
'@
        $window = [Dvdisaster.ConsoleProbe]::GetConsoleWindow()
        if ($window -eq [IntPtr]::Zero) { throw 'Probe has no console window' }
        $state.Ready = $true
    } catch {
        $state.Error = $_.ToString()
    }
    $state | ConvertTo-Json | Set-Content $Result
    # Merely exiting the shell would leave its console alive if the GUI were
    # still attached. Close the console window explicitly to test detachment.
    if ($state.Ready) {
        if (![Dvdisaster.ConsoleProbe]::PostMessage($window, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero)) {
            $state.Ready = $false
            $state.Error = 'Could not close the probe console'
            $state | ConvertTo-Json | Set-Content $Result
            exit 0
        }
        # Do not catch the pipeline cancellation caused by console closure.
        Start-Sleep -Seconds 10
        $state.Ready = $false
        $state.Error = 'Probe survived closing its console'
        $state | ConvertTo-Json | Set-Content $Result
    }
    exit 0
}

$temporary = Join-Path ([IO.Path]::GetTempPath()) ([IO.Path]::GetRandomFileName())
New-Item -ItemType Directory $temporary | Out-Null
try {
    foreach ($directLaunch in @($false, $true)) {
        foreach ($imageArgument in @($false, $true)) {
            $resultFile = Join-Path $temporary 'result.json'
            Remove-Item $resultFile -ErrorAction SilentlyContinue
            $arguments = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`"",
                           '-Probe', '-Dist', "`"$Dist`"", '-Result', "`"$resultFile`"")
            if ($directLaunch) { $arguments += '-Direct' }
            if ($imageArgument) { $arguments += '-WithImage' }
            $hostProcess = Start-Process powershell.exe -ArgumentList $arguments -PassThru
            try {
                if (!$hostProcess.WaitForExit(90000)) { throw 'Console probe timed out' }
                $state = Get-Content -Raw $resultFile | ConvertFrom-Json
                if (!$state.Ready) { throw "Console probe failed: $($state.Error)" }
                # Allow delivery of the console-close notification before checking.
                Start-Sleep -Milliseconds 500
                $gui = Get-Process -Id $state.ChildPid
                if ($gui.HasExited) { throw 'Closing the console terminated the GUI' }
                Write-Host "GUI survived console exit: direct=$directLaunch image=$imageArgument"
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
    }
} finally {
    Remove-Item -Recurse -Force $temporary
}
