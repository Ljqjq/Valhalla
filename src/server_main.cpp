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
                    playerMap[senderPort] = {400.0f, 300.0f, 100, senderIp.value()};
                }

                float speed = 5.0f;
                if (input.moveUp)    playerMap[senderPort].y -= speed;
                if (input.moveDown)  playerMap[senderPort].y += speed;
                if (input.moveLeft)  playerMap[senderPort].x -= speed;
                if (input.moveRight) playerMap[senderPort].x += speed;
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
                    // Collision detected! 
                    // Resolve: Push them away from each other
                    float overlap = minDistance - distance;
                    
                    // Direction vector
                    float nx = dx / distance;
                    float ny = dy / distance;
        
                    // Move both balls back by half the overlap
                    p1.x -= nx * (overlap / 2.0f);
                    p1.y -= ny * (overlap / 2.0f);
                    p2.x += nx * (overlap / 2.0f);
                    p2.y += ny * (overlap / 2.0f);
                    
                    // Optional: Reduce HP on collision
                    // p1.hp -= 1; 
                    // p2.hp -= 1;
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
