#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <cstdint>

// 4 (count) + 6 * 16 (players) = 100 bytes
#define MAX_PLAYERS 6 

#pragma pack(push, 1)
struct PlayerInfo {
    uint32_t id;   // 4 bytes
    float x;       // 4 bytes
    float y;       // 4 bytes
    int32_t hp;    // 4 bytes
};

struct GameState {
    uint32_t playerCount;              // 4 bytes
    PlayerInfo players[MAX_PLAYERS];    // 96 bytes
};

struct ClientInput {
    uint8_t moveUp, moveDown, moveLeft, moveRight, pressSpace;
};
#pragma pack(pop)

#endif
