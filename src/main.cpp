// main.cpp
#include <iostream>
#include <vector>
#include <unistd.h> // For sleep
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include "plane.h"
#include "radar.h"
#include "ComputerSystem.h"
#include "DataDisplay.h"
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

    int computerSystemDataDisplayCoid = ConnectAttach(ND_LOCAL_NODE, 0, computerSystem.getDataDisplayChannelId(), _NTO_SIDE_CHANNEL, 0);
    if (computerSystemDataDisplayCoid == -1) {
    	perror("Failed to connect to ComputerSystem DataDisplay channel");
        return -1;
    }

    // Create Radar and connect to ComputerSystem
    Radar radar(computerSystemRadarCoid);
    radar.start();

    // Create DataDisplay and connect to ComputerSystem
        DataDisplay dataDisplay(computerSystemDataDisplayCoid);
        dataDisplay.start();

    // Add planes
    radar.add_plane("#1", Vector(1, 10, 3), Vector(1, 0, 0));
    radar.add_plane("#2", Vector(1, 2, 9), Vector(1, 0, 0));
    // Uncomment if needed
    // radar.add_plane("#3", Vector(13, 14, 15), Vector(0, 0, 100));
    // radar.add_plane("#4", Vector(19, 20, 21), Vector(-100, -100, -100));

    // Simulation runs for 30 seconds
    sleep(30);

    // Stop all systems
    radar.stop();
    dataDisplay.stop();
    computerSystem.stop();


    // Detach connections
    ConnectDetach(computerSystemRadarCoid);
    ConnectDetach(computerSystemDataDisplayCoid);

    return 0;
}

