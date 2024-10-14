#include <iostream>
#include "plane.h"
#include <vector>

int main() {
	std::vector<Plane> planes;
	planes.push_back(Plane("1", Vector{1, 2, 3}, Vector{4, 5, 6}));
    planes.push_back(Plane("2", Vector{7, 8, 9}, Vector{10, 11, 12}));
    planes.push_back(Plane("3", Vector{13, 14, 15}, Vector{16, 17, 18}));
    planes.push_back(Plane("4", Vector{19, 20, 21}, Vector{22, 23, 24}));

    for (auto plane : planes) {
        std::cout << "Plane " << plane.get_id() << " is at position " <<
          plane.get_pos().x << "\t" << plane.get_pos().y << "\t" << plane.get_pos().z << "\n";
    }

	return 0;
}
