$PROJECT_NAME="FileDelete"
$ARCHITECTURE="x64"
$BUILD_DIRECOTY="$PSScriptRoot\$PROJECT_NAME\$ARCHITECTURE\Debug"
$SOURCE_DIRECOTY="$PSScriptRoot\$PROJECT_NAME\$PROJECT_NAME"

$VMWARE_VMRUN="C:\\Program Files (x86)\\VMware\\VMware Workstation\\vmrun.exe"
$WINDBG="C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x64\\windbg.exe"

$TEST_MACHINE_PATH="C:\\Users\\Rhydon\\Documents\\Virtual Machines\\Windows 10 x64 kernel\\Windows 10 x64 kernel.vmx"
$TEST_MACHINE_SNAPSHOT="ReadyWindbg"

$TEST_MACHINE_USERNAME="Rhydon"
$TEST_MACHINE_PASSWORD="123456"
$TEST_MACHINE_IP="192.168.112.129"

$REMOTE_DIR_PRODUCT="C:\"
$REMOTE_DRIVER_PATH="$REMOTE_DIR_PRODUCT\Debug\$PROJECT_NAME.sys"
$CREATE_SERVICE="sc create DriverSrv type=kernel binPath=`"$REMOTE_DRIVER_PATH`""
$START_SERVICE="sc start DriverSrv"

$DEBUG_PORT="55555"
$DEBUG_KEY="ehjno5eyjyiq.1qoua9kqvbuqu.2qu59nezkk31w.1z1dt8qrtzjih"

Write-Host "[+] Reverting test machine snapshot"
"`"$VMWARE_VMRUN`" -T ws revertToSnapshot `"$TEST_MACHINE_PATH`" $TEST_MACHINE_SNAPSHOT" | cmd

Start-Sleep -Seconds 1

$running_machines = "`"$VMWARE_VMRUN`" list" | cmd

if (-not [bool]($running_machines -Match $TEST_MACHINE_PATH)) {
    Write-Host "[+] Starting test machine"
    "`"$VMWARE_VMRUN`" -T ws start `"$TEST_MACHINE_PATH`"" | cmd
}

$return_value = [int](plink -batch -P 22 -pw $TEST_MACHINE_PASSWORD $TEST_MACHINE_USERNAME`@$TEST_MACHINE_IP "exit")
while (0 -ne $return_value) {
    Start-Sleep -Seconds 1
    $return_value = [int](plink -batch -P 22 -pw $TEST_MACHINE_PASSWORD $TEST_MACHINE_USERNAME`@$TEST_MACHINE_IP "exit")
}

Write-Host "[+] Uploading build directory to the machine"
pscp -batch -q -r -P 22 -pw $TEST_MACHINE_PASSWORD `"$BUILD_DIRECOTY`" $TEST_MACHINE_USERNAME`@$TEST_MACHINE_IP`:$REMOTE_DIR_PRODUCT

Write-Host "[+] Loading the driver"
plink -batch -P 22 -pw $TEST_MACHINE_PASSWORD $TEST_MACHINE_USERNAME`@$TEST_MACHINE_IP "$CREATE_SERVICE"
plink -batch -P 22 -pw $TEST_MACHINE_PASSWORD $TEST_MACHINE_USERNAME`@$TEST_MACHINE_IP "$START_SERVICE"

$WINDBG_SCRIPT="$PSScriptRoot\script.txt"
"taskkill /IM windbg.exe" | cmd

Write-Host "[+] Attaching the debugging session"
"echo .srcpath `"$SOURCE_DIRECOTY`" > $WINDBG_SCRIPT" | cmd
"echo .reload /fn $PROJECT_NAME.sys >> $WINDBG_SCRIPT" | cmd
"`"$WINDBG`" -k net:port=$DEBUG_PORT,key=$DEBUG_KEY -c `"$<$WINDBG_SCRIPT`" " | cmd
