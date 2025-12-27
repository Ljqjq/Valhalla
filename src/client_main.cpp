// src/client_main.cpp - Final UDP Sender (SFML 3.0.2 Compatible)
#include <SFML/Network.hpp>
#include <iostream>
#include <array>
#include <string>
#include <optional> // REQUIRED for std::optional<sf::IpAddress>

constexpr unsigned short SERVER_PORT = 45678;
constexpr std::size_t ACK_BUFFER_SIZE = 100;

int main() {
    sf::UdpSocket socket;
    
    std::string message = "SFML 3.0.2 Client Test Message.";
    sf::IpAddress receiverAddress = sf::IpAddress::LocalHost; // Use LocalHost instead of string literal for 3.x safety

    // Sending logic remains simple
    socket.send(message.c_str(), message.size() + 1, receiverAddress, SERVER_PORT);
    std::cout << "CLIENT: Successfully sent message." << std::endl;

    // --- Wait for Acknowledgment (ACK) ---
    std::array<char, ACK_BUFFER_SIZE> ackBuffer; 
    std::size_t ackReceived = 0;
    
    // Declarations required by the SFML 3.0.2 receive function:
    std::optional<sf::IpAddress> ackSender; // NOTE: std::optional is REQUIRED here
    unsigned short ackPort = 0;

    socket.setBlocking(true);
    
    
    // Use the function name your compiler expects with the correct argument types
    if (socket.receive(ackBuffer.data(), ackBuffer.size(), ackReceived, ackSender, ackPort) == sf::Socket::Status::Done) {
        if (ackSender.has_value()) {
            std::string ackString(ackBuffer.data(), ackReceived);
            std::cout << "CLIENT ACK: " << ackString << std::endl;
        }
    } else {
        std::cout << "CLIENT: Did not receive acknowledgment within timeout." << std::endl;
    }

    return 0;
}