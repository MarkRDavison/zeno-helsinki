SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-ui${LIB_EXTENSION_SHARED}"
)

testsubmodule("ui" "${MODULE_LIBS}")
