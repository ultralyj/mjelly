set(mlx90393    "mjd_mlx90393.c")

idf_component_register(SRCS "${mlx90393}"
                       INCLUDE_DIRS "include"
                       REQUIRES "driver")
