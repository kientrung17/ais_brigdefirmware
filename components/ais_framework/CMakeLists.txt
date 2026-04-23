idf_component_register(
    SRCS
        ""
    INCLUDE_DIRS
        "include"
        "include/HAL/HAL_ABSTRACT"
        "include/HAL/HAL_ESP32"
        "include/OSAL"
        "include/OSAL/ESP32"
        "include/OSAL/Platform"
    REQUIRES
        driver
        esp_event
        nvs_flash
        ais_commonlib
        esp_wifi
        esp_adc
)

target_link_libraries(${COMPONENT_LIB} INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/src/libframework.a"
)
