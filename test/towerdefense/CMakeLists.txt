SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	TowerDefense
	"helsinki-system"
	"helsinki-engine"
)

testsubmodule("towerdefense" "${MODULE_LIBS}")
TARGET_INCLUDE_DIRECTORIES(towerdefense-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
