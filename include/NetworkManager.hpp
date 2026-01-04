// include/NetworkManager.hpp
#pragma once

#include <SFML/Network.hpp>
#include <iostream>
#include <array>
#include <optional>

// Define a common buffer size for sending/receiving
constexpr std::size_t DEFAULT_BUFFER_SIZE = 256;
using NetworkDataBuffer = std::array<char, DEFAULT_BUFFER_SIZE>;

class NetworkManager {
public:
    // Constructor (we will let the socket initialize here)
    NetworkManager();

    // Method to bind the socket to a specific port (used by the Server)
    bool bind(unsigned short port);

    // Method to send raw data
    sf::Socket::Status send(
        const NetworkDataBuffer& buffer, 
        std::size_t size, 
        const sf::IpAddress& address, 
        unsigned short port
    );

    // Method to receive raw data
    sf::Socket::Status receive(
        NetworkDataBuffer& buffer, 
        std::size_t& receivedSize, 
        std::optional<sf::IpAddress>& senderAddress, 
        unsigned short& senderPort
    );

private:
    sf::UdpSocket m_socket;
};
