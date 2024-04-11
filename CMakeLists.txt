# CMakeLists.txt of ESP32ENCODER component

# Src files
file(GLOB_RECURSE ESP32ENCODER_SRCS
    src/*.c
    src/*.cpp
)
# Include
set(ESP32ENCODER_INCS
    src/
)

# Public component requirement
set(ESP32ENCODER_REQUIRES
    driver
    esp_timer
)

# Private component requirement
set(ESP32ENCODER_PRIV_REQUIRES
)

# Register component
idf_component_register(
    SRCS ${ESP32ENCODER_SRCS}
    INCLUDE_DIRS ${ESP32ENCODER_INCS}
    REQUIRES ${ESP32ENCODER_REQUIRES}
    PRIV_REQUIRES ${ESP32ENCODER_PRIV_REQUIRES}
)
