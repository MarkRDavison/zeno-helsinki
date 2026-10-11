SET(MODULE_LIBS
	"helsinki-system"
	ThirdParty_miniaudio
)

SET(INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/lib/audio/include"
)

submodule("Audio" "${MODULE_LIBS}" "${INCLUDE_DIRS}")
