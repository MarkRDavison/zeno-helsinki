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

# -------------------- Wrap glslang in an interface target --------------------
add_library(MyGlslang INTERFACE)
target_include_directories(MyGlslang SYSTEM INTERFACE
    "${glslang_SOURCE_DIR}/glslang/Public"
)
target_link_libraries(MyGlslang INTERFACE glslang ThirdParty_SPIRV_Tools)

# Create alias only if it does not already exist
if(NOT TARGET glslang::glslang)
    add_library(glslang::glslang ALIAS MyGlslang)
endif()

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