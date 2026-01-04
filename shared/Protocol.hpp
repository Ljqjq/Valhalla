#pragma once
#include <cstdint>

#pragma pack(push, 1) // Це гарантує, що розмір буде однаковим всюди
struct PlayerInfo {
    uint32_t id;          // 4 байти
    float x, y;           // 8 байтів
    int32_t hp;           // 4 байти
    uint8_t isSpaceActive;// 1 байт
    uint8_t padding[3];   // 3 байти для вирівнювання до 20 байтів на гравця
};

struct GameState {
    uint32_t playerCount;      // 4 байти
    PlayerInfo players[6];     // 6 гравців * 20 байтів = 120 байтів
}; // Разом: 124 байти
#pragma pack(pop)

struct ClientInput {
    uint8_t moveUp, moveDown, moveLeft, moveRight;
    uint8_t pressSpace; 
    uint8_t pressShift;
};
