SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-renderer"
)

testsubmodule("renderer" "${MODULE_LIBS}")