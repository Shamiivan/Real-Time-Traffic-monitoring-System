#include <iostream>
#include "plane.h"
#include <vector>
#include <chrono>
#include <thread>

int main() {
	Plane plane1("1", {0, 0, 0}, {1, 1, 1});
    Plane plane2("2", {0, 0, 0}, {1, 1, 1});
    // sleep for 5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));
    printf("Plane 1: %f %f %f\n", plane1.get_pos().x, plane1.get_pos().y, plane1.get_pos().z);
    printf("Plane 2: %f %f %f\n", plane2.get_pos().x, plane2.get_pos().y, plane2.get_pos().z);
	return 0;
}
