// messages.h
#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include <vector>
#include "vector.h"

// Message from Radar to Plane
struct RadarQueryMsg {
    int aircraft_id; // Optional
};

// Message from Plane to Radar
struct PlaneResponseMsg {
    struct {
        char id[16]; // Aircraft ID
        double x, y, z;
        double speedX, speedY, speedZ;
    } data;
};

// Structure to represent plane state
struct PlaneState {
    std::string id;
    Vector position;
    Vector velocity;
};

// Message from Radar to ComputerSystem
struct RadarToComputerMsg {
    int numAircraft;
    PlaneState aircraftData[100]; // Adjust size as needed
};

// Operator commands
struct OperatorCommand {
    enum CommandType {
        SET_LOOKAHEAD_TIME,
        SEND_TO_PLANE
    } type;

    // For SET_LOOKAHEAD_TIME
    int lookaheadTime;

    // For SEND_TO_PLANE
    char aircraftId[16];
    double speedX, speedY, speedZ;
};

// Message from OperatorConsole to ComputerSystem
struct OperatorCommandMsg {
    OperatorCommand command;
};

// Message from ComputerSystem to CommunicationSystem
struct CommunicationCommandMsg {
    char aircraftId[16];
    double speedX, speedY, speedZ;
};

#endif // MESSAGES_H
