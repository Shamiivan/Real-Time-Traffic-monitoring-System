// main.cpp
#include <iostream>
#include <vector>
#include <unistd.h> // For sleep
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include "plane.h"
#include "radar.h"
#include "computersystem.h"
#include "vector.h"
#include "messages.h"

int main() {
    // Create ComputerSystem
    ComputerSystem computerSystem;
    computerSystem.start();

    // Get the channel IDs for Radar and OperatorConsole
    int computerSystemRadarCoid = ConnectAttach(ND_LOCAL_NODE, 0, computerSystem.getRadarChannelId(), _NTO_SIDE_CHANNEL, 0);
    if (computerSystemRadarCoid == -1) {
        perror("Failed to connect to ComputerSystem radar channel");
        return -1;
    }

    // Create Radar and connect to ComputerSystem
    Radar radar(computerSystemRadarCoid);
    radar.start();

    // Add planes
    radar.add_plane("Plane1", Vector(1, 2, 3), Vector(100, 0, 0));
    radar.add_plane("Plane2", Vector(7, 8, 9), Vector(0, 100, 0));
    radar.add_plane("Plane3", Vector(13, 14, 15), Vector(0, 0, 100));
    radar.add_plane("Plane4", Vector(19, 20, 21), Vector(-100, -100, -100));

    // Simulation runs for 30 seconds
    sleep(30);

    // Stop all systems
    radar.stop();
    computerSystem.stop();

    // Detach connections
    ConnectDetach(computerSystemRadarCoid);

    return 0;
}


