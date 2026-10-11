SET(MODULE_LIBS
	"helsinki-system"
)

SET(INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/lib/ui/include"
)

submodule("Ui" "${MODULE_LIBS}" "${INCLUDE_DIRS}")
