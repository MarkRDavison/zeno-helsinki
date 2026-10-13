file(GLOB_RECURSE TWO_DIMENSIONAL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(TwoDimensional STATIC ${TWO_DIMENSIONAL_SOURCES})
target_include_directories(TwoDimensional PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

TARGET_LINK_LIBRARIES(TwoDimensional PRIVATE "helsinki-system")
TARGET_LINK_LIBRARIES(TwoDimensional PRIVATE "helsinki-renderer")
TARGET_LINK_LIBRARIES(TwoDimensional PRIVATE "helsinki-engine")

SET(MODULE_LIBS
	"helsinki-system"
	"helsinki-renderer"
	"helsinki-engine"
	TwoDimensional
)

if(HELSINKI_ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( TwoDimensionalConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/TwoDimensionalConfig.hpp")

helsinkiApplication("HelsinkiTwoDimensionalExample" "TwoDimensional.cpp" "${MODULE_LIBS}")
