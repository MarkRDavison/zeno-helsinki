file(GLOB_RECURSE DRILLER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(Driller STATIC ${DRILLER_SOURCES})
target_include_directories(Driller PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(Driller PUBLIC "helsinki-scripting")
TARGET_LINK_LIBRARIES(Driller PRIVATE "helsinki-system")
TARGET_LINK_LIBRARIES(Driller PRIVATE "helsinki-ui")
TARGET_LINK_LIBRARIES(Driller PRIVATE "helsinki-renderer")
TARGET_LINK_LIBRARIES(Driller PRIVATE "helsinki-engine")

SET(MODULE_LIBS
	"helsinki-system"
	"helsinki-scripting"
	"helsinki-ui"
	"helsinki-renderer"
	"helsinki-engine"
	Driller
)

if(HELSINKI_ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( DrillerConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/DrillerConfig.hpp")

helsinkiApplication("HelsinkiDrillerExample" "Driller.cpp" "${MODULE_LIBS}")
