SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-audio${LIB_EXTENSION_SHARED}"
)

testsubmodule("audio" "${MODULE_LIBS}")
