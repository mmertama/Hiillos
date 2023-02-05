# Hiillos

Hiillos is a native UI server for [Gempyre](https://github.com/mmertama/Gempyre). Gempyre itself let you freely choose the UI server, as long as it supports standard web technologies, and it not other server are provided, the OS default browser is used. However that is limited in away that file dialogs are not supported. Therefore from the begining alternative UI servers were delivered as 'extensions' (in affiliates directory). There were 'qt_client' and 'py_client' (called clients as tecnically they connect to Gempyre). The qt_client (implemented using Qt) is dropped off from the latest version and replaced with Hiillos.

Hiillos implements an OS native application. Therefore Gempyre application using Hiillos does look like and behave like native application, unlike when Gempyre is using py_client, qt_client or web browser.

## Building Hiillos


1) Clone Hiillos (git clone...)
2) Build it

mkdir build
cd build
cmake ..  -DCMAKE_BUILD_TYPE=Release
cmake --build .  --config Release

Then for OSX and Linux install it

sudo cmake --install .


### Windows spesific

For MSVC you have to do compile in the  x64 Native Tools Command Prompt for VS 2019 (others may work as well, but not tested).

For MinGW add -G parameter.
cmake ..  -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

... and then it is not working before  https://bitbucket.org/chromiumembedded/cef/issues/2643/windows-add-mingw-compile-support get fixed, sorry. However for Gempyre, the MSVC Hiillos shll work for both MSVC and MinGW.


## Applying Hiillos
----------------

### Presequites

Gempyre is installed 

### CMakeLists.txt
```cmake
cmake_minimum_required (VERSION 3.18) 

project (...

find_package (gempyre REQUIRED) <-- first this , and after then...
include(hiillos) <-- This line!
include(gempyre) <- also gempyre!

...

add_executable(....

...

hiillos_make_application(${PROJECT_NAME})  <--- This line

gempyre_add_resources(PROJECT ${PROJECT_NAME} ...

target_link_libraries (${PROJECT_NAME}
    gempyre::gempyre <- also gempyre
    ...

...
```

### Source
Hiillos application is using a spesific override of Gempyre::Ui constructor:

```cpp
 explicit Ui(const Filemap& filemap, const std::string& indexHtml, int argc, char** argv);
```

and therefore 

```cpp
int main(int argc, char** argv)  {

    Gempyre::Ui ui(Myapp_resourceh), "myapp.html", argc, argv);

    const auto arguments = GempyreUtils::parseArgs(argc, argv, {});
    
    ...
    
    ui.run();
    return 0;
}

```


