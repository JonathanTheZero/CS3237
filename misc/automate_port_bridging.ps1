# Usage:
# powershell.exe -File "[SCRIPT_PATH]\automate_port_bridging.ps1" -ScriptDir "[FULL_PATH_TO_BRIDGE_SCRIPT]"
# The script to bridge the ports to WSL is the other .ps1 script in this directory
# This is needed only once and will automate the whole port forwarding process as a scheduled task
# It will be run on every start up of the Windows machine


param (
    [string]$ScriptPath
)

# Validate the script path
if (-not (Test-Path -Path $ScriptPath)) {
    Write-Host "Error: Invalid script path provided." -ForegroundColor Red
    exit 1
}

$taskName = "WSL2PortsBridge"

# Unregister existing task if it exists
if (Get-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue) {
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false
}


try {
    $a = New-ScheduledTaskAction -Execute "powershell.exe" -Argument "-File `"$ScriptPath`" -WindowStyle Hidden -ExecutionPolicy Bypass"
    $t = New-ScheduledTaskTrigger -AtLogon
    $s = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries
    $p = New-ScheduledTaskPrincipal -GroupId "BUILTIN\Administrators" -RunLevel Highest

    Register-ScheduledTask -TaskName $taskName -Action $a -Trigger $t -Settings $s -Principal $p
    Write-Host "Scheduled task '$taskName' has been registered successfully." -ForegroundColor Green
}
catch {
    Write-Host "Error registering the scheduled task: $_" -ForegroundColor Red
    exit 1
}

Write-Host "Press Enter to exit..." -ForegroundColor Yellow
[void][System.Console]::ReadLine()