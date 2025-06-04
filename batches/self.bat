@echo off

set /p user_input=Please input the message:

git checkout main
git add .
git commit -m "%user_input%"
git push origin main

echo The process has finished.
pause