# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS	"main.cpp"
			"Dot.h" "Dot.cpp"
			"Animations.h" "Animations.cpp"
			"CardReader.h" "CardReader.cpp"
			"CardReaderEventHandler.h" "CardReaderEventHandler.cpp"
			"BuildConfig.h" )
			
set(COMPONENT_ADD_INCLUDEDIRS ".")

set(COMPONENT_EMBED_TXTFILES ipcertdummy.crt ipcertdummy.key)

register_component()

component_compile_options(-std=gnu++17)
