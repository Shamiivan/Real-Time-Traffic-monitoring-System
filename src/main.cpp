#include <iostream>
#include "plane.h"
#include <vector>
#include <chrono>
#include <pthread.h>
#include <sys/neutrino.h>
#include <thread>
#include <memory>

int main() {
    std::vector<std::unique_ptr<Plane>> planes;
    planes.emplace_back(std::make_unique<Plane>("1", Vector{0, 0, 0}, Vector{1, 1, 1}));
    planes.emplace_back(std::make_unique<Plane>("2", Vector{0, 0, 0}, Vector{30, 20, 10}));
	planes.emplace_back(std::make_unique<Plane>("3", Vector{0, 0, 0}, Vector{5, 5, 5}));


    for (auto& plane : planes) {
		plane->start();
	}
    // sleep for 5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));
    for (auto& plane : planes) {
      if(plane->get_id() == "1"){
        plane->stop();
	  }
      printf("Plane %s: (%f, %f, %f)\n", plane->get_id().c_str(), plane->get_pos().x, plane->get_pos().y, plane->get_pos().z);
  	}


    std::this_thread::sleep_for(std::chrono::seconds(5));

	for (auto& plane : planes) {
      printf("Plane %s: (%f, %f, %f)\n", plane->get_id().c_str(), plane->get_pos().x, plane->get_pos().y, plane->get_pos().z);
  	}

	for (auto& plane : planes) {
		plane->stop();
	}


	return 0;
}
