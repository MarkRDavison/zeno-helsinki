SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	TowerDefense
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
)

testsubmodule("towerdefense" "${MODULE_LIBS}")
TARGET_INCLUDE_DIRECTORIES(towerdefense-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
