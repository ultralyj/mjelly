idf_component_register(SRCS "wireless.c" "phy.c" "tcp_client.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "esp_wifi nvs_flash lwip esp_netif esp_event")
