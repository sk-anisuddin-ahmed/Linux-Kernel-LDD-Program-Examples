@echo off
REM === Configuration ===
set USER=debian
set HOST=192.168.137.160
set DEST=/home/debian/

REM === Transfer current directory recursively ===
scp -r "%cd%" %USER%@%HOST%:%DEST%

pause
