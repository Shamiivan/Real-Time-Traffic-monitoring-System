// ComputerSystem.h
#ifndef COMPUTERSYSTEM_H
#define COMPUTERSYSTEM_H

#include <vector>
#include <mutex>
#include <string>
#include <pthread.h>
#include "messages.h"
#include "vector.h"
#include <sys/neutrino.h>


// Define pulse codes
#define PULSE_CODE_EXIT (_PULSE_CODE_MINAVAIL + 1)

class ComputerSystem {
public:
    ComputerSystem();
    ~ComputerSystem();

    void start();
    void stop();

    // Get channel IDs for IPC
    int getRadarChannelId() const;
    int getOperatorChannelId() const;
    int getDataDisplayChannelId() const;

private:
    static void* threadFunc(void* arg);
    static void* radarThreadFunc(void* arg);
    static void* operatorThreadFunc(void* arg);
    static void* dataDisplayThreadFunc(void* arg);

    void run();
    void radarLoop();
    void operatorLoop();
    void dataDisplayLoop();

    //int getPlaneChannelIdById(const std::string& planeId);
    void sendCourseCorrection(const std::string& planeId, const Vector& velocity, int coid);

    // Methods for separation checks and alerts
    void checkForViolations();
    void emitAlert(const std::string& message);

    pthread_t thread_;          // Main thread for separation checks
    pthread_t radar_thread_;    // Thread for handling radar messages
    pthread_t operator_thread_; // Thread for handling operator messages
    pthread_t dataDisplay_thread_; // Thread for handling DataDisplay requests
    bool running_;
    mutable std::mutex mtx;

    // IPC channels
    int radar_chid_;
    int operator_chid_;
    int dataDisplay_chid_;

    // Data storage
    std::vector<PlaneState> aircraftStates_;
    int lookaheadTime_; // 'n' parameter

    // Synchronization
    pthread_mutex_t data_mutex_;
};

#endif // COMPUTERSYSTEM_H

