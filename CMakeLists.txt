cmake_minimum_required(VERSION 3.16)
project(StockChart VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts Network)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/SettingsDialog.cpp
    src/StockDataProvider.cpp
    src/AlphaVantageProvider.cpp
    src/FinnhubProvider.cpp
    src/PolygonProvider.cpp
    src/TwelveDataProvider.cpp
)

set(HEADERS
    src/MainWindow.h
    src/SettingsDialog.h
    src/StockDataProvider.h
    src/AlphaVantageProvider.h
    src/FinnhubProvider.h
    src/PolygonProvider.h
    src/TwelveDataProvider.h
)

add_executable(StockChart ${SOURCES} ${HEADERS})

target_link_libraries(StockChart PRIVATE
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
)
