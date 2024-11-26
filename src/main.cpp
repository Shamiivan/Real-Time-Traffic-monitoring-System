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
#include <fstream>
#include <sstream>
#include "Logger.h"
#include "Console.h"


void read_planes(Radar&);

int main() {
	auto & logger = Logger::getInstance();
	logger.enable(Logger::Level::DEBUG);
	std::string tag = "Main";
	logger.disable(Logger::Level::INFO);
	LOG_INFO("Main", "System Started");

    // Create ComputerSystem
    ComputerSystem computerSystem;
    computerSystem.start();

    // Get the channel IDs for Radar and OperatorConsole
    int computerSystemRadarCoid = ConnectAttach(ND_LOCAL_NODE, 0, computerSystem.getRadarChannelId(), _NTO_SIDE_CHANNEL, 0);
    if (computerSystemRadarCoid == -1) {
        LOG_ERROR("Main", "Failed to connect to ComputerSystem Radar channel");
        return -1;
    }

    int computerSystemDataDisplayCoid = ConnectAttach(ND_LOCAL_NODE, 0, computerSystem.getDataDisplayChannelId(), _NTO_SIDE_CHANNEL, 0);
    if (computerSystemDataDisplayCoid == -1) {
         LOG_ERROR("Main", "Failed to connect to ComputerSystem DataDisplay channel");
        return -1;
    }
    int computerSystemOperatorCoid = ConnectAttach(ND_LOCAL_NODE, 0, computerSystem.getOperatorChannelId(), _NTO_SIDE_CHANNEL, 0);
    if (computerSystemOperatorCoid == -1) {
      LOG_ERROR("Main", "Failed to connect to ComputerSystem Operator channel");
        return -1;
    }

    Console console(computerSystemOperatorCoid);
    console.start();






    // Create Radar and connect to ComputerSystem
    Radar radar(computerSystemRadarCoid);
    read_planes(radar);
    radar.start();

    // Create DataDisplay and connect to ComputerSystem
    DataDisplay dataDisplay(computerSystemDataDisplayCoid);
    dataDisplay.start();

    while(true){};
    ConnectDetach(computerSystemRadarCoid);
    ConnectDetach(computerSystemDataDisplayCoid);
    ConnectDetach(computerSystemOperatorCoid);

//    // Stop all systems
    radar.stop();
    dataDisplay.stop();
    computerSystem.stop();

    LOG_INFO("Main", "PROGRAM DONE!");

    return 0;
}

//read and create planes from planes.txt file
//IDs are hardcoded for now, they are in the .txt file.
void read_planes(Radar& radar) {
	std::string filePath = "./planes.txt";
//	std::string filePath = "./planes_10.txt";
	std::ifstream plane_file;
	plane_file.open(filePath);
	Vector Position;
	Vector Velocity;
	std::string ID;


	if(!plane_file.is_open()){
      LOG_ERROR("Main", "Could not open file " + filePath);
	}

	std::string line;
	    while (std::getline(plane_file, line, ';'))
	    {

	        if (line.empty() || line == "\n") {
	            continue;
	        }

	        // Remove any trailing newline characters
	        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
	    	// Split each line by semicolons
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
	    	    radar.add_plane(ID,Position, Velocity);
	   }

	    plane_file.close();
}


