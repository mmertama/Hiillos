cmake_minimum_required (VERSION 3.16)

set(CMAKE_POLICY_DEFAULT_ CMP0091 NEW) # otwrwise set fetched content wont work

project(hiillos LANGUAGES CXX C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE  "Release" CACHE STRING "build type" FORCE)
endif()

include(GNUInstallDirs)
include(FetchContent)

find_package(gempyre REQUIRED) # at least for utils

if(NOT DEFINED PROJECT_ARCH)
  set(PROJECT_ARCH "x86_64")
endif()

# Allow includes relative to the current source directory.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17) 




# lib UV is linking only MD, therefore GEmpoyre to be linked MD, and thus MT wont work here either, sigh
# maybe use Fetch command instead of Externalproject add and then override flags 
#IF(WIN32) # set windows inserting runtime
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT /Zc:sizedDealloc /Zc:__cplusplus")
#endif()

#
# CEF configuration.
#

set(USE_SANDBOX OFF)
option(USE_SANDBOX OFF)

if(MSVC)
  set(CEF_RUNTIME_LIBRARY_FLAG "/MD") # as gempyre is built MD, CEF has to be as well
endif()

# Specify the CEF distribution version.
set(CEF_VERSION "91.1.22+gc67b5dd+chromium-91.0.4472.124")

# Determine the platform.
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  if("${PROJECT_ARCH}" STREQUAL "arm64")
    set(CEF_PLATFORM "macosarm64")
  else()
    set(CEF_PLATFORM "macosx64")
  endif()
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(CEF_PLATFORM "linux64")
  else()
    set(CEF_PLATFORM "linux32")
  endif()
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(CEF_PLATFORM "windows64")
  else()
    set(CEF_PLATFORM "windows32")
  endif()
endif()

set(EXTRA_DIR ${CMAKE_BINARY_DIR})

# Add this project's cmake/ directory to the module path.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Download and extract the CEF binary distribution (executes DownloadCEF.cmake).
include(DownloadCEF)
DownloadCEF("${CEF_PLATFORM}" "${CEF_VERSION}" "${EXTRA_DIR}/third_party/cef")

message("CEF_ROOT ${CEF_ROOT}")

# Add the CEF binary distribution's cmake/ directory to the module path.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

# Load the CEF configuration (executes FindCEF.cmake).
find_package(CEF REQUIRED)

#
# Web sockets
#

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # otwrwise set fetched content wont work
# set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "noname")
# set(CMAKE_INSTALL_PREFIX "fiifoo" CACHE PATH "..." FORCE)

# installish
set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL TRUE)


set(FETCHCONTENT_BASE_DIR ${EXTRA_DIR}/third_party/libwebsockets)
FetchContent_Declare(
  libwebsockets
  GIT_REPOSITORY https://libwebsockets.org/repo/libwebsockets
  GIT_TAG "v4.2.1"
  GIT_PROGRESS TRUE
)

set(LWS_WITHOUT_SERVER ON)
set(LWS_WITHOUT_CLIENT OFF)
option(LWS_WITHOUT_SERVER ON)
option(LWS_WITHOUT_CLIENT OFF)

#installish
#set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "noname") 
#set(CMAKE_INSTALL_PREFIX "fiifoo")

if(WIN32)
  option(LWS_WITH_SYS_SMD OFF)
  set(LWS_WITH_SYS_SMD OFF)

  option(LWS_EXT_PTHREAD_LIBRARIES "")
  set(LWS_EXT_PTHREAD_LIBRARIES "")
  

  #set(MSVC_RUNTIME_LIBRARY ON)
  #set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib")
  #set(CMAKE_STATIC_LINKER_FLAGS  "${CMAKE_STATIC_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib")
  #set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib")
    
  #set_property(TARGET libwebsockets PROPERTY
  #MSVC_RUNTIME_LIBRARY MultiThreaded)

  option(LWS_STATIC_PIC  ON)
  set(LWS_STATIC_PIC  ON)
  if(MSVC)
    #add_compile_options(
    #    $<$<CONFIG:>:/MT> #---------|
    #    $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
    #    $<$<CONFIG:Release>:/MT> #--|
    #)
  endif()

  set(LWS_INSTALL_LIB_DIR ${CMAKE_BINARY_DIR}/foo1)
  set(LWS_INSTALL_CMAKE_DIR ${CMAKE_BINARY_DIR}/foo2)
  set(LWS_INSTALL_BIN_DIR ${CMAKE_BINARY_DIR}/foo3)
  #set(LWS_INSTALL_INCLUDE_DIR ${CMAKE_BINARY_DIR}/foo4)
  set(LWS_INSTALL_EXAMPLES_DIR ${CMAKE_BINARY_DIR}/foo5)
  #set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/foo6)
  #set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME base)
  

