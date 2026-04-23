idf_component_register(
    SRCS
        ""
    INCLUDE_DIRS
        "include"
        "include/architectureabs"
        "include/logger"
    REQUIRES
        ais_framework
)

target_link_libraries(${COMPONENT_LIB} INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/src/libcommon.a"
)