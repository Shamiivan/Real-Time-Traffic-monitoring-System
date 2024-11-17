// DataDisplay.cpp
#include "DataDisplay.h"
#include "messages.h"
#include <sys/neutrino.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

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
        updateDisplay();
        sleep(5); // Update every 5 seconds
    }
}

void DataDisplay::requestDataFromComputerSystem() {
    // Send a message to the ComputerSystem to get the latest aircraft states
    DataDisplayRequestMsg requestMsg;
    requestMsg.requestAugmentedData = !augmentedAircraftIds_.empty();

    // Buffer to receive aircraft data
    PlaneState aircraftBuffer[50]; // Maximum of 50 aircraft
    memset(aircraftBuffer, 0, sizeof(aircraftBuffer)); // Initialize buffer

    int status = MsgSend(computerSystemCoid_, &requestMsg, sizeof(requestMsg), aircraftBuffer, sizeof(aircraftBuffer));
    if (status == -1) {
        perror("DataDisplay: Failed to request data from ComputerSystem");
    } else {
        int numBytesReceived = status; // MsgSend returns the number of bytes received in the reply
        int numAircraft = numBytesReceived / sizeof(PlaneState);
        std::lock_guard<std::mutex> lock(mtx);
        aircraftStates_.assign(aircraftBuffer, aircraftBuffer + numAircraft);
    }
}




void DataDisplay::updateDisplay() {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "DataDisplay: Current Aircraft Positions:\n";
    for (const auto& state : aircraftStates_) {
        std::cout << "Aircraft " << state.id << ": Position (" << state.position.x << ", " << state.position.y << ", " << state.position.z << ")";
        // Check if augmented data is requested for this aircraft
        if (std::find(augmentedAircraftIds_.begin(), augmentedAircraftIds_.end(), state.id) != augmentedAircraftIds_.end()) {
            std::cout << ", Speed (" << state.velocity.x << ", " << state.velocity.y << ", " << state.velocity.z << ")";
        }
        std::cout << std::endl;
    }
}

void DataDisplay::addAugmentedAircraft(const std::string& aircraftId) {
    std::lock_guard<std::mutex> lock(mtx);
    augmentedAircraftIds_.push_back(aircraftId);
}

void DataDisplay::removeAugmentedAircraft(const std::string& aircraftId) {
    std::lock_guard<std::mutex> lock(mtx);
    augmentedAircraftIds_.erase(std::remove(augmentedAircraftIds_.begin(), augmentedAircraftIds_.end(), aircraftId), augmentedAircraftIds_.end());
}