endif()

 set(LWS_WITHOUT_TESTAPPS ON) 
 option(LWS_WITHOUT_TESTAPPS ON)
 set(LWS_WITH_MINIMAL_EXAMPLES ON) 
 option(LWS_WITH_MINIMAL_EXAMPLES OFF)
 set(LWS_WITH_STATIC ON) 
 option(LWS_WITH_STATIC ON)
 set(LWS_WITH_SHARED OFF) 
 option(LWS_WITH_SHARED OFF)

#add_definitions(-DLWS_WITH_SSL=OFF -DLWS_WITHOUT_BUILTIN_SHA1=OFF -DLWS_ROLE_H1=ON)
option(LWS_WITH_SSL OFF)
set(LWS_WITH_NETWORK ON)
option(LWS_WITH_NETWORK ON)
set(LWS_WITH_SSL OFF)
set(LWS_WITHOUT_BUILTIN_SHA1 OFF)

option(LWS_ROLE_H1 ON)
set(LWS_ROLE_H1 ON)

option(LWS_ROLE_H2 ON)
set(LWS_ROLE_H2 ON)

set(LWS_WITH_HTTP2 ON)
option(LWS_WITH_HTTP2 ON)

option(LWS_WITH_DIR OFF)
option(LWS_ROLE_WS ON)
set(LWS_ROLE_WS ON)

option(LWS_CTEST_INTERNET_AVAILABLE OFF)
set(LWS_CTEST_INTERNET_AVAILABLE OFF)
set(LWS_WITH_MINIMAL_EXAMPLES OFF)
set(NOT LWS_WITHOUT_TESTAPPS OFF)

# Windows
option(LWS_SSL_CLIENT_USE_OS_CA_CERTS OFF)
set(LWS_SSL_CLIENT_USE_OS_CA_CERTS OFF)

# installish
# FetchContent_MakeAvailable(libwebsockets)

# installish
FetchContent_GetProperties(libwebsockets)
if(NOT libwebsockets_POPULATED)
  FetchContent_Populate(libwebsockets)
  add_subdirectory(${libwebsockets_SOURCE_DIR} ${libwebsockets_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# installish
#set_property(DIRECTORY "${libwebsockets_SOURCE_DIR}"
#  PROPERTY EXCLUDE_FROM_ALL TRUE) # prevent install fetched content

include_directories(${libwebsockets_SOURCE_DIR}/include ${libwebsockets_BINARY_DIR})

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${libwebsockets_SOURCE_DIR}/cmake")
include(LwsCheckRequirements)

#option(LWS_WITH_DIR OFF)
#option(LWS_ROLE_H1 ON)
#set(requirements 1)
#require_lws_config(LWS_ROLE_WS 1 requirements)
#require_lws_config(LWS_WITH_CLIENT 1 requirements)
#require_lws_config(LWS_WITHOUT_EXTENSIONS 0 requirements)

set(WS_LIBS ${SAMP} websockets ${LIBWEBSOCKETS_DEP_LIBS})

message("ws include ${libwebsockets_SOURCE_DIR}/include")

#
#    nlohmann
#


set(FETCHCONTENT_BASE_DIR ${EXTRA_DIR}/third_party/nlohmann)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG 03270ef09070baae6ab49d005d9ef216b8a971e8 # master and 3.9.1 has a bug and cannot be compiled
)

option(JSON_BuildTests OFF)
FetchContent_MakeAvailable(json)
include_directories(${json_SOURCE_DIR}/include)

#
# Image magic for Linux icons 
#

if(OS_LINUX)
    find_package(ImageMagick COMPONENTS Magick++)
    if(NOT ImageMagick_Magick++_FOUND)
        if(EXISTS "/usr/include/GraphicsMagick")
            message("-- Using GraphicsMagick instead")
            set(ImageMagick_INCLUDE_DIRS "/usr/GraphicsMagick")
            find_library(MAGICK_LIB magic)
            find_library(GMAGICK_LIB GraphicsMagick++)

            set(ImageMagick_LIBRARIES "-lmagic -lGraphicsMagick++")

            execute_process(COMMAND Magick++-config --ldflags
              OUTPUT_VARIABLE ImageMagick_LINK_FLAGS
              OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ImageMagick_LINK_FLAGS}")
        endif()
        if((MAGICK_LIB-NOTFOUND OR GMAGICK_LIB-NOTFOUND) AND NOT ImageMagick_Magick++_FOUND)
            message(FATAL_ERROR "Image magic is needed, try apt-get install libmagick-dev")
        endif()
    endif()
    include_directories(${ImageMagick_INCLUDE_DIRS})
 endif()


#
# Sources
#


set(SOURCES
        extension/native.h
        extension/extension.cpp
        extension/ws.cpp
        extension/extension.h
        app/hiillos_app.cc
        app/hiillos_app.h
        app/hiillos_handler.cc
        app/hiillos_handler.h
        app/hiillos_app.h
        cmake/hiillos.cmake
        )

if(OS_LINUX)
    set(SOURCES ${SOURCES}
        app/hiillos_linux.cc
        app/hiillos_handler_linux.cc
        extension/native_linux.cpp
        )
endif()

if(OS_WINDOWS)
    set(SOURCES ${SOURCES}
        app/hiillos_win.cc
        app/hiillos_handler_win.cc
        extension/native_win.cpp
        app/Hiillos.rc
        app/resource.h
        )

    set(RESOURCE
        app/Hiillos.exe.manifest
        app/res/cefsimple.ico
        app/res/small.ico
        )
endif()

if(OS_MAC)
    set(SOURCES ${SOURCES}
        app/hiillos_mac.mm
        app/hiillos_handler_mac.mm
        extension/native_mac.mm
        )

    set(RESOURCES
        app/mac/cefsimple.icns
        app/mac/English.lproj/InfoPlist.strings
        app/mac/English.lproj/MainMenu.xib
        app/mac/Info.plist
        )

    set(HELPER_SOURCES
        app/process_helper_mac.cc
        )
endif()


#
# Shared configuration.
#

set(CEFSIMPLE_SRCS ${SOURCES})
set(CEFSIMPLE_HELPER_SRCS ${HELPER_SOURCES})

# Include the libcef_dll_wrapper target (executes libcef_dll/CMakeLists.txt).
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

# Target executable names.
set(CEF_TARGET "Hiillos")
if(OS_MAC)
  set(CEF_HELPER_TARGET "Hiillos_Helper")
  set(CEF_HELPER_OUTPUT_NAME "Hiillos Helper")
else()
  # Logical target used to link the libcef library.
  ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")
endif()

# Determine the target output directory.
SET_CEF_TARGET_OUT_DIR()


#
# Linux configuration.
#

if(OS_LINUX)
  # Executable target.
  add_executable(${CEF_TARGET} ${CEFSIMPLE_SRCS})
  SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_TARGET}
      PRIVATE libcef_lib
      PRIVATE libcef_dll_wrapper
      PRIVATE ${CEF_STANDARD_LIBS}
      PRIVATE ${WS_LIBS}
      PRIVATE ${ImageMagick_LIBRARIES}
      PRIVATE gempyre::gempyre)

  # Set rpath so that libraries can be placed next to the executable.
  set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
  set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
  set_target_properties(${CEF_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
  if (EXISTS "${CEF_BINARY_DIR}/libminigbm.so")
    COPY_FILES("${CEF_TARGET}" "libminigbm.so" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  endif()

  # Set SUID permissions on the chrome-sandbox target.
  SET_LINUX_SUID_PERMISSIONS("${CEF_TARGET}" "${CEF_TARGET_OUT_DIR}/chrome-sandbox")

  target_compile_definitions(${CEF_TARGET} PRIVATE WINDOWS_HANDLE_TYPE=long)
endif()


#
# Mac OS X configuration.
#

if(OS_MAC)
  set(cefsimple_sources_mac_helper)
  option(OPTION_USE_ARC "Build with ARC (automatic Reference Counting) on macOS." ON)
  if(OPTION_USE_ARC)
    list(APPEND CEF_COMPILER_FLAGS
      -fobjc-arc
      )
    set_target_properties(${target} PROPERTIES
      CLANG_ENABLE_OBJC_ARC "YES"
      )
  endif()

  # Output path for the main app bundle.
  set(CEF_APP "${CEF_TARGET_OUT_DIR}/${CEF_TARGET}.app")

  # Variables referenced from the main Info.plist file.
  set(EXECUTABLE_NAME "${CEF_TARGET}")
  set(PRODUCT_NAME "${CEF_TARGET}")

  if(USE_SANDBOX)
    # Logical target used to link the cef_sandbox library.
    ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
  endif()

  # Main app bundle target.
  add_executable(${CEF_TARGET} MACOSX_BUNDLE ${CEFSIMPLE_RESOURCES_SRCS} ${CEFSIMPLE_SRCS})
  SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_TARGET}
      PRIVATE libcef_dll_wrapper
      PRIVATE ${CEF_STANDARD_LIBS}
      PRIVATE ${WS_LIBS}
      PRIVATE gempyre::gempyre)
  set_target_properties(${CEF_TARGET} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/app/mac/Info.plist
    )

  # Copy the CEF framework into the Frameworks directory.
  add_custom_command(
    TARGET ${CEF_TARGET}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
            "${CEF_APP}/Contents/Frameworks/Chromium Embedded Framework.framework"
    VERBATIM
    )

  # Create the multiple Helper app bundle targets.
  foreach(_suffix_list ${CEF_HELPER_APP_SUFFIXES})
    # Convert to a list and extract the suffix values.
    string(REPLACE ":" ";" _suffix_list ${_suffix_list})
    list(GET _suffix_list 0 _name_suffix)
    list(GET _suffix_list 1 _target_suffix)
    list(GET _suffix_list 2 _plist_suffix)

    # Define Helper target and output names.
    set(_helper_target "${CEF_HELPER_TARGET}${_target_suffix}")
    set(_helper_output_name "${CEF_HELPER_OUTPUT_NAME}${_name_suffix}")

    # Create Helper-specific variants of the helper-Info.plist file. Do this
    # manually because the configure_file command (which is executed as part of
    # MACOSX_BUNDLE_INFO_PLIST) uses global env variables and would insert the
    # wrong values with multiple targets.
    set(_helper_info_plist "${CMAKE_CURRENT_BINARY_DIR}/helper-Info${_target_suffix}.plist")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/app/mac/helper-Info.plist" _plist_contents)
    string(REPLACE "\${EXECUTABLE_NAME}" "${_helper_output_name}" _plist_contents ${_plist_contents})
    string(REPLACE "\${PRODUCT_NAME}" "${_helper_output_name}" _plist_contents ${_plist_contents})
    string(REPLACE "\${BUNDLE_ID_SUFFIX}" "${_plist_suffix}" _plist_contents ${_plist_contents})
    file(WRITE ${_helper_info_plist} ${_plist_contents})

    # Create Helper executable target.
    add_executable(${_helper_target} MACOSX_BUNDLE ${CEFSIMPLE_HELPER_SRCS})
    SET_EXECUTABLE_TARGET_PROPERTIES(${_helper_target})
    add_dependencies(${_helper_target} libcef_dll_wrapper)
    target_link_libraries(${_helper_target}
        PRIVATE libcef_dll_wrapper
        PRIVATE ${CEF_STANDARD_LIBS}
        PRIVATE ${WS_LIBS})
    set_target_properties(${_helper_target} PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${_helper_info_plist}
      OUTPUT_NAME ${_helper_output_name}
      )

    if(USE_SANDBOX)
      target_link_libraries(${_helper_target} cef_sandbox_lib)
    endif()

    # Add the Helper as a dependency of the main executable target.
    add_dependencies(${CEF_TARGET} "${_helper_target}")

    # Copy the Helper app bundle into the Frameworks directory.
    add_custom_command(
      TARGET ${CEF_TARGET}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
              "${CEF_TARGET_OUT_DIR}/${_helper_output_name}.app"
              "${CEF_APP}/Contents/Frameworks/${_helper_output_name}.app"
      VERBATIM
      )
  endforeach()

  # Manually process and copy over resource files.
  # The Xcode generator can support this via the set_target_properties RESOURCE
  # directive but that doesn't properly handle nested resource directories.
  # Remove these prefixes from input file paths.
  set(PREFIXES "mac/")
  COPY_MAC_RESOURCES("${CEFSIMPLE_RESOURCES_SRCS}" "${PREFIXES}" "${CEF_TARGET}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_APP}")
  target_compile_definitions(${CEF_TARGET} PRIVATE WINDOWS_HANDLE_TYPE=void*)
endif()


#
# Windows configuration.
#

if(OS_WINDOWS)
  #if(MSVC)
  #  set(WIN_LIB psapi.lib userenv.lib iphlpapi.lib) # kernel32.lib libvcruntime.lib msvcrt.lib ucrt.lib libvcruntime.lib libucrt.lib
  #else()
  #  set(WIN_LIB psapi.lib userenv.lib iphlpapi.lib ws2_32.lib)
  #endif()
  # Executable target.
  add_executable(${CEF_TARGET} WIN32 ${CEFSIMPLE_SRCS})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
  target_link_libraries(${CEF_TARGET}
    PRIVATE libcef_lib
    PRIVATE libcef_dll_wrapper
    PRIVATE  ${CEF_STANDARD_LIBS}
    PRIVATE  ${WS_LIBS}
   # PRIVATE  ${CMAKE_THREAD_LIBS_INIT}
   # PRIVATE  ${WIN_LIB}
    PRIVATE  gempyre::gempyre
   )

     
  if(USE_SANDBOX)
    message(FATAL_ERROR "I assume not")
    # Logical target used to link the cef_sandbox library.
    ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
    target_link_libraries(${CEF_TARGET} PRIVATE cef_sandbox_lib ${CEF_SANDBOX_STANDARD_LIBS})
  endif()

  # Add the custom manifest files to the executable.
  ADD_WINDOWS_MANIFEST("${CMAKE_CURRENT_SOURCE_DIR}/app" "${CEF_TARGET}" "exe")

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
  target_compile_definitions(${CEF_TARGET} PRIVATE WINDOWS_HANDLE_TYPE=void*)
endif()


target_compile_features(${CEF_TARGET} PUBLIC cxx_std_17)
target_compile_options(${CEF_TARGET} PRIVATE -fexceptions)



install(CODE "message(\"Only copy files into gempyre development, application is not installed.\")")

if(NOT WIN32)
  set(HIILLOS_CONFIG_INSTALL_DIR  "${CMAKE_INSTALL_FULL_LIBDIR}/cmake/gempyre")
else() # to copy the same folder as gempyre
  string(REPLACE "hiillos" "gempyre" out_string ${CMAKE_INSTALL_FULL_LIBDIR})
  set(HIILLOS_CONFIG_INSTALL_DIR  "${out_string}/cmake/gempyre")
endif()

install(FILES
    ${CMAKE_SOURCE_DIR}/cmake/hiillos.cmake
    #${CMAKE_SOURCE_DIR}/cmake/makeApplication.py
    DESTINATION "${HIILLOS_CONFIG_INSTALL_DIR}"
    )

if(UNIX AND NOT APPLE)
    install(FILES
        ${CMAKE_SOURCE_DIR}/app/linux/desktop.in
        DESTINATION "${HIILLOS_CONFIG_INSTALL_DIR}"
        )
endif()

install(CODE "
    message(\"Archive build into ${HIILLOS_CONFIG_INSTALL_DIR}/hiillos.zip\")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar cfv \"${HIILLOS_CONFIG_INSTALL_DIR}/hiillos.zip\" --format=zip \"${CEF_TARGET_OUT_DIR}\")
    "
    )

