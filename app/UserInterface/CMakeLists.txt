file(GLOB_RECURSE USER_INTERFACE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(UserInterface STATIC ${USER_INTERFACE_SOURCES})
target_include_directories(UserInterface PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(UserInterface PRIVATE "helsinki-system")
TARGET_LINK_LIBRARIES(UserInterface PRIVATE "helsinki-ui")
TARGET_LINK_LIBRARIES(UserInterface PRIVATE "helsinki-renderer")
TARGET_LINK_LIBRARIES(UserInterface PRIVATE "helsinki-engine")

SET(MODULE_LIBS
	"helsinki-system"
	"helsinki-ui"
	"helsinki-renderer"
	"helsinki-engine"
	UserInterface
)

if(HELSINKI_ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( UserInterfaceConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/UserInterfaceConfig.hpp")

helsinkiApplication("HelsinkiUserInterfaceExample" "UserInterface.cpp" "${MODULE_LIBS}")
