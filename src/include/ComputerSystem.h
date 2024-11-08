// computersystem.h
#ifndef COMPUTERSYSTEM_H
#define COMPUTERSYSTEM_H

#include <vector>
#include <mutex>
#include <string>
#include <pthread.h>
#include "messages.h"
#include "vector.h"

class ComputerSystem {
public:
    ComputerSystem();
    ~ComputerSystem();

    void start();
    void stop();

    // Get channel IDs for IPC
    int getRadarChannelId() const;
    int getOperatorChannelId() const;

private:
    static void* threadFunc(void* arg);
    void run();

    // Methods for separation checks and alerts
    void checkForViolations();
    void emitAlert(const std::string& message);

    pthread_t thread_;
    bool running_;
    mutable std::mutex mtx;

    // IPC channels
    int radar_chid_;
    int operator_chid_;

    // Data storage
    std::vector<PlaneState> aircraftStates_;
    int lookaheadTime_; // 'n' parameter

    // Synchronization
    pthread_mutex_t data_mutex_;
};

#endif // COMPUTERSYSTEM_H
