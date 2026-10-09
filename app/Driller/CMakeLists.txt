file(GLOB_RECURSE DRILLER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(Driller STATIC ${DRILLER_SOURCES})
target_include_directories(Driller PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(Driller PUBLIC "helsinki-scripting${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(Driller PRIVATE "helsinki-system${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(Driller PRIVATE "helsinki-renderer${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(Driller PRIVATE "helsinki-engine${LIB_EXTENSION_SHARED}")

SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-scripting${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
	Driller
)

if(ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( DrillerConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/DrillerConfig.hpp")

helsinkiApplication("HelsinkiDrillerExample" "Driller.cpp" "${MODULE_LIBS}")
