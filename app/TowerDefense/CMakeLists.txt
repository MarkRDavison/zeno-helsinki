file(GLOB_RECURSE TOWERDEFENSE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(TowerDefense STATIC ${TOWERDEFENSE_SOURCES})
target_include_directories(TowerDefense PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(TowerDefense PRIVATE "helsinki-system")
TARGET_LINK_LIBRARIES(TowerDefense PRIVATE "helsinki-audio")
TARGET_LINK_LIBRARIES(TowerDefense PRIVATE "helsinki-ui")
TARGET_LINK_LIBRARIES(TowerDefense PRIVATE "helsinki-renderer")
TARGET_LINK_LIBRARIES(TowerDefense PRIVATE "helsinki-engine")

SET(MODULE_LIBS
	"helsinki-system"
	"helsinki-audio"
	"helsinki-ui"
	"helsinki-renderer"
	"helsinki-engine"
	TowerDefense
)

if(HELSINKI_ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( TowerDefenseConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/TowerDefenseConfig.hpp")

helsinkiApplication("HelsinkiTowerDefenseExample" "TowerDefense.cpp" "${MODULE_LIBS}")
