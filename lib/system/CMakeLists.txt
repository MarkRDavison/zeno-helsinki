SET(MODULE_LIBS
	glm::glm
)

SET(INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/lib/system/include"
)

submodule("System" "${MODULE_LIBS}" "${INCLUDE_DIRS}")