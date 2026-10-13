include(GenerateExportHeader)
include(GNUInstallDirs)

function(helsinki_apply_target_options target_name)
	set_property(TARGET ${target_name} PROPERTY CXX_STANDARD ${HELSINKI_CXX_STANDARD})
	set_property(TARGET ${target_name} PROPERTY CXX_STANDARD_REQUIRED ON)
	if(MSVC)
		target_compile_options(${target_name} PRIVATE /Zc:__cplusplus)
	endif()
	if(HELSINKI_WARNING_AS_ERROR)
		set_property(TARGET ${target_name} PROPERTY COMPILE_WARNING_AS_ERROR ON)
	endif()
	target_compile_definitions(${target_name} PUBLIC $<$<CONFIG:Debug>:HELSINKI_DEBUG>)
	if(HELSINKI_ENABLE_TRACY)
		target_compile_definitions(${target_name} PUBLIC HELSINKI_TRACY_ENABLE TRACY_ENABLE)
	endif()
	if(HELSINKI_ENABLE_ASAN)
		if(MSVC)
			target_compile_options(${target_name} PRIVATE /fsanitize=address /Zi /Od /MDd /EHsc)
			target_link_options(${target_name} PRIVATE /fsanitize=address /INCREMENTAL:NO)
		else()
			target_compile_options(${target_name} PRIVATE -fsanitize=address -O1 -g)
			target_link_options(${target_name} PRIVATE -fsanitize=address)
		endif()
	endif()
endfunction()

FUNCTION(submodule MODULE_NAME MODULE_LIBS INCLUDE_DIRS)
	STRING(TOLOWER ${MODULE_NAME} MODULE_NAME_LOWER)
	STRING(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)
	SET(LIB_NAME "helsinki-${MODULE_NAME_LOWER}")

	SET(LIBRARY_MODULE_PATH "${PROJECT_SOURCE_DIR}/lib/${MODULE_NAME_LOWER}")
	SET(LIBRARY_SRC_PATH "${LIBRARY_MODULE_PATH}/src" )
	SET(LIBRARY_INCLUDE_PATH "${LIBRARY_MODULE_PATH}/include")

	FILE(GLOB_RECURSE LIB_HEADER_FILES "${LIBRARY_INCLUDE_PATH}/helsinki/${MODULE_NAME}/*.hpp")
	FILE(GLOB_RECURSE LIB_INLINE_FILES "${LIBRARY_INCLUDE_PATH}/helsinki/${MODULE_NAME}/*.inl")
	FILE(GLOB_RECURSE LIB_SOURCE_FILES "${LIBRARY_SRC_PATH}/*.cpp")

	SET(LIB_HEADER_FILES ${LIB_HEADER_FILES} ${LIB_INLINE_FILES})

	add_library(${LIB_NAME} ${LIB_SOURCE_FILES} ${LIB_HEADER_FILES})
	add_library(helsinki::${MODULE_NAME_LOWER} ALIAS ${LIB_NAME})
	set_property(TARGET ${LIB_NAME} PROPERTY DEBUG_POSTFIX "-d")
	set_property(TARGET ${LIB_NAME} PROPERTY EXPORT_NAME ${MODULE_NAME_LOWER})
	helsinki_apply_target_options(${LIB_NAME})

	set(_helsinki_export_header "${CMAKE_BINARY_DIR}/include/helsinki/${MODULE_NAME}/Export.hpp")
	generate_export_header(${LIB_NAME}
		BASE_NAME HELSINKI_${MODULE_NAME_UPPER}
		EXPORT_FILE_NAME "${_helsinki_export_header}")
	if(NOT BUILD_SHARED_LIBS)
		target_compile_definitions(${LIB_NAME} PUBLIC HELSINKI_${MODULE_NAME_UPPER}_STATIC_DEFINE)
	endif()

	target_include_directories(${LIB_NAME} PUBLIC
		$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
		$<BUILD_INTERFACE:${LIBRARY_INCLUDE_PATH}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

	if (HELSINKI_STRICT)
		if (MSVC)
			target_compile_options(${LIB_NAME} PRIVATE /W4 /wd4251 /wd4275)
		else()
			target_compile_options(${LIB_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-missing-field-initializers)
		endif()
	elseif(MSVC)
		target_compile_options(${LIB_NAME} PRIVATE /wd4251 /wd4275)
	endif()

	foreach(liblib IN LISTS MODULE_LIBS)
		TARGET_LINK_LIBRARIES(${LIB_NAME} PUBLIC ${liblib})
	endforeach()
	foreach(dir IN LISTS INCLUDE_DIRS)
		target_include_directories(${LIB_NAME} PUBLIC $<BUILD_INTERFACE:${dir}>)
	endforeach()

	install(TARGETS ${LIB_NAME}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

	if(BUILD_SHARED_LIBS AND MSVC)
		install(FILES $<TARGET_PDB_FILE:${LIB_NAME}> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
	endif()

	install(
		DIRECTORY "${LIBRARY_INCLUDE_PATH}/helsinki/${MODULE_NAME}/"
		DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/helsinki/${MODULE_NAME}"
		FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl")
	install(FILES "${_helsinki_export_header}"
		DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/helsinki/${MODULE_NAME}")
ENDFUNCTION()

ADD_SUBDIRECTORY(system)
ADD_SUBDIRECTORY(audio)
ADD_SUBDIRECTORY(ui)
ADD_SUBDIRECTORY(physics)
ADD_SUBDIRECTORY(scripting)
ADD_SUBDIRECTORY(renderer)
ADD_SUBDIRECTORY(engine)
ADD_SUBDIRECTORY(imgui)

function(helsinki_deploy_app_data target)
	set(_helsinki_app_data "${CMAKE_CURRENT_SOURCE_DIR}/data")
	if(NOT EXISTS "${_helsinki_app_data}")
		return()
	endif()
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${_helsinki_app_data}"
			"$<TARGET_FILE_DIR:${target}>/data"
		COMMENT "Deploy app data next to ${target}")
endfunction()

function(helsinki_deploy_renderer_shaders target)
	if(NOT HELSINKI_RENDERER_SHADER_SOURCE_DIR)
		message(FATAL_ERROR "helsinki_deploy_renderer_shaders: renderer shaders are not configured")
	endif()
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${HELSINKI_RENDERER_SHADER_SOURCE_DIR}"
			"$<TARGET_FILE_DIR:${target}>/data/shaders"
		COMMENT "Deploy Helsinki shaders to data/shaders next to ${target}")
endfunction()
