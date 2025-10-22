SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
	"helsinki-example${LIB_EXTENSION_SHARED}"
)

SET(MODULE_LIBS_POST
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
	"helsinki-example${LIB_EXTENSION_SHARED}"
	vk-bootstrap::vk-bootstrap
)

CONFIGURE_FILE( ExampleConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/ExampleConfig.hpp")

helsinkiApplication("HelsinkiExample" "example.cpp" "${MODULE_LIBS}")
helsinkiApplication("PostProcessingExample" "PostProcessing.cpp" "${MODULE_LIBS_POST}")