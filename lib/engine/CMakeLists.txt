SET(MODULE_LIBS
	"helsinki-system"
	"helsinki-ui"
	"helsinki-renderer"
)

SET(INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/lib/engine/include"
)

submodule("Engine" "${MODULE_LIBS}" "${INCLUDE_DIRS}")