SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-audio"
)

testsubmodule("audio" "${MODULE_LIBS}")
