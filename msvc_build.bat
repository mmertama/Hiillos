if not exist "build_msvc" mkdir build_msvc 
pushd build_msvc 
cmake .. -DCMAKE_BUILD_TYPE=Release 
cmake --build . --config Release 
powershell -Command "Start-Process ../run.bat -Verb RunAs -ArgumentList "%~dp0\build_msvc,cmake,--install,."
popd

