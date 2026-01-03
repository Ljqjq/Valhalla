#include <iostream>
#include <cmath>
#include <map>
#include <cstring>
#include <optional>
#include <SFML/System.hpp>
#include "../include/NetworkManager.hpp"
#include "../shared/Protocol.hpp"

struct ServerPlayerData {
    float x = 400.0f;
    float y = 300.0f;
    float vx = 0.0f;
    float vy = 0.0f;

    int32_t hp = 100;
    sf::IpAddress ip = sf::IpAddress::LocalHost;
};

int main() {
    NetworkManager net;
    if (!net.bind(5000)) {
        std::cerr << "[ERROR] Cannot bind to port 5000" << std::endl;
        return -1;
    }

    std::map<unsigned short, ServerPlayerData> playerMap;
    NetworkDataBuffer recvBuf;
    std::size_t size;
    std::optional<sf::IpAddress> senderIp;
    unsigned short senderPort;

    std::cout << "--- SERVER PROTOCOL CHECK ---" << std::endl;
    std::cout << "MAX_PLAYERS: " << MAX_PLAYERS << std::endl;
    std::cout << "GameState size: " << sizeof(GameState) << std::endl;
    std::cout << "-----------------------------" << std::endl;

    std::cout << "[SERVER] Valhalla Server Started on Port 5000" << std::endl;

    while (true) {
        while (net.receive(recvBuf, size, senderIp, senderPort) == sf::Socket::Status::Done) {
            if (size >= sizeof(ClientInput)) {
                ClientInput input;
                std::memcpy(&input, recvBuf.data(), sizeof(ClientInput));

                if (playerMap.find(senderPort) == playerMap.end()) {
                    std::cout << "[NEW PLAYER] Port: " << senderPort << " from IP: " << senderIp->toString() << std::endl;
                    playerMap[senderPort] = {400.0f, 300.0f, 0.0f, 0.0f, 100, senderIp.value()};
                }

                float acceleration = 1.0f;
                float friction = 0.9f;

                if (input.moveUp && playerMap[senderPort].vy > -5.0f)    playerMap[senderPort].vy -= acceleration;
                if (input.moveDown && playerMap[senderPort].vy < 5.0f)  playerMap[senderPort].vy += acceleration;
                if (input.moveLeft && playerMap[senderPort].vx > -5.0f)  playerMap[senderPort].vx -= acceleration;
                if (input.moveRight && playerMap[senderPort].vx < 5.0f) playerMap[senderPort].vx += acceleration;

                playerMap[senderPort].vy *= friction;
                playerMap[senderPort].vx *= friction;

                playerMap[senderPort].x += playerMap[senderPort].vx;
                playerMap[senderPort].y += playerMap[senderPort].vy;

                if (playerMap[senderPort].x < 20)  { playerMap[senderPort].x = 20;  playerMap[senderPort].vx *= -0.5f; }
                if (playerMap[senderPort].x > 780) { playerMap[senderPort].x = 780; playerMap[senderPort].vx *= -0.5f; }
                if (playerMap[senderPort].y < 20)  { playerMap[senderPort].y = 20;  playerMap[senderPort].vy *= -0.5f; }
                if (playerMap[senderPort].y > 580) { playerMap[senderPort].y = 580; playerMap[senderPort].vy *= -0.5f; }
            }
        }

        // Збірка стану світу
        GameState worldState = {0};
        uint32_t count = 0;
        for (auto const& [port, data] : playerMap) {
            if (count < MAX_PLAYERS) {
                worldState.players[count] = { (uint32_t)port, data.x, data.y, data.hp };
                count++;
            }
        }
        worldState.playerCount = count;
        
        // 2. Collision Detection (Simple Physics)
        float radius = 20.0f;
        float minDistance = radius * 2.0f; // 40.0f
        
        for (auto it1 = playerMap.begin(); it1 != playerMap.end(); ++it1) {
            for (auto it2 = std::next(it1); it2 != playerMap.end(); ++it2) {
                ServerPlayerData& p1 = it1->second;
                ServerPlayerData& p2 = it2->second;
        
                // Calculate distance between centers
                float dx = p2.x - p1.x;
                float dy = p2.y - p1.y;
                float distance = std::sqrt(dx * dx + dy * dy);
       
                if (distance < minDistance) {
                      float overlap = minDistance - distance;
                      float nx = dx / distance; // Нормаль зіткнення
                      float ny = dy / distance;


                      // ВІДСКІК: обмін швидкостями (спрощено)
                      std::swap(p1.vx, p2.vx);
                      std::swap(p1.vy, p2.vy);
                      
                      // Додаємо трохи імпульсу від удару
                      p1.vx -= nx * 2.0f; p1.vy -= ny * 2.0f;
                      p2.vx += nx * 2.0f; p2.vy += ny * 2.0f;
                }
            }
        }
        // Розсилка та логування (тільки якщо є гравці)
        if (count > 0) {
            NetworkDataBuffer sendBuf;
            std::memcpy(sendBuf.data(), &worldState, sizeof(GameState));
            for (auto const& [port, data] : playerMap) {
                net.send(sendBuf, sizeof(GameState), data.ip, port);
            }
            // Розкоментуй наступний рядок для дуже детального дебагу:
            // std::cout << "[BROADCAST] Sent state with " << count << " players." << std::endl;
        }

        sf::sleep(sf::milliseconds(16));
    }
    return 0;
}
