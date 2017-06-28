@echo off

set DestPath=.
set DestExt=*~ *.bak *#

echo //=================================//
echo //           Clean up              //
echo //=================================//

for /f "delims=" %%i in ('dir  /b/a-d/s  %DestPath%\%DestExt%') do (
    echo cleaning up %%i
    del /p %%i)

echo clean up finished
pause
