set(COMPONENT_SRCS
        "src/motors_driver.cpp")
set(COMPONENT_ADD_INCLUDEDIRS "include")

set(COMPONENT_REQUIRES
        driver
        )

register_component()