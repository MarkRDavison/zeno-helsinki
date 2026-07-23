file(GLOB_RECURSE HURRICANE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(Hurricane STATIC ${HURRICANE_SOURCES})
target_include_directories(Hurricane PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(Hurricane PRIVATE "helsinki-system${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(Hurricane PRIVATE "helsinki-renderer${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(Hurricane PRIVATE "helsinki-engine${LIB_EXTENSION_SHARED}")

SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
	Hurricane
)

if(ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( HurricaneConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/HurricaneConfig.hpp")

helsinkiApplication("HelsinkiHurricaneExample" "Hurricane.cpp" "${MODULE_LIBS}")
