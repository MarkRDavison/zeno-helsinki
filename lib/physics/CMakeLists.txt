SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
)

SET(INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/lib/physics/include"
)

submodule("Physics" "${MODULE_LIBS}" "${INCLUDE_DIRS}")

target_link_libraries(helsinki-physics PRIVATE Jolt)
target_include_directories(helsinki-physics SYSTEM PRIVATE
	$<TARGET_PROPERTY:Jolt,INTERFACE_INCLUDE_DIRECTORIES>)
