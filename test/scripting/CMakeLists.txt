SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-scripting"
)

testsubmodule("scripting" "${MODULE_LIBS}")
