idf_component_register(SRCS "mconsole.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "console driver motor bdc_motor mlx90393 i2c_mux leds led_strip")
