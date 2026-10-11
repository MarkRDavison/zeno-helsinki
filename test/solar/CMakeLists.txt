SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	Solar
	"helsinki-system"
)

testsubmodule("solar" "${MODULE_LIBS}")
TARGET_INCLUDE_DIRECTORIES(solar-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
