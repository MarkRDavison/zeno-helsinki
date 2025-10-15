SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
)

SET(INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/lib/example/include"
)

submodule("Example" "${MODULE_LIBS}" "${INCLUDE_DIRS}")