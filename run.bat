@echo off
echo "run service......"
start cmd /k "sc create BgService binpath= %CD%\ZefraBgSwitchService.exe & sc config BgService start= AUTO & net start BgService"
exit