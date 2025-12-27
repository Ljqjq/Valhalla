cmake_minimum_required(VERSION 3.10)
project(Valhalla)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML 3
find_package(SFML 3 REQUIRED COMPONENTS Graphics Network Window System)

# Define where headers are
include_directories(include shared)

# --- SERVER ---
# Explicitly listing sources to avoid confusion
set(SERVER_SOURCES 
    src/server_main.cpp 
    src/Network/NetworkManager.cpp
)

add_executable(ManagedServer ${SERVER_SOURCES})
target_link_libraries(ManagedServer SFML::Network SFML::System)

# --- CLIENT ---
set(CLIENT_SOURCES 
    src/client_main.cpp 
    src/Network/NetworkManager.cpp
)

add_executable(ManagedClient ${CLIENT_SOURCES})
target_link_libraries(ManagedClient SFML::Graphics SFML::Network SFML::Window SFML::System)
