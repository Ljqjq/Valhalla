#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <cstring>
#include <optional>
#include "../include/NetworkManager.hpp"
#include "../shared/Protocol.hpp"

int main() {
    std::string ipString;
    std::cout << "Enter Server IP: ";
    std::cin >> ipString;

    auto serverIpOpt = sf::IpAddress::resolve(ipString);
    if (!serverIpOpt) {
        std::cerr << "Invalid IP address!" << std::endl;
        return -1;
    }
    sf::IpAddress serverIp = serverIpOpt.value();
    unsigned short serverPort = 5000;

    sf::RenderWindow window(sf::VideoMode({800, 600}), "Valhalla Online");
    window.setFramerateLimit(60);

    NetworkManager net;
    net.bind(0); // Рандомний порт для клієнта

    sf::CircleShape ball(20.f);
    ball.setOrigin({20.f, 20.f});
    ball.setFillColor(sf::Color::Cyan);

    GameState currentWorld = {0};

    std::cout << "[CLIENT] Connected to " << serverIp.toString() << ". Press WASD to move." << std::endl;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        // 1. Відправка інпуту
        ClientInput input = {0};
        if(window.hasFocus()) {
          input.moveUp    = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);      
          input.moveDown  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
          input.moveLeft  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
          input.moveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
          input.pressSpace = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
          input.pressShift = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift); 
        }
        else {
           input = {0, 0, 0, 0, 0, 0};  
        }        

        NetworkDataBuffer sendBuf;
        std::memcpy(sendBuf.data(), &input, sizeof(ClientInput));
        net.send(sendBuf, sizeof(ClientInput), serverIp, serverPort);

        // 2. Отримання стану світу
        NetworkDataBuffer recvBuf;
        std::size_t size;
        std::optional<sf::IpAddress> senderIp;
        unsigned short senderPort;
        
        while (net.receive(recvBuf, size, senderIp, senderPort) == sf::Socket::Status::Done) {
            if (size >= sizeof(GameState)) {
               std::memcpy(&currentWorld, recvBuf.data(), sizeof(GameState));

               // ЛОГУВАННЯ НА КЛІЄНТІ
               if (currentWorld.playerCount > 0) {
                   std::cout << "[CLIENT] Received GameState | Packet Size: " << size 
                             << " | Expected Size: " << sizeof(GameState)
                             << " | P0 ID: " << currentWorld.players[0].id
                             << " | P0 Pos: " << currentWorld.players[0].x << ", " << currentWorld.players[0].y 
                             << " | Space: " << (int)currentWorld.players[0].isSpaceActive << std::endl;
               }
           } else {
               std::cout << "[CLIENT] Warning: Packet too small! Received: " << size 
                         << " Expected: " << sizeof(GameState) << std::endl;
           }
        }

        // 3. Малювання
        window.clear(sf::Color(30, 30, 35));
        for (uint32_t i = 0; i < currentWorld.playerCount; i++) {
            ball.setPosition({currentWorld.players[i].x, currentWorld.players[i].y});
            window.draw(ball);
        }
        window.display();
    }
    return 0;
}
