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
    radar.add_plane("Plane1", Vector(1, 2, 3), Vector(100, 0, 0));
    radar.add_plane("Plane2", Vector(7, 8, 9), Vector(0, 100, 0));
    radar.add_plane("Plane3", Vector(13, 14, 15), Vector(0, 0, 100));
    radar.add_plane("Plane4", Vector(19, 20, 21), Vector(-100, -100, -100));
    read_planes(radar);

    // Simulation runs for 30 seconds
    sleep(30);

    // Stop all systems
    radar.stop();
    computerSystem.stop();

    // Detach connections
    ConnectDetach(computerSystemRadarCoid);

    return 0;
}

//read and create planes from planes.txt file
//IDs are hardcoded for now, they are in the .txt file.
void read_planes(Radar& radar) {
	std::string filePath = "./planes.txt";
	std::ifstream plane_file;
	plane_file.open(filePath);
	Vector Position;
	Vector Velocity;
	std::string ID;


	if(!plane_file.is_open()){
		std::cerr << "Error: Could not open file " << filePath << std::endl;
	}

	std::string line;
	    while (std::getline(plane_file, line, ';'))
	    {  // Split each line by semicolons
	    		std::vector<std::string> row;
	    		std::stringstream ss(line);
	    		std::string value;

	    	        // Split each entry in the line by commas
	    	        while (std::getline(ss, value, ',')) {
	    	            row.push_back(value);
	    	        }
	    	    ID = row[0];
	    	    Position = {std::stof(row[1]), std::stof(row[2]), std::stof(row[3])};
	    	    Velocity = {std::stof(row[4]), std::stof(row[5]), std::stof(row[6])};
	    	    std::cout<< row[0] << std::endl;
	    	    radar.add_plane(ID, Velocity, Position);
	    }

	    plane_file.close();
}


