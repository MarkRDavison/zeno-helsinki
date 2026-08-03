SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-system${LIB_EXTENSION_SHARED}"
	"Hurricane${LIB_EXTENSION_SHARED}"
)

testsubmodule("hurricane" "${MODULE_LIBS}")