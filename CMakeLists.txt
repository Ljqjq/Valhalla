cmake_minimum_required(VERSION 3.10)
project(ManagedSocketTest CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find SFML (3.0.2)
find_package(SFML 3.0 COMPONENTS System Network REQUIRED)

# 2. Define Include Directory for NetworkManager.hpp
# This allows CPP files to use #include "NetworkManager.hpp"
include_directories(include)

# 3. Define the NetworkManager source path
# NOTE: The implementation is now inside the src/Network folder
set(NETWORK_MANAGER_SRC src/Network/NetworkManager.cpp)

# 4. Define the two executables, linking the manager source to both
add_executable(ManagedServer 
    src/server_main.cpp 
    ${NETWORK_MANAGER_SRC}
)
add_executable(ManagedClient 
    src/client_main.cpp 
    ${NETWORK_MANAGER_SRC}
)

# 5. Link both to the required SFML components
target_link_libraries(ManagedServer PRIVATE 
    SFML::System 
    SFML::Network
)

target_link_libraries(ManagedClient PRIVATE 
    SFML::System 
    SFML::Network
)