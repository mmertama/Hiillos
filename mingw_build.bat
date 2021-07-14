echo I suppose CEF is not working https://bitbucket.org/chromiumembedded/cef/issues/2643/windows-add-mingw-compile-support
goto end
if not exist "build_mingw" mkdir build_mingw 
pushd build_mingw 
cmake ..  -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release 
cmake --build . --config Release 
powershell -Command "Start-Process ../run.bat -Verb RunAs -WorkingDirectory "." -ArgumentList "%~dp0\build_mingw,cmake,--install,."
popd
:end
