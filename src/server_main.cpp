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
    ClientInput lastInput;
    sf::Clock dashClock;
    sf::Clock spaceActivationTimer;  // ТАЙМЕР ДЛЯ ТОЧНОСТІ УДАРУ (Space)
    
    bool isSpaceActive = false;      // Чи активна зараз "біла аура"
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
    std::cout << "MAX_PLAYERS: " << "6" << std::endl;
    std::cout << "GameState size: " << sizeof(GameState) << std::endl;
    std::cout << "-----------------------------" << std::endl;

    std::cout << "[SERVER] Valhalla Server Started on Port 5000" << std::endl;

    while (true) {
              
        while (net.receive(recvBuf, size, senderIp, senderPort) == sf::Socket::Status::Done) {
            if (size >= sizeof(ClientInput)) {
                ClientInput incomingInput;
                std::memcpy(&incomingInput, recvBuf.data(), sizeof(ClientInput));

                // Якщо гравець новий — додаємо в мапу
                if (playerMap.find(senderPort) == playerMap.end()) {
                    std::cout << "[JOIN] Port: " << senderPort << std::endl;
                    // Ініціалізація: x, y, vx, vy, hp, ip, lastInput
                    playerMap[senderPort] = {400.0f, 300.0f, 0.0f, 0.0f, 100, senderIp.value(), incomingInput};
                } else {
                    // Оновлюємо лише стан клавіш для існуючого гравця
                    playerMap[senderPort].lastInput = incomingInput;
                }
            }
        }

        // --- 2. ФІЗИКА ТА РУХ (Physics Phase) ---
        float acceleration = 0.8f;
        float friction = 0.92f;
        float dashPower = 18.0f;

        for (auto& [port, p] : playerMap) {

            // Логіка DASH (Shift)
            if (p.lastInput.pressShift && p.dashClock.getElapsedTime().asSeconds() > 1.2f) {
              p.vx *= 3.5f; 
              p.vy *= 3.5f;
              p.dashClock.restart();
            }

            // 2. Стан SPACE (Блок/Удар)
            if (p.lastInput.pressSpace) {
                p.isSpaceActive = true;
                // Коли гравець ТІЛЬКИ НАТИСНУВ пробіл (в цей кадр), скидаємо таймер
                // Це дозволяє вирахувати точність таймінгу
            } else {
                p.isSpaceActive = false;
                p.spaceActivationTimer.restart(); 
            }
            
            // 3. Обмеження руху в стані Space
            if (!p.isSpaceActive) {
                if (p.lastInput.moveUp)    p.vy -= acceleration;
                if (p.lastInput.moveDown)  p.vy += acceleration;
                if (p.lastInput.moveLeft)  p.vx -= acceleration;
                if (p.lastInput.moveRight) p.vx += acceleration;
            }

            // Застосування тертя
            p.vx *= friction;
            p.vy *= friction;

            // Оновлення координат
            p.x += p.vx;
            p.y += p.vy;

            // Межі екрану (800x600) з відскоком
            if (p.x < 20)  { p.x = 20;  p.vx *= -0.5f; }
            if (p.x > 780) { p.x = 780; p.vx *= -0.5f; }
            if (p.y < 20)  { p.y = 20;  p.vy *= -0.5f; }
            if (p.y > 580) { p.y = 580; p.vy *= -0.5f; }
        }
        
        

        // Збірка стану світу
        GameState worldState = {0};
        uint32_t count = 0;
        for (auto const& [port, data] : playerMap) {
            if (count < 6) {
                worldState.players[count] = { (uint32_t)port, data.x, data.y, data.hp, data.isSpaceActive };
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

            // ЛОГУВАННЯ НА СЕРВЕРІ
        if (worldState.playerCount > 0) {
            std::cout << "[SERVER] Sending GameState | Size: " << sizeof(GameState) 
                      << " | Players: " << worldState.playerCount 
                      << " | P0 Pos: " << worldState.players[0].x << ", " << worldState.players[0].y 
                      << " | SpaceActive: " << (int)worldState.players[0].isSpaceActive << std::endl;
        }
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
