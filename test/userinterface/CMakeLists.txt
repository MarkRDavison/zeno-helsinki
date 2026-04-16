SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-userinterface${LIB_EXTENSION_SHARED}"
)

testsubmodule("userinterface" "${MODULE_LIBS}")