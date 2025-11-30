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

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1
)

FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG fede005abaf93d9d7f3a679d1999b2db341b360f)

FetchContent_MakeAvailable(stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

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