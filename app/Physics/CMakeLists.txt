file(GLOB_RECURSE PHYSICS_APP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(PhysicsApp STATIC ${PHYSICS_APP_SOURCES})
target_include_directories(PhysicsApp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(PhysicsApp PRIVATE "helsinki-system")
TARGET_LINK_LIBRARIES(PhysicsApp PRIVATE "helsinki-renderer")
TARGET_LINK_LIBRARIES(PhysicsApp PRIVATE "helsinki-engine")
TARGET_LINK_LIBRARIES(PhysicsApp PRIVATE "helsinki-physics")
TARGET_LINK_LIBRARIES(PhysicsApp PRIVATE "helsinki-imgui")

SET(MODULE_LIBS
	"helsinki-system"
	"helsinki-renderer"
	"helsinki-engine"
	"helsinki-physics"
	"helsinki-imgui"
	PhysicsApp
)

if(HELSINKI_ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE(PhysicsConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/PhysicsConfig.hpp")

helsinkiApplication("HelsinkiPhysicsExample" "Physics.cpp" "${MODULE_LIBS}")
