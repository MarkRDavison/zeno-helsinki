SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-system${LIB_EXTENSION_SHARED}"
)

testsubmodule("system" "${MODULE_LIBS}")