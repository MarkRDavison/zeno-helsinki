SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
)

SET(INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/lib/ui/include"
)

submodule("Ui" "${MODULE_LIBS}" "${INCLUDE_DIRS}")
