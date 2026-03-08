set(SRCS
    "src/WifiManager.cpp"
    "src/WebServer.cpp"
    "src/ConfigManager.cpp"
    "src/MoistureSensor.cpp"
    "main.cpp"
)

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "inc"
    EMBED_FILES "index.html"
    PRIV_REQUIRES
        esp_wifi
        esp_http_server
        nvs_flash
        driver
        esp_netif
        esp_event
        esp_timer
        json
        esp_adc
)
