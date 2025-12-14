set(SRCS
    "src/WifiManager.cpp"
    "src/WebServer.cpp"
    "src/ConfigManager.cpp"
    "src/TimeSync.cpp"
    "src/WeatherFetcher.cpp"
    "src/Quotes.cpp"
    "src/MAX7219.cpp"
    "src/Font5x7.cpp"
    "src/DisplayManager.cpp"
    "src/DisplayController.cpp"
    "main.cpp"
)

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "inc"
    EMBED_FILES "index.html"
    PRIV_REQUIRES
        esp_wifi
        esp_http_server
        esp_http_client
        nvs_flash
        driver
        esp_netif
        esp_event
        esp_timer
        json
        esp_sntp
)
