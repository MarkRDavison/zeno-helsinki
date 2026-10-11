file(GLOB_RECURSE PONG_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(Pong STATIC ${PONG_SOURCES})
target_include_directories(Pong PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(Pong PRIVATE "helsinki-system")
TARGET_LINK_LIBRARIES(Pong PRIVATE "helsinki-renderer")
TARGET_LINK_LIBRARIES(Pong PRIVATE "helsinki-engine")

SET(MODULE_LIBS
	"helsinki-system"
	"helsinki-renderer"
	"helsinki-engine"
	Pong
)

if(HELSINKI_ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( PongConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/PongConfig.hpp")

helsinkiApplication("HelsinkiPongExample" "Pong.cpp" "${MODULE_LIBS}")
