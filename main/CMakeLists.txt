set(require_componet, "
motor
spi_flash
cmd_i2ctools
console
leds
mlx90393
i2c_mux
pid
console
mconsole
")
idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ""
                    REQUIRES "${require_componet}")
