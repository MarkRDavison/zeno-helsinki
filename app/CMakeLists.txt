FUNCTION(helsinkiApplication AppMain CPP_NAME MODULE_LIBS)
	ADD_EXECUTABLE(${AppMain} ${CPP_NAME})
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

	FOREACH(library IN LISTS MODULE_LIBS)
		TARGET_LINK_LIBRARIES(${AppMain} PRIVATE ${library})
	ENDFOREACH()

ENDFUNCTION()

ADD_SUBDIRECTORY(TwoDimensional)
ADD_SUBDIRECTORY(Skeleton)