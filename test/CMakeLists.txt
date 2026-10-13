function(testsubmodule MODULE_NAME MODULE_LIBS)
	SET(TestMain "${MODULE_NAME}-test")

	FILE(GLOB_RECURSE LIB_TEST_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

	ADD_EXECUTABLE(${TestMain} ${LIB_TEST_FILES})
	helsinki_apply_target_options(${TestMain})

	TARGET_LINK_LIBRARIES(${TestMain} PRIVATE FakeIt::FakeIt-catch)

	foreach(liblib IN LISTS MODULE_LIBS)
		TARGET_LINK_LIBRARIES(${TestMain} PRIVATE ${liblib})
	endforeach()

	include(CTest)
	include("${PROJECT_SOURCE_DIR}/cmake/ParseAndAddCatchTests.cmake")
	catch_discover_tests(${TestMain})
endfunction()

ADD_SUBDIRECTORY(system)
ADD_SUBDIRECTORY(audio)
ADD_SUBDIRECTORY(ui)
ADD_SUBDIRECTORY(physics)
ADD_SUBDIRECTORY(scripting)
ADD_SUBDIRECTORY(renderer)
if(TARGET TowerDefense)
	ADD_SUBDIRECTORY(towerdefense)
endif()
if(TARGET Driller)
	ADD_SUBDIRECTORY(driller)
endif()
if(TARGET Solar)
	ADD_SUBDIRECTORY(solar)
endif()