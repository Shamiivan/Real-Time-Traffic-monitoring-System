// DataDisplay.cpp
#include "DataDisplay.h"
#include "messages.h"
#include <sys/neutrino.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <algorithm> // For std::find
#include "Logger.h"
#include "radar.h"
DataDisplay::DataDisplay(int computerSystemCoid)
    : computerSystemCoid_(computerSystemCoid), running_(false) {}

DataDisplay::~DataDisplay() {
    stop();
}

void DataDisplay::start() {
    running_ = true;
    int ret = pthread_create(&thread_, nullptr, DataDisplay::threadFunc, this);
    if (ret != 0) {
        LOG_ERROR("DataDisplay", "Failed to create thread");
        exit(EXIT_FAILURE);
    }
    LOG_INFO("DataDisplay", "DataDisplay thread started.");
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
        updateDisplay();
        sleep(5); // Update every 5 seconds
    }
}

void DataDisplay::requestDataFromComputerSystem() {
    DataDisplayRequestMsg requestMsg;
    requestMsg.requestAugmentedData = !augmentedAircraftIds_.empty();

    // Prepare to receive the reply
    ComputerToDataDisplayMsg replyMsg;
    LOG_INFO("DataDisplay", "Requesting data from ComputerSystem");

    int status = MsgSend(computerSystemCoid_, &requestMsg, sizeof(requestMsg), &replyMsg, sizeof(replyMsg));
    if (status == -1) {
      LOG_ERROR("DataDisplay", "Failed to request data from ComputerSystem");
    } else {
        // status will be EOK (0)
        int numAircraft = replyMsg.numAircraft;
        std::lock_guard<std::mutex> lock(mtx);
        aircraftStates_.assign(replyMsg.aircraftData, replyMsg.aircraftData + numAircraft);
        LOG_INFO("DataDisplay", "Received " + std::to_string(numAircraft) + " aircraft from ComputerSystem.");
    }
}



void DataDisplay::updateDisplay() {
    std::lock_guard<std::mutex> lock(mtx);
    LOG_INFO("Plane", "Update display");

    if (aircraftStates_.empty()) {
        LOG_INFO("DataDisplay", "No aircraft data to display");
        return;
    }

    // Constants for grid size and bounds
    const int GRID_WIDTH = 50;
    const int GRID_HEIGHT = 25;
    const double MAX_X = 100000;
    const double MAX_Y = 100000;

    // Create grid with empty spaces
    std::vector<std::vector<char>> grid(GRID_HEIGHT, std::vector<char>(GRID_WIDTH, '.'));

    LOG_INFO("DataDisplay", "Displaying aircraft data on grid");

    // Plot each aircraft on grid
    for (const auto& aircraft : aircraftStates_) {
        // Scale coordinates to grid size
        // Flip Y coordinate since grid origin is top-left but aircraft coordinates have origin at bottom-left
        int gridX = static_cast<int>((aircraft.position.x / MAX_X) * (GRID_WIDTH - 1));
        int gridY = GRID_HEIGHT - 1 - static_cast<int>((aircraft.position.y / MAX_Y) * (GRID_HEIGHT - 1));

        // Ensure coordinates are within bounds
        gridX = std::min(std::max(gridX, 0), GRID_WIDTH - 1);
        gridY = std::min(std::max(gridY, 0), GRID_HEIGHT - 1);

        // Select symbol based on altitude
        char symbol;
        double altitude = aircraft.position.z;
        if (altitude < 5000) {
            symbol = 'v';  // Low altitude
        } else if (altitude < 10000) {
            symbol = '#';  // Medium altitude
        } else {
            symbol = '^';  // High altitude
        }

        // Place aircraft on grid
        grid[gridY][gridX] = symbol;

        // Display aircraft info
        LOG_INFO("DataDisplay", "Aircraft " + std::string(aircraft.id) + " Position: (" +
                 std::to_string(aircraft.position.x) + ", " +
                 std::to_string(aircraft.position.y) + ", " +
                 std::to_string(aircraft.position.z) + ")");
    }

    // Build grid visualization as a string
       std::stringstream gridDisplay;
       gridDisplay << "\nCurrent Airspace Status:\n";
       gridDisplay << std::string(GRID_WIDTH + 2, '+') << '\n';

       for (int y = 0; y < GRID_HEIGHT; y++) {
           gridDisplay << '+';
           for (int x = 0; x < GRID_WIDTH; x++) {
               gridDisplay << grid[y][x];
           }
           gridDisplay << "+\n";
       }

       gridDisplay << std::string(GRID_WIDTH + 2, '+') << '\n';
       gridDisplay << "Legend: v=Low altitude  #=Medium altitude  ^=High altitude\n";
       LOG_WARNING("DataDisplay", gridDisplay.str());
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
