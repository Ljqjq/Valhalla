// src/server_main.cpp - Final UDP Receiver (SFML 3.0.2 Compatible)
#include <SFML/Network.hpp>
#include <iostream>
#include <array>
#include <string>
#include <optional> // REQUIRED for std::optional<sf::IpAddress>

constexpr unsigned short SERVER_PORT = 45678;
constexpr std::size_t MAX_BUFFER_SIZE = 100;

int main() {
    sf::UdpSocket socket;
    
    if (socket.bind(SERVER_PORT) != sf::Socket::Status::Done) {
        std::cerr << "SERVER: Error binding socket to port " << SERVER_PORT << ". Exiting." << std::endl;
        return 1;
    }
    socket.setBlocking(false); 
    std::cout << "SERVER: SFML 3.0.2 Listening on port " << SERVER_PORT << "..." << std::endl;

    // Declarations required by the SFML 3.0.2 receive function:
    std::array<char, MAX_BUFFER_SIZE> data; 
    std::size_t received = 0;
    std::optional<sf::IpAddress> sender; // NOTE: std::optional is REQUIRED here
    unsigned short port = 0;             // Initialize port

    while (true) {
        // Use the function name your compiler expects with the correct argument types
        sf::Socket::Status status = socket.receive(
            data.data(),
            data.size(),
            received,
            sender, // Passed as the required std::optional<IpAddress>&
            port
        );

        if (status == sf::Socket::Status::Done) {
            
            // 3.0.2 check: The address is inside the optional
            if (sender.has_value()) {
                 std::string receivedString(data.data(), received);
                 
                 std::cout << "\nSERVER RECEIVED data from " << sender.value().toString() << " on port " << port << std::endl;
                 std::cout << "Message: \"" << receivedString << "\"" << std::endl;

                 // Send ACK back using sender.value() to get the IpAddress object
                 std::string ack = "ACK: Received " + std::to_string(received) + " bytes.";
                 socket.send(ack.c_str(), ack.size() + 1, sender.value(), port); 
            } else {
                 std::cout << "SERVER RECEIVED data, but sender address was empty." << std::endl;
            }
            break; 
        }
        sf::sleep(sf::milliseconds(1));
    }

    return 0;
}