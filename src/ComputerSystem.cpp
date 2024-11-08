// computersystem.cpp
#include "computersystem.h"
#include "messages.h"
#include <sys/neutrino.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cmath>

ComputerSystem::ComputerSystem() : running_(false), lookaheadTime_(180) { // Default 'n' is 180 seconds
    pthread_mutex_init(&data_mutex_, nullptr);

    // Create channels for receiving messages
    radar_chid_ = ChannelCreate(0);
    if (radar_chid_ == -1) {
        perror("ComputerSystem: Failed to create radar channel");
        exit(EXIT_FAILURE);
    }

    operator_chid_ = ChannelCreate(0);
    if (operator_chid_ == -1) {
        perror("ComputerSystem: Failed to create operator channel");
        exit(EXIT_FAILURE);
    }
}

ComputerSystem::~ComputerSystem() {
    stop();
    pthread_mutex_destroy(&data_mutex_);
}

void ComputerSystem::start() {
    running_ = true;
    int ret = pthread_create(&thread_, nullptr, ComputerSystem::threadFunc, this);
    if (ret != 0) {
        perror("ComputerSystem: Failed to create thread");
        exit(EXIT_FAILURE);
    } else {
        std::cout << "ComputerSystem thread started.\n";
    }
}

void ComputerSystem::stop() {
    if (running_) {
        running_ = false;
        pthread_join(thread_, nullptr);
        ChannelDestroy(radar_chid_);
        ChannelDestroy(operator_chid_);
    }
}

void* ComputerSystem::threadFunc(void* arg) {
    ComputerSystem* self = static_cast<ComputerSystem*>(arg);
    self->run();
    return nullptr;
}

void ComputerSystem::run() {
    while (running_) {
        // Receive data from Radar
        struct _msg_info info;
        RadarToComputerMsg radarMsg;
        int rcvid = MsgReceive(radar_chid_, &radarMsg, sizeof(radarMsg), &info);
        if (rcvid > 0) {
            // Process radar data
            pthread_mutex_lock(&data_mutex_);
            aircraftStates_.clear();
            for (int i = 0; i < radarMsg.numAircraft; ++i) {
                aircraftStates_.push_back(radarMsg.aircraftData[i]);
            }
            pthread_mutex_unlock(&data_mutex_);
            MsgReply(rcvid, EOK, nullptr, 0);
        }

        // Receive commands from OperatorConsole
        OperatorCommandMsg opCmdMsg;
        rcvid = MsgReceive(operator_chid_, &opCmdMsg, sizeof(opCmdMsg), &info);
        if (rcvid > 0) {
            // Process operator command
            if (opCmdMsg.command.type == OperatorCommand::SET_LOOKAHEAD_TIME) {
                pthread_mutex_lock(&data_mutex_);
                lookaheadTime_ = opCmdMsg.command.lookaheadTime;
                pthread_mutex_unlock(&data_mutex_);
                std::cout << "ComputerSystem: Lookahead time set to " << lookaheadTime_ << " seconds.\n";
            }
            MsgReply(rcvid, EOK, nullptr, 0);
        }

        // Perform separation checks
        checkForViolations();

        // Sleep for a short duration before next cycle
        usleep(100000); // 100 ms
    }
}

void ComputerSystem::checkForViolations() {
    pthread_mutex_lock(&data_mutex_);
    // Predict positions at current_time + n seconds and check for violations
    // For simplicity, we'll assume linear motion
    for (size_t i = 0; i < aircraftStates_.size(); ++i) {
        for (size_t j = i + 1; j < aircraftStates_.size(); ++j) {
            // Predict positions
            Vector pos1 = aircraftStates_[i].position;
            Vector speed1 = aircraftStates_[i].velocity;
            Vector futurePos1 = pos1 + speed1 * lookaheadTime_;

            Vector pos2 = aircraftStates_[j].position;
            Vector speed2 = aircraftStates_[j].velocity;
            Vector futurePos2 = pos2 + speed2 * lookaheadTime_;

            // Check separation
            double horizontalDist = sqrt(pow(futurePos1.x - futurePos2.x, 2) +
                                         pow(futurePos1.y - futurePos2.y, 2));
            double verticalDist = fabs(futurePos1.z - futurePos2.z);

            if (horizontalDist < 3000.0 && verticalDist < 1000.0) {
                // Violation detected
                std::string message = "Potential violation between " + aircraftStates_[i].id +
                                      " and " + aircraftStates_[j].id;
                emitAlert(message);
            }
        }
    }
    pthread_mutex_unlock(&data_mutex_);
}

void ComputerSystem::emitAlert(const std::string& message) {
    // Emit an alert (e.g., print to console)
    std::cout << "ALERT: " << message << std::endl;
}

int ComputerSystem::getRadarChannelId() const {
    return radar_chid_;
}

int ComputerSystem::getOperatorChannelId() const {
    return operator_chid_;
}
