SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	Vulkan::Vulkan
	glslang::glslang
	glfw
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
CONFIGURE_FILE(
	"${CMAKE_CURRENT_SOURCE_DIR}/include/helsinki/Renderer/RendererShaderRoot.hpp.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/helsinki/Renderer/RendererShaderRoot.hpp"
)

submodule("Renderer" "${MODULE_LIBS}" "${INCLUDE_DIRS}")