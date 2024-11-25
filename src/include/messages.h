// messages.h
#ifndef MESSAGES_H
#define MESSAGES_H

#include "vector.h"

enum class ConsoleCommand {
    LIST_PLANES = 0,
    DISPLAY_PLANE_DATA = 1,
    UPDATE_PLANE_VELOCITY = 2,
    UPDATE_PLANE_POSITION = 3
};


// Structure to represent plane state
struct PlaneState {
    char id[16];
    Vector position;
    Vector velocity;
    int coid_comp;
};
// For sending multiple planes to console
struct PlaneListMsg {
    int numPlanes;
    PlaneState planes[50];  // Maximum 50 planes
};

struct courseCorrectionMsg {
	std::string id;
	Vector newVelocity;
};

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



// Message from Radar to ComputerSystem
struct RadarToComputerMsg {
    int numAircraft;
    PlaneState aircraftData[100]; // Adjust size as needed
    int coid_comp;
};

// Message from ComputerSystem to DataDisplay
struct ComputerToDataDisplayMsg {
    int numAircraft;
    PlaneState aircraftData[50]; // Maximum of 50 aircraft
};

// Operator commands
struct OperatorCommandMsg {
	ConsoleCommand type;
    // For SEND_TO_PLANE
    char planeId[16];
    Vector velocity;
    Vector position;
};

struct DataDisplayRequestMsg {
    bool requestAugmentedData;
    // Additional fields if needed
};
#endif // MESSAGES_H
