SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-physics${LIB_EXTENSION_SHARED}"
)

testsubmodule("physics" "${MODULE_LIBS}")
