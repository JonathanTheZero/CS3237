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
  }
  catch {
    Write-Host "Error while setting port forwarding for port $port" -ForegroundColor Red
    exit
  }
}

# Set firewall rule name
$fireWallDisplayName = 'WSL Port Forwarding';
$portsStr = $ports -join ",";

# Remove any existing firewall rules with the same display name
try { 
  Remove-NetFirewallRule -DisplayName $fireWallDisplayName 
}
catch {
  Write-Host "Error removing existing firewall rules" -ForegroundColor Red
}

# Create outbound and inbound firewall rules
try {
  New-NetFirewallRule -DisplayName $fireWallDisplayName -Direction Outbound -LocalPort $portsStr -Action Allow -Protocol TCP
  New-NetFirewallRule -DisplayName $fireWallDisplayName -Direction Inbound -LocalPort $portsStr -Action Allow -Protocol TCP
}
catch {
  Write-Host "Error creating firewall rules" -ForegroundColor Red
  exit
}