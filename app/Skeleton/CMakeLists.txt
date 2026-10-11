file(GLOB_RECURSE SKELETON_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(Skeleton STATIC ${SKELETON_SOURCES})
target_include_directories(Skeleton PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(Skeleton PRIVATE "helsinki-system")
TARGET_LINK_LIBRARIES(Skeleton PRIVATE "helsinki-renderer")
TARGET_LINK_LIBRARIES(Skeleton PRIVATE "helsinki-engine")

SET(MODULE_LIBS
	"helsinki-system"
	"helsinki-renderer"
	"helsinki-engine"
	Skeleton
)

if(HELSINKI_ENABLE_TRACY)
	list(APPEND MODULE_LIBS ThirdParty_Tracy)
endif()

CONFIGURE_FILE( SkeletonConfig.hpp.in "${CMAKE_CURRENT_LIST_DIR}/SkeletonConfig.hpp")

helsinkiApplication("HelsinkiSkeletonExample" "Skeleton.cpp" "${MODULE_LIBS}")
