# Use like this: 
# powershell.exe -File "[PATH_TO_SCRIPT]\brigde_wsl_ports.ps1"

$ports = @(1883);

# Get the WSL IP address using bash command
$wslAddress = bash.exe -c "ifconfig eth0 | grep -oP '(?<=inet\s)\d+(\.\d+){3}'"

# Validate the WSL IP address
if ($wslAddress -match '^(\d{1,3}\.){3}\d{1,3}$') {
  Write-Host "WSL IP address: $wslAddress" -ForegroundColor Green
  Write-Host "Ports: $ports" -ForegroundColor Green
}
else {
  Write-Host "Error: Could not find WSL IP address." -ForegroundColor Red
  exit
}


# Set listen address
$listenAddress = '0.0.0.0';

# Setup port forwarding rules
foreach ($port in $ports) {
  try {
    Invoke-Expression "netsh interface portproxy delete v4tov4 listenport=$port listenaddress=$listenAddress";
    Invoke-Expression "netsh interface portproxy add v4tov4 listenport=$port listenaddress=$listenAddress connectport=$port connectaddress=$wslAddress";
    
    Write-Host "Port forwarding set up for port $port" -ForegroundColor Green;
  }
  catch {
    Write-Host "Error while setting port forwarding for port $port" -ForegroundColor Red
  }
}


# Set firewall rule name
$fireWallDisplayName = 'WSL Port Forwarding';
$portsStr = $ports -join ",";

# Remove any existing firewall rules with the same display name if they exist
# they should come from earlier iterations of this script
if (Get-NetFirewallRule -DisplayName $fireWallDisplayName -ErrorAction SilentlyContinue) {
  try {
    Remove-NetFirewallRule -DisplayName $fireWallDisplayName
  }
  catch {
    Write-Host "Error removing existing firewall rules" -ForegroundColor Red
  }
}


# Create outbound and inbound firewall rules
try {
  New-NetFirewallRule -DisplayName $fireWallDisplayName -Direction Outbound -LocalPort $portsStr -Action Allow -Protocol TCP
  New-NetFirewallRule -DisplayName $fireWallDisplayName -Direction Inbound -LocalPort $portsStr -Action Allow -Protocol TCP
  Write-Host "Firewall rules created for ports $portsStr" -ForegroundColor Green
}
catch {
  Write-Host "Error creating firewall rules" -ForegroundColor Red
  exit
}