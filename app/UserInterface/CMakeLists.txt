file(GLOB_RECURSE USER_INTERFACE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(UserInterface STATIC ${USER_INTERFACE_SOURCES})
target_include_directories(UserInterface PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(UserInterface PRIVATE "helsinki-system${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(UserInterface PRIVATE "helsinki-renderer${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(UserInterface PRIVATE "helsinki-engine${LIB_EXTENSION_SHARED}")
TARGET_LINK_LIBRARIES(UserInterface PRIVATE "helsinki-userinterface${LIB_EXTENSION_SHARED}")

SET(MODULE_LIBS
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
	"helsinki-engine${LIB_EXTENSION_SHARED}"
	"helsinki-userinterface${LIB_EXTENSION_SHARED}"
	UserInterface
)

if(ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( UserInterfaceConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/UserInterfaceConfig.hpp")

helsinkiApplication("HelsinkiUserInterfaceExample" "UserInterface.cpp" "${MODULE_LIBS}")
