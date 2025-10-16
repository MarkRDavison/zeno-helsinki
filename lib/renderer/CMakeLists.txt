SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	Vulkan::Vulkan
	glfw
	stb
)

SET(INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/lib/renderer/include"
)

submodule("Renderer" "${MODULE_LIBS}" "${INCLUDE_DIRS}")