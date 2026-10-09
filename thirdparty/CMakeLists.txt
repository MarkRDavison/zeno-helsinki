INCLUDE(FetchContent)

# -------------------- SPIRV-Headers --------------------
FetchContent_Declare(
    SPIRV-Headers
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
    GIT_TAG vulkan-sdk-1.4.309.0
)
FetchContent_MakeAvailable(SPIRV-Headers)

add_library(ThirdParty_SPIRV_Headers INTERFACE)
target_include_directories(ThirdParty_SPIRV_Headers SYSTEM INTERFACE
    "${spirv-headers_SOURCE_DIR}/include"
)

# -------------------- SPIRV-Tools --------------------
FetchContent_Declare(
    SPIRVTools
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
    GIT_TAG vulkan-sdk-1.4.309.0
)
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SPIRV-Headers_DIR "${spirv-headers_SOURCE_DIR}" CACHE PATH "" FORCE)
FetchContent_MakeAvailable(SPIRVTools)

add_library(ThirdParty_SPIRV_Tools INTERFACE)
target_include_directories(ThirdParty_SPIRV_Tools SYSTEM INTERFACE
    "${spirvtools_SOURCE_DIR}/include"
)
target_link_libraries(ThirdParty_SPIRV_Tools INTERFACE ThirdParty_SPIRV_Headers)

# -------------------- glslang --------------------
FetchContent_Declare(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG vulkan-sdk-1.4.309.0
)
set(ENABLE_OPT ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(ALLOW_EXTERNAL_SPIRV_TOOLS OFF CACHE BOOL "" FORCE)
set(SPIRV_TOOLS_SOURCE_DIR "${spirvtools_SOURCE_DIR}" CACHE PATH "" FORCE)
set(SPIRV_TOOLS_BINARY_DIR "${spirvtools_BINARY_DIR}" CACHE PATH "" FORCE)
FetchContent_MakeAvailable(glslang)


# -------------------- Vulkan --------------------
find_package(Vulkan REQUIRED)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.10.0)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

FetchContent_Declare(
  FakeIt
  GIT_REPOSITORY https://github.com/eranpeer/FakeIt.git
  GIT_TAG        2.4.1)

FetchContent_MakeAvailable(FakeIt)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4
)

FetchContent_MakeAvailable(glfw)

# SOURCE_SUBDIR skips glm's CMake (cmake_minimum_required 3.6 deprecation warning).
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1
  SOURCE_SUBDIR  skip-cmake
)
FetchContent_MakeAvailable(glm)
add_library(glm INTERFACE)
add_library(glm::glm ALIAS glm)
target_include_directories(glm SYSTEM INTERFACE "${glm_SOURCE_DIR}")

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG fede005abaf93d9d7f3a679d1999b2db341b360f)

FetchContent_MakeAvailable(stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

# -------------------- miniaudio (Ogg via existing stb_vorbis) --------------------
# SOURCE_SUBDIR skips miniaudio's CMake; we compile ThirdParty_miniaudio ourselves.
FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_TAG        0.11.22
    SOURCE_SUBDIR  skip-cmake
)
FetchContent_MakeAvailable(miniaudio)

enable_language(C)
find_package(Threads REQUIRED)

add_library(ThirdParty_miniaudio STATIC
    "${PROJECT_SOURCE_DIR}/thirdparty/miniaudio_impl.c"
)
target_include_directories(ThirdParty_miniaudio SYSTEM PUBLIC
    "${miniaudio_SOURCE_DIR}"
)
target_compile_definitions(ThirdParty_miniaudio PRIVATE MA_NO_ENCODING)
target_link_libraries(ThirdParty_miniaudio PUBLIC stb Threads::Threads)
if(UNIX AND NOT APPLE)
    target_link_libraries(ThirdParty_miniaudio PUBLIC m ${CMAKE_DL_LIBS})
endif()

# -------------------- Lua 5.4 (helsinki-scripting; not Lua 5.5 / LuaJIT) --------------------
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG        v5.4.8
)
FetchContent_MakeAvailable(lua)

