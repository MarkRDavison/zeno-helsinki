file(GLOB_RECURSE TWO_DIMENSIONAL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(TwoDimensional STATIC ${TWO_DIMENSIONAL_SOURCES})
target_include_directories(TwoDimensional PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

TARGET_LINK_LIBRARIES(TwoDimensional PRIVATE "helsinki-system${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(TwoDimensional PRIVATE "helsinki-renderer${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(TwoDimensional PRIVATE "helsinki-engine${LIB_EXTENSION_SHARED}")

SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
	TwoDimensional
	vk-bootstrap::vk-bootstrap
	GPUOpen::VulkanMemoryAllocator
)

if(ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( TwoDimensionalConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/TwoDimensionalConfig.hpp")

helsinkiApplication("HelsinkiTwoDimensionalExample" "TwoDimensional.cpp" "${MODULE_LIBS}")
