SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-scripting${LIB_EXTENSION_SHARED}"
)

testsubmodule("scripting" "${MODULE_LIBS}")
