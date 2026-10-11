SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-system"
)

testsubmodule("system" "${MODULE_LIBS}")