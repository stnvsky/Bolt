set(COMPONENT_SRCS
		"src/vl53l0x.cpp"
		"src/vl53l0x_init.cpp"
		"src/vl53l0x_io.cpp")

set(COMPONENT_ADD_INCLUDEDIRS "include")

set(COMPONENT_REQUIRES
		driver
		)

register_component()