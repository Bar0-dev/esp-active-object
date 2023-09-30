idf_component_register(SRCS "button_ao.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver
                    PRIV_REQUIRES esp_ao)