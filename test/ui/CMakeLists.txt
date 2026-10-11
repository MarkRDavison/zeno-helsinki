SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-ui"
)

testsubmodule("ui" "${MODULE_LIBS}")
