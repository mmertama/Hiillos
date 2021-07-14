cmake_minimum_required (VERSION 3.17)

function(hiillos_make_application HIILLOS_TARGET)
#if(NOT DEFINED HIILLOS_TARGET)
#    set(HIILLOS_TARGET ${PROJECT_NAME})
#endif()

    message("${HIILLOS_TARGET} build type is ${CMAKE_BUILD_TYPE}")

    set(default_build_type "Release")
 
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
        set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
        set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE PARENT_SCOPE)
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo") 
    endif()

    if(NOT DEFINED CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release")
        set(CMAKE_BUILD_TYPE "Release" PARENT_SCOPE)
    endif()

    if(EXISTS ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
        add_custom_command(TARGET ${HIILLOS_TARGET} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()

    add_custom_command(TARGET ${HIILLOS_TARGET} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/hiillos.zip"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Deflating..."
        VERBATIM
        )

    add_library(hiillos::hiillos INTERFACE IMPORTED)     
 
    if(NOT APPLE)
        set_target_properties(${HIILLOS_TARGET} PROPERTIES PREFIX "gempyre_")
        add_custom_command(TARGET ${HIILLOS_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename Hiillos${CMAKE_EXECUTABLE_SUFFIX} ${HIILLOS_TARGET}${CMAKE_EXECUTABLE_SUFFIX}
            COMMENT "Rename to Hiillos to ${HIILLOS_TARGET}${CMAKE_EXECUTABLE_SUFFIX}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}"
            )
        if(NOT MSVC) # GCC and mingGW
            if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
              #set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}" CACHE STRING "" FORCE)
              #set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}" PARENT_SCOPE)
              set_target_properties(${HIILLOS_TARGET} PROPERTIES
                 RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
             endif()
        endif()
    endif()


    if(WIN32)
        if(MSVC)
            #set_property(TARGET ${HIILLOS_TARGET} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
            #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}
            #    /NODEFAULTLIB:msvcrt.lib
            #    /NODEFAULTLIB:libucrt.lib
            #    /NODEFAULTLIB:libcmt.lib")
            #target_link_options(${HIILLOS_TARGET}
            #    PUBLIC /NODEFAULTLIB:msvcrt.lib
            #    PUBLIC /NODEFAULTLIB:libucrt.lib
            #    PUBLIC /NODEFAULTLIB:libcmt.lib)
            #set_target_properties(hiillos::hiillos PROPERTIES
            #    INTERFACE_LINK_LIBRARIES  "psapi.lib;userenv.lib;iphlpapi.lib;vcruntime.lib;ucrt.lib;kernel32.lib;msvcrt.lib;shlwapi.lib")
        #else()
        #    set_target_properties(hiillos::hiillos PROPERTIES
        #        INTERFACE_LINK_LIBRARIES  "psapi.lib;userenv.lib;iphlpapi.lib;ws2_32.lib;shlwapi.lib")
        endif()
    endif() 
    

    #message("target dir: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")


    if(APPLE)
        add_custom_command(TARGET ${HIILLOS_TARGET} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_BUILD_TYPE}/Hiillos.app ${CMAKE_BUILD_TYPE}/${HIILLOS_TARGET}.app
            COMMENT "Rename to ${CMAKE_BUILD_TYPE}/${HIILLOS_TARGET}.app"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
        add_custom_command(TARGET ${HIILLOS_TARGET} PRE_BUILD
            COMMAND defaults write ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${HIILLOS_TARGET}.app/Contents/Info CFBundleName -string "${HIILLOS_TARGET}"
            COMMENT "Set application name to \"${HIILLOS_TARGET}\""
            )
        add_custom_command(TARGET ${HIILLOS_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${HIILLOS_TARGET}.app/Contents/gempyre
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${HIILLOS_TARGET}> ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${HIILLOS_TARGET}.app/Contents/gempyre
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Copy into host application"
            )
        if(EXISTS ${CMAKE_SOURCE_DIR}/icons.iconset)
            add_custom_command(TARGET ${HIILLOS_TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/icons.iconset ${CMAKE_BINARY_DIR}/icons.iconset
                COMMAND iconutil --convert icns iconset
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${HIILLOS_TARGET}.app/Contents/Resources
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/icons.icns ${CMAKE_BUILD_TYPE}/${HIILLOS_TARGET}.app/Contents/Resources
                COMMAND defaults write ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${HIILLOS_TARGET}.app/Contents/Info CFBundleIconFile -string "icons.icns"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                )
        endif()

    elseif(UNIX)
         # unzip overrides everyhing, therefore we can copy only after zip (POS_BUILD)
       configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/desktop.in"
           "${CMAKE_BINARY_DIR}/.desktop"
           NEWLINE_STYLE UNIX)
       add_custom_command(TARGET ${HIILLOS_TARGET} POST_BUILD
           COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/.desktop" "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/.desktop"
           )
       if(NOT DEFINED APP_ICON)
           set(APP_ICON ${CMAKE_SOURCE_DIR}/${HIILLOS_TARGET}.png)
       endif()
       if(EXISTS ${CMAKE_SOURCE_DIR}/icons.iconset)
           add_custom_command(TARGET ${HIILLOS_TARGET} POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy ${APP_ICON} "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${HIILLOS_TARGET}.png"
               )
       endif()
   endif()
endfunction()
