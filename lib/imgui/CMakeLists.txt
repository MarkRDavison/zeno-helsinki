SET(MODULE_LIBS
	"helsinki-engine"
	ThirdParty_imgui
)

SET(INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/lib/imgui/include"
)

submodule("Imgui" "${MODULE_LIBS}" "${INCLUDE_DIRS}")

target_include_directories(helsinki-imgui PRIVATE
	"${imgui_SOURCE_DIR}/backends")