set(_lua_lib_sources
    lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c
    llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c
    ltable.c ltm.c lundump.c lvm.c lzio.c
    lauxlib.c lbaselib.c lcorolib.c ldblib.c liolib.c lmathlib.c
    loadlib.c loslib.c lstrlib.c ltablib.c lutf8lib.c linit.c
)
set(_lua_lib_paths)
foreach(_lua_src IN LISTS _lua_lib_sources)
    list(APPEND _lua_lib_paths "${lua_SOURCE_DIR}/${_lua_src}")
endforeach()

add_library(ThirdParty_lua STATIC ${_lua_lib_paths})
set_target_properties(ThirdParty_lua PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    COMPILE_WARNING_AS_ERROR OFF
)
target_include_directories(ThirdParty_lua SYSTEM PUBLIC "${lua_SOURCE_DIR}")
if(UNIX)
    target_link_libraries(ThirdParty_lua PUBLIC m)
endif()

# -------------------- sol2 v3.5.0 (INTERFACE; skip sol2's CMake — it FetchContents its own Lua) --------------------
FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG        v3.5.0
    SOURCE_SUBDIR  skip-cmake
)
FetchContent_MakeAvailable(sol2)

add_library(ThirdParty_sol2 INTERFACE)
target_include_directories(ThirdParty_sol2 SYSTEM INTERFACE "${sol2_SOURCE_DIR}/include")
target_link_libraries(ThirdParty_sol2 INTERFACE ThirdParty_lua)

FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG v2.0.0rc13)
    
FetchContent_MakeAvailable(tinyobjloader)
    
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5)
    
FetchContent_MakeAvailable(imgui)
add_library(imgui INTERFACE)
target_include_directories(imgui INTERFACE ${imgui_SOURCE_DIR})

FetchContent_Declare(
    Tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
    GIT_TAG        v0.13.0
)
FetchContent_MakeAvailable(Tracy)

add_library(ThirdParty_Tracy STATIC
    "${tracy_SOURCE_DIR}/public/TracyClient.cpp"
)

target_include_directories(ThirdParty_Tracy PUBLIC
    "${tracy_SOURCE_DIR}/public"
)

set(FT_DISABLE_BROTLI TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_ZLIB   TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2  TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG    TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ TRUE CACHE BOOL "" FORCE)
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG        VER-2-14-1
)

FetchContent_MakeAvailable(freetype)

add_library(Freetype::Freetype ALIAS freetype)


FetchContent_Declare(
  tinyxml2
  GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
  GIT_TAG        11.0.0
)

FetchContent_MakeAvailable(tinyxml2)

set(MSDFGEN_DYNAMIC_RUNTIME ON CACHE BOOL "" FORCE)
set(MSDFGEN_BUILD_STANDALONE OFF CACHE BOOL "" FORCE)
set(MSDFGEN_USE_VCPKG OFF CACHE BOOL "" FORCE)
set(MSDFGEN_USE_SKIA OFF CACHE BOOL "" FORCE)
set(MSDFGEN_DISABLE_SVG ON CACHE BOOL "" FORCE)
set(MSDFGEN_DISABLE_PNG ON CACHE BOOL "" FORCE)
FetchContent_Declare(
  msdfgen
  GIT_REPOSITORY https://github.com/Chlumsky/msdfgen.git
  GIT_TAG        v1.12.1
)
FetchContent_MakeAvailable(msdfgen)

# -------------------- Jolt Physics (helsinki-physics PRIVATE; do not PUBLIC-link) --------------------
set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(TARGET_HELLO_WORLD OFF CACHE BOOL "" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "" FORCE)
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "" FORCE)
set(DOUBLE_PRECISION OFF CACHE BOOL "" FORCE)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
set(ENABLE_ALL_WARNINGS OFF CACHE BOOL "" FORCE)
set(JPH_USE_VK OFF CACHE BOOL "" FORCE)
set(JPH_USE_DX12 OFF CACHE BOOL "" FORCE)
set(JPH_USE_MTL OFF CACHE BOOL "" FORCE)
set(OVERRIDE_CXX_FLAGS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	JoltPhysics
	GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
	GIT_TAG        v5.6.0
	SOURCE_SUBDIR  Build
)
FetchContent_MakeAvailable(JoltPhysics)
if(TARGET Jolt)
	set_target_properties(Jolt PROPERTIES COMPILE_WARNING_AS_ERROR OFF)
endif()
