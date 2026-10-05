file(GLOB_RECURSE TOWERDEFENSE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(TowerDefense STATIC ${TOWERDEFENSE_SOURCES})
target_include_directories(TowerDefense PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(TowerDefense PRIVATE "helsinki-system${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(TowerDefense PRIVATE "helsinki-audio${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(TowerDefense PRIVATE "helsinki-ui${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(TowerDefense PRIVATE "helsinki-renderer${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(TowerDefense PRIVATE "helsinki-engine${LIB_EXTENSION_SHARED}")

SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-audio${LIB_EXTENSION_SHARED}"
	"helsinki-ui${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
	TowerDefense
)

if(ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( TowerDefenseConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/TowerDefenseConfig.hpp")

helsinkiApplication("HelsinkiTowerDefenseExample" "TowerDefense.cpp" "${MODULE_LIBS}")
