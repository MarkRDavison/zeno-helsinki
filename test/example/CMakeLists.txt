SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-example${LIB_EXTENSION_SHARED}"
)

testsubmodule("example" "${MODULE_LIBS}")