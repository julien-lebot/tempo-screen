set(srcs
    "app_ui.cpp"
    "main.cpp"
    "mqtt.cpp"
    "wifi.cpp"
)

idf_component_register(SRCS ${srcs}
    INCLUDE_DIRS "." ${IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos ${IDF_PATH}/components/freertos/FreeRTOS-Kernel-SMP/include/freertos
)
