file(GLOB_RECURSE PHYSICS_APP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(PhysicsApp STATIC ${PHYSICS_APP_SOURCES})
target_include_directories(PhysicsApp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(PhysicsApp PRIVATE "helsinki-system${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(PhysicsApp PRIVATE "helsinki-renderer${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(PhysicsApp PRIVATE "helsinki-engine${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(PhysicsApp PRIVATE "helsinki-physics${LIB_EXTENSION_SHARED}")

SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
	"helsinki-physics${LIB_EXTENSION_SHARED}"
	PhysicsApp
)

if(ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE(PhysicsConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/PhysicsConfig.hpp")

helsinkiApplication("HelsinkiPhysicsExample" "Physics.cpp" "${MODULE_LIBS}")
