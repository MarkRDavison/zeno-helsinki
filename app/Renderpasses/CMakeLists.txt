file(GLOB_RECURSE RENDERPASSES_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(Renderpasses STATIC ${RENDERPASSES_SOURCES})
target_include_directories(Renderpasses PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(Renderpasses PRIVATE "helsinki-system${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(Renderpasses PRIVATE "helsinki-renderer${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(Renderpasses PRIVATE "helsinki-engine${LIB_EXTENSION_SHARED}")

SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
	Renderpasses
	vk-bootstrap::vk-bootstrap
	GPUOpen::VulkanMemoryAllocator
	imgui
)

if(ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( RenderpassesConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/RenderpassesConfig.hpp")

helsinkiApplication("HelsinkiRenderpassesExample" "Renderpasses.cpp" "${MODULE_LIBS}")
