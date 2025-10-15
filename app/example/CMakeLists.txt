SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
	"helsinki-example${LIB_EXTENSION_SHARED}"
)


FILE(GLOB_RECURSE CPP_FILES "*.cpp")

CONFIGURE_FILE( ExampleConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/ExampleConfig.hpp")

helsinkiApplication("HelsinkiExample" "${CPP_FILES}" "${MODULE_LIBS}")