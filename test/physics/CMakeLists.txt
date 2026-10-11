SET(MODULE_LIBS
	"Catch2::Catch2WithMain"
	"helsinki-physics"
)

testsubmodule("physics" "${MODULE_LIBS}")
