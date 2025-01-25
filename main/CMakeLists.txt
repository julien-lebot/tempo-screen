set(srcs
    "app_ui.cpp"
    "color_manip.cpp"
    "main.cpp"
    "mqtt.cpp"
    "wifi.cpp"
    "data_model/electricity_rate_provider.cpp"
    "data_model/home_assistant_electricity_rate_provider.cpp"
    "images/wifi.c"
    "fonts/CourierPrimeRegular14.c"
    "fonts/CourierPrimeBold40.c"
    "fonts/CourierPrimeBold14.c"
)

idf_component_register(SRCS ${srcs}
    INCLUDE_DIRS "." ${IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos ${IDF_PATH}/components/freertos/FreeRTOS-Kernel-SMP/include/freertos
)

target_compile_options(${COMPONENT_LIB} INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-std=c++20>)
