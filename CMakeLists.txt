CMAKE_MINIMUM_REQUIRED(VERSION 3.15)
PROJECT(helsinki CXX)

SET(HELSINKI_CXX_STANDARD 23)

SET(CMAKE_CXX_STANDARD ${HELSINKI_CXX_STANDARD})
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/Zc:__cplusplus)  # Make __cplusplus reflect the real standard
endif()

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
OPTION(CMAKE_COMPILE_WARNING_AS_ERROR "Compile warnings as errors" ON)
OPTION(HELSINKI_STRICT "Enable strict compile options" ON)
option(ENABLE_TRACY "Enable Tracy Profiler" OFF)

if(ENABLE_ASAN)
    if(MSVC)
        message(STATUS "Enabling AddressSanitizer with MSVC")
        add_compile_options(/fsanitize=address /Zi /Od /MDd /EHsc)
        add_link_options(/fsanitize=address /INCREMENTAL:NO)
        MESSAGE(STATUS "VC TOOLS DIR")
        MESSAGE(STATUS $ENV{VCToolsInstallDir})
        set(ASAN_DLL "$ENV{VCToolsInstallDir}/bin/Hostx64/x64clang_rt.asan_dynamic-x86_64.dll")
        if(EXISTS "${ASAN_DLL}")
            add_custom_command(TARGET YourTarget POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ASAN_DLL}"
                $<TARGET_FILE_DIR:YourTarget>
                COMMENT "Copying ASan runtime DLL to output directory (Windows)"
            )
        else()
            message(WARNING "ASan DLL not found at ${ASAN_DLL}, runtime may fail")
        endif()
    else()
        message(STATUS "Enabling AddressSanitizer with non-MSVC")
        add_compile_options(-fsanitize=address -O1 -g)
        add_link_options(-fsanitize=address)
    endif()
endif()

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

SET(CMAKE_DEBUG_POSTFIX "-d")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DHELSINKI_DEBUG)
    set(ENABLE_TRACY ON)
endif()

if(ENABLE_TRACY)
    add_definitions(-DHELSINKI_TRACY_ENABLE)
    add_definitions(-DTRACY_ENABLE)
endif()

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.19)
  CMAKE_POLICY(SET CMP0110 NEW)
endif()

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/CMakeLists.txt")
ADD_SUBDIRECTORY(lib)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # We're in the root, define additional targets for developers.
    option(HELSINKI_BUILD_APPS     "whether or not apps should be built" ON)
    option(HELSINKI_BUILD_TESTS    "whether or not tests should be built" ON)

    if(HELSINKI_BUILD_APPS)
        ADD_SUBDIRECTORY(app)
    endif()
    if(HELSINKI_BUILD_TESTS)
        ENABLE_TESTING()
        ADD_SUBDIRECTORY(test)
    endif()
endif()
