SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
	"helsinki-example${LIB_EXTENSION_SHARED}"
)

SET(MODULE_LIBS_POST
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
	"helsinki-example${LIB_EXTENSION_SHARED}"
	vk-bootstrap::vk-bootstrap
	GPUOpen::VulkanMemoryAllocator
)

CONFIGURE_FILE( ExampleConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/ExampleConfig.hpp")

helsinkiApplication("HelsinkiExample" "example.cpp" "${MODULE_LIBS}")


SET(AppMain "PostProcessingExample" )
ADD_EXECUTABLE(${AppMain} 
	"PostProcessing.cpp" 
	"PostProcess/vk_descriptors.cpp" 
	"PostProcess/vkInit.cpp" 
	"PostProcess/vk_images.cpp"
	"PostProcess/vk_pipelines.cpp"
	"PostProcess/VulkanEngine.cpp")
SET_PROPERTY(TARGET ${AppMain} PROPERTY CXX_STANDARD ${HELSINKI_CXX_STANDARD})
SET_PROPERTY(TARGET ${AppMain} PROPERTY CXX_STANDARD_REQUIRED ON)
	
#if (HELSINKI_STRICT)
#	if (MSVC)
#		# warning level 4
#		target_compile_options(${AppMain} PRIVATE /W4)
#	else()
#		# additional warnings
#		target_compile_options(${AppMain} PRIVATE -Wall -Wextra -Wpedantic)
#	endif()
#endif()

FOREACH(library IN LISTS MODULE_LIBS_POST)
	TARGET_LINK_LIBRARIES(${AppMain} PRIVATE ${library})
ENDFOREACH()
