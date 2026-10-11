SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	Driller
	"helsinki-system"
	"helsinki-scripting"
)

testsubmodule("driller" "${MODULE_LIBS}")
TARGET_INCLUDE_DIRECTORIES(driller-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(driller-test PRIVATE
	DRILLER_DATA_DIR="${CMAKE_SOURCE_DIR}/app/Driller/data"
)
