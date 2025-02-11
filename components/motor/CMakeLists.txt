idf_component_register(SRCS "src/motor.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "driver bdc_motor esp_timer esp_adc PID")
