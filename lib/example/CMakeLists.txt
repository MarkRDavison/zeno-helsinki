SET(MODULE_LIBS

)

SET(INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/lib/example/include"
)

submodule("Example" "${MODULE_LIBS}" "${INCLUDE_DIRS}")