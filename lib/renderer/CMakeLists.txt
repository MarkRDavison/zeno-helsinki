SET(MODULE_LIBS
	helsinki-system
	Vulkan::Vulkan
	glslang::glslang
	stb
	tinyobjloader
	tinyxml2
	Freetype::Freetype
	msdfgen-core
	msdfgen-ext
)

SET(INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/lib/renderer/include"
)

file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/shaders" HELSINKI_RENDERER_SHADER_ROOT)
set(HELSINKI_RENDERER_SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/shaders" CACHE INTERNAL "")
CONFIGURE_FILE(
	"${CMAKE_CURRENT_SOURCE_DIR}/include/helsinki/Renderer/RendererShaderRootBaked.hpp.in"
	"${CMAKE_CURRENT_BINARY_DIR}/RendererShaderRootBaked.hpp"
	@ONLY
)

submodule("Renderer" "${MODULE_LIBS}" "${INCLUDE_DIRS}")
target_include_directories(helsinki-renderer PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")