
#pragma once

#include <SFML/Network.hpp>

// --- 1. Enumeration of Packet Types ---
// This enum defines the "header" of every packet. 
// The recipient reads this first to know how to interpret the rest of the data.
enum PacketType : sf::Int32 {
    // A. CONNECTION & RELIABILITY
    CONNECT_REQUEST,    // Client -> Host: I want to join.
    CONNECT_ACCEPT,     // Host -> Client: You are in. Here is your Player ID.
    DISCONNECT,         // Both -> Both: Gracefully ending session.
    HEARTBEAT,          // Both -> Both: Used to keep connection alive and measure latency.
    
    // B. GAME DATA FLOW
    PLAYER_INPUT,       // Client -> Host: My current key/mouse/joystick state.
    GAME_STATE,         // Host -> Client: The authoritative position of all entities.
    
    // C. GAME MESSAGES (Future)
    PLAYER_HIT,         // Host -> Client: Player X took damage.
    CHAT_MESSAGE,       // Client -> Host: Send a chat message.
    
    // Keep this last
    PACKET_COUNT
};

// --- 2. Shared Constants ---
// Define the standard port used by the server.
constexpr unsigned short SERVER_PORT = 45678;

// --- 3. Custom Structures (Future: Defining the Data Payload) ---
// When your game state gets complex, you define structures here
// and overload the SFML operators so they can be easily sent/received.

// Example for future use: A small structure defining player input
// struct PlayerInput {
//     sf::Int32 sequenceNumber;
//     bool keyUp, keyDown, keyLeft, keyRight;
//     // ... other buttons
// };

// Example SFML Operator Overload (for future use)
// sf::Packet& operator<<(sf::Packet& packet, const PlayerInput& input);
// sf::Packet& operator>>(sf::Packet& packet, PlayerInput& input);