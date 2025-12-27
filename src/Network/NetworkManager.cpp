// src/NetworkManager.cpp
#include "../include/NetworkManager.hpp"

// We must include the optional header here as well
#include <optional> 
#include <cstring> // For memcpy/memset if needed, but not strictly required here

NetworkManager::NetworkManager() {
    // We can set some default properties here if needed
    // Example: set a common timeout for all operations
    // m_socket.setReceiveTimeout(sf::milliseconds(500)); 
}

bool NetworkManager::bind(unsigned short port) {
    // Bind the internal socket
    if (m_socket.bind(port) != sf::Socket::Status::Done) {
        std::cerr << "Manager: Error binding socket to port " << port << "." << std::endl;
        return false;
    }
    m_socket.setBlocking(false); // Default to non-blocking for simplicity
    std::cout << "Manager: Socket bound to port " << port << "." << std::endl;
    return true;
}

sf::Socket::Status NetworkManager::send(
    const NetworkDataBuffer& buffer, 
    std::size_t size, 
    const sf::IpAddress& address, 
    unsigned short port) 
{
    // Use the raw SFML send function
    return m_socket.send(
        buffer.data(), // pointer to array data
        size,          // actual size to send
        address,
        port
    );
}

sf::Socket::Status NetworkManager::receive(
    NetworkDataBuffer& buffer, 
    std::size_t& receivedSize, 
    std::optional<sf::IpAddress>& senderAddress, 
    unsigned short& senderPort) 
{
    // Use the SFML 3.0.2 receive function with std::optional
    return m_socket.receive(
        buffer.data(),
        buffer.size(),
        receivedSize,
        senderAddress, // std::optional<sf::IpAddress>&
        senderPort
    );
}