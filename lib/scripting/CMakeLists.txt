SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	ThirdParty_sol2
)

SET(INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/lib/scripting/include"
)

submodule("Scripting" "${MODULE_LIBS}" "${INCLUDE_DIRS}")

target_link_libraries(helsinki-scripting PUBLIC ThirdParty_lua)
target_compile_definitions(helsinki-scripting PUBLIC
	$<$<CONFIG:Debug>:SOL_ALL_SAFETIES_ON>
)
