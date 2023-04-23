@echo off
echo "exit service......"
start cmd /k "net stop BgService & sc delete BgService"
exit