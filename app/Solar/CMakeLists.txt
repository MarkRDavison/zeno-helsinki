file(GLOB_RECURSE SOLAR_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(Solar STATIC ${SOLAR_SOURCES})
helsinki_apply_target_options(Solar)
target_include_directories(Solar PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(Solar PRIVATE "helsinki-system")
TARGET_LINK_LIBRARIES(Solar PRIVATE "helsinki-renderer")
TARGET_LINK_LIBRARIES(Solar PRIVATE "helsinki-engine")

SET(MODULE_LIBS
	"helsinki-system"
	"helsinki-renderer"
	"helsinki-engine"
	Solar
)

if(HELSINKI_ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE(SolarConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/SolarConfig.hpp")

helsinkiApplication("ZenoSolar" "Solar.cpp" "${MODULE_LIBS}")
