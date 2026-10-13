file(GLOB_RECURSE HURRICANE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(Hurricane STATIC ${HURRICANE_SOURCES})
target_include_directories(Hurricane PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(Hurricane PRIVATE "helsinki-system")
TARGET_LINK_LIBRARIES(Hurricane PRIVATE "helsinki-audio")
TARGET_LINK_LIBRARIES(Hurricane PRIVATE "helsinki-ui")
TARGET_LINK_LIBRARIES(Hurricane PRIVATE "helsinki-renderer")
TARGET_LINK_LIBRARIES(Hurricane PRIVATE "helsinki-engine")

SET(MODULE_LIBS
	"helsinki-system"
	"helsinki-audio"
	"helsinki-ui"
	"helsinki-renderer"
	"helsinki-engine"
	Hurricane
)

if(HELSINKI_ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( HurricaneConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/HurricaneConfig.hpp")

helsinkiApplication("HelsinkiHurricaneExample" "Hurricane.cpp" "${MODULE_LIBS}")
