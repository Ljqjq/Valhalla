#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstring>
#include <optional>
#include "../include/NetworkManager.hpp"
#include "../shared/Protocol.hpp"

int main() {
    // ДРУКУЄМО РОЗМІРИ ДЛЯ ПЕРЕВІРКИ
    std::cout << "Expected GameState size: " << sizeof(GameState) << " bytes" << std::endl;

    sf::RenderWindow window(sf::VideoMode({800, 600}), "Valhalla - Debug");
    window.setFramerateLimit(60);

    NetworkManager net;
    net.bind(0);

    sf::IpAddress serverIp = sf::IpAddress::resolve("127.0.0.1").value();
    unsigned short serverPort = 5000;

    sf::CircleShape ball(20.f);
    ball.setOrigin({20.f, 20.f});
    GameState currentWorld = {0};

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        // 1. Send Input
        ClientInput input = {0};
        input.moveUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
        input.moveDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
        input.moveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
        input.moveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);

        NetworkDataBuffer sendBuf;
        std::memcpy(sendBuf.data(), &input, sizeof(ClientInput));
        net.send(sendBuf, sizeof(ClientInput), serverIp, serverPort);

        // 2. Receive World State
        NetworkDataBuffer recvBuf;
        std::size_t receivedSize;
        std::optional<sf::IpAddress> ip;
        unsigned short port;
        
        while (net.receive(recvBuf, receivedSize, ip, port) == sf::Socket::Status::Done) {
            if (receivedSize == sizeof(GameState)) {
                std::memcpy(&currentWorld, recvBuf.data(), sizeof(GameState));
            } else {
                // ЦЕЙ ЛОГ ПІДКАЖЕ НАМ ПРАВДУ:
                std::cerr << "[NET DEBUG] Size mismatch! Got: " << receivedSize 
                          << " Expected: " << sizeof(GameState) << std::endl;
            }
        }

        // 3. Render
        window.clear(sf::Color(30, 30, 35));
        for (uint32_t i = 0; i < currentWorld.playerCount; i++) {
            ball.setPosition({currentWorld.players[i].x, currentWorld.players[i].y});
            ball.setFillColor(sf::Color::Cyan);
            window.draw(ball);
        }
        window.display();
    }
    return 0;
}
