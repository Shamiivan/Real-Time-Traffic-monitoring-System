// DataDisplay.cpp
#include "DataDisplay.h"
#include "messages.h"
#include <sys/neutrino.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <algorithm> // For std::find

DataDisplay::DataDisplay(int computerSystemCoid)
    : computerSystemCoid_(computerSystemCoid), running_(false) {}

DataDisplay::~DataDisplay() {
    stop();
}

void DataDisplay::start() {
    running_ = true;
    int ret = pthread_create(&thread_, nullptr, DataDisplay::threadFunc, this);
    if (ret != 0) {
        perror("DataDisplay: Failed to create thread");
        exit(EXIT_FAILURE);
    } else {
        std::cout << "DataDisplay thread started.\n";
    }
}

void DataDisplay::stop() {
    if (running_) {
        running_ = false;
        pthread_join(thread_, nullptr);
    }
}

void* DataDisplay::threadFunc(void* arg) {
    DataDisplay* self = static_cast<DataDisplay*>(arg);
    self->run();
    return nullptr;
}

void DataDisplay::run() {
    while (running_) {
        requestDataFromComputerSystem();
//        updateDisplay();
        sleep(5); // Update every 5 seconds
    }
}

void DataDisplay::requestDataFromComputerSystem() {
    DataDisplayRequestMsg requestMsg;
    requestMsg.requestAugmentedData = !augmentedAircraftIds_.empty();

    // Prepare to receive the reply
    ComputerToDataDisplayMsg replyMsg;

    std::cout << "DataDisplay: Requesting data from ComputerSystem.\n";

    int status = MsgSend(computerSystemCoid_, &requestMsg, sizeof(requestMsg), &replyMsg, sizeof(replyMsg));
    if (status == -1) {
        perror("DataDisplay: Failed to request data from ComputerSystem");
    } else {
        // status will be EOK (0)
        int numAircraft = replyMsg.numAircraft;
        std::lock_guard<std::mutex> lock(mtx);
        aircraftStates_.assign(replyMsg.aircraftData, replyMsg.aircraftData + numAircraft);
        std::cout << "DataDisplay: Received " << numAircraft << " aircraft from ComputerSystem.\n";
    }
}



void DataDisplay::updateDisplay() {
    std::lock_guard<std::mutex> lock(mtx);

    // Check if we have any aircraft data
    if (aircraftStates_.empty()) {
    	std::cout << "DataDisplay: No aircraft data to display.\n";
        return;
    }

    // Define grid dimensions
    const int GRID_WIDTH = 50;
    const int GRID_HEIGHT = 50;
    const double MAX_COORD = 20.0; // Maximum coordinate value for scaling

    // Initialize grid with empty spaces
    std::vector<std::string> grid(GRID_HEIGHT, std::string(GRID_WIDTH, ' '));

    // Map to store plane symbols and their positions
    std::vector<std::pair<int, int>> planePositions; // (x, y) positions

    // Assign symbols to planes
    char planeSymbols[50]; // Supports up to 50 planes
    for (size_t i = 0; i < aircraftStates_.size(); ++i) {
        planeSymbols[i] = (i < 26) ? ('A' + i) : ('a' + (i - 26)); // Use letters A-Z, a-z
    }

    // Place planes on the grid
    for (size_t i = 0; i < aircraftStates_.size(); ++i) {
        int gridX, gridY;
        scalePositionsToGrid(gridX, gridY, aircraftStates_[i].position.x, aircraftStates_[i].position.y, GRID_WIDTH, GRID_HEIGHT, MAX_COORD);

        // Ensure grid coordinates are within bounds
        gridX = std::min(std::max(gridX, 0), GRID_WIDTH - 1);
        gridY = std::min(std::max(gridY, 0), GRID_HEIGHT - 1);

        // Place the plane symbol on the grid
        grid[gridY][gridX] = planeSymbols[i];

        // Store the position for altitude display
        planePositions.push_back({gridX, gridY});
    }

    // Display the grid
    std::cout << "\nDataDisplay: Current Aircraft Positions:\n";
    for (int y = GRID_HEIGHT - 1; y >= 0; --y) { // y-axis goes from bottom to top
        std::cout << std::setw(2) << y << " | ";
        for (int x = 0; x < GRID_WIDTH; ++x) {
            std::cout << grid[y][x];
        }
        std::cout << '\n';
    }
    std::cout << "    ";
    for (int x = 0; x < GRID_WIDTH + 2; ++x) std::cout << '-';
    std::cout << "\n     ";
    for (int x = 0; x < GRID_WIDTH; ++x) {
        if (x % 2 == 0)
            std::cout << x % 10;
        else
            std::cout << ' ';
    }
    std::cout << "\n";

    // Display altitudes for each plane
    for (size_t i = 0; i < aircraftStates_.size(); ++i) {
        std::cout << "Plane " << planeSymbols[i] << " (" << aircraftStates_[i].id << "): ";
        std::cout << "Altitude: " << aircraftStates_[i].position.z << " units\n";
    }
}

void DataDisplay::scalePositionsToGrid(int& gridX, int& gridY, double planeX, double planeY, int gridWidth, int gridHeight, double maxCoord) {
    // Scale plane positions to grid coordinates
    gridX = static_cast<int>((planeX / maxCoord) * (gridWidth - 1));
    gridY = static_cast<int>((planeY / maxCoord) * (gridHeight - 1));
}

void DataDisplay::addAugmentedAircraft(const std::string& aircraftId) {
    std::lock_guard<std::mutex> lock(mtx);
    augmentedAircraftIds_.push_back(aircraftId);
}

void DataDisplay::removeAugmentedAircraft(const std::string& aircraftId) {
    std::lock_guard<std::mutex> lock(mtx);
    augmentedAircraftIds_.erase(std::remove(augmentedAircraftIds_.begin(), augmentedAircraftIds_.end(), aircraftId), augmentedAircraftIds_.end());
}
