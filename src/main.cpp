#include <iostream>
#include "plane.h"
#include <vector>
#include <chrono>
#include <pthread.h>
#include <sys/neutrino.h>
#include <thread>
#include <memory>
#include <fstream>
#include <string>
#include <sstream>


void read_planes(std::vector<std::unique_ptr<Plane>>& planes);

int main() {
    std::vector<std::unique_ptr<Plane>> planes;
    planes.emplace_back(std::make_unique<Plane>("1", Vector{0, 0, 0}, Vector{1, 1, 1}));
    planes.emplace_back(std::make_unique<Plane>("2", Vector{0, 0, 0}, Vector{30, 20, 10}));
	planes.emplace_back(std::make_unique<Plane>("3", Vector{0, 0, 0}, Vector{5, 5, 5}));

	read_planes(planes);

    for (auto& plane : planes) {
		plane->start();
	}
    std::this_thread::sleep_for(std::chrono::seconds(5));
    // sleep for 5 seconds
    for (auto& plane : planes) {
      printf("Plane %s: (%f, %f, %f)\n", plane->get_id().c_str(), plane->get_pos().x, plane->get_pos().y, plane->get_pos().z);
  }
	for (auto& plane : planes) {
		plane->stop();
	}


	return 0;
}

//read and create planes from planes.txt file
//IDs are hardcoded for now, they are in the .txt file.
void read_planes(std::vector<std::unique_ptr<Plane>>& planes) {
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
	    	            ID = row[0];
	    	            Position = {std::stof(row[1]), std::stof(row[2]), std::stof(row[3])};
	    	            Velocity = {std::stof(row[4]), std::stof(row[5]), std::stof(row[6])};
	    	            planes.emplace_back(std::make_unique<Plane>(ID, Position, Velocity));
	    	        }
	    }

	    plane_file.close();
}
