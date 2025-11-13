SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-renderer${LIB_EXTENSION_SHARED}"
)

testsubmodule("renderer" "${MODULE_LIBS}")