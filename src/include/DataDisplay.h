// DataDisplay.h
#ifndef DATADISPLAY_H
#define DATADISPLAY_H

#include <pthread.h>
#include <vector>
#include <string>
#include <mutex>
#include "messages.h" // Include messages.h instead

class DataDisplay {
public:
    DataDisplay(int computerSystemCoid);
    ~DataDisplay();

    void start();
    void stop();

    void addAugmentedAircraft(const std::string& aircraftId);
    void removeAugmentedAircraft(const std::string& aircraftId);

private:
    static void* threadFunc(void* arg);
    void run();

    int computerSystemCoid_;
    pthread_t thread_;
    bool running_;
    std::mutex mtx;

    std::vector<PlaneState> aircraftStates_;
    std::vector<std::string> augmentedAircraftIds_;

    // Helper methods
    void updateDisplay();
    void requestDataFromComputerSystem();
};

#endif // DATADISPLAY_H
