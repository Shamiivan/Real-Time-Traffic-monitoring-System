// ComputerSystem.cpp
#include "ComputerSystem.h"
#include "messages.h"
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sched.h> // Include this header for scheduling functions
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cmath>
#include <errno.h>

ComputerSystem::ComputerSystem() : running_(false), lookaheadTime_(3) { // Default 'n' is 180 seconds
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

    dataDisplay_chid_ = ChannelCreate(0);
    if (dataDisplay_chid_ == -1) {
    	perror("ComputerSystem: Failed to create DataDisplay channel");
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
        perror("ComputerSystem: Failed to create main thread");
        exit(EXIT_FAILURE);
    } else {
        std::cout << "ComputerSystem main thread started.\n";
    }

    // Start radar thread
    ret = pthread_create(&radar_thread_, nullptr, ComputerSystem::radarThreadFunc, this);
    if (ret != 0) {
        perror("ComputerSystem: Failed to create radar thread");
        exit(EXIT_FAILURE);
    } else {
        std::cout << "ComputerSystem radar thread started.\n";
    }

    // Start operator thread
    ret = pthread_create(&operator_thread_, nullptr, ComputerSystem::operatorThreadFunc, this);
    if (ret != 0) {
        perror("ComputerSystem: Failed to create operator thread");
        exit(EXIT_FAILURE);
    } else {
        std::cout << "ComputerSystem operator thread started.\n";
    }

    // Start DataDisplay thread
    ret = pthread_create(&dataDisplay_thread_, nullptr, ComputerSystem::dataDisplayThreadFunc, this);
    if (ret != 0) {
    	perror("ComputerSystem: Failed to create DataDisplay thread");
        exit(EXIT_FAILURE);
    } else {
    	std::cout << "ComputerSystem DataDisplay thread started.\n";
    }
}

void ComputerSystem::stop() {
    if (running_) {
        running_ = false;

        // Get current thread's priority
        struct sched_param param;
        int policy;
        pthread_getschedparam(pthread_self(), &policy, &param);
        int priority = param.sched_priority;

        // Unblock the threads by sending pulses to the channels
        int radar_coid = ConnectAttach(0, 0, radar_chid_, _NTO_SIDE_CHANNEL, 0);
        if (radar_coid != -1) {
            MsgSendPulse(radar_coid, priority, PULSE_CODE_EXIT, 0);
            ConnectDetach(radar_coid);
        }

        int operator_coid = ConnectAttach(0, 0, operator_chid_, _NTO_SIDE_CHANNEL, 0);
        if (operator_coid != -1) {
            MsgSendPulse(operator_coid, priority, PULSE_CODE_EXIT, 0);
            ConnectDetach(operator_coid);
        }

        int dataDisplay_coid = ConnectAttach(0, 0, dataDisplay_chid_, _NTO_SIDE_CHANNEL, 0);
        if (dataDisplay_coid != -1) {
        	MsgSendPulse(dataDisplay_coid, priority, PULSE_CODE_EXIT, 0);
            ConnectDetach(dataDisplay_coid);
        }

        pthread_join(thread_, nullptr);
        pthread_join(radar_thread_, nullptr);
        pthread_join(operator_thread_, nullptr);
        pthread_join(dataDisplay_thread_, nullptr);

        ChannelDestroy(radar_chid_);
        ChannelDestroy(operator_chid_);
        ChannelDestroy(dataDisplay_chid_);
    }
}

void* ComputerSystem::threadFunc(void* arg) {
    ComputerSystem* self = static_cast<ComputerSystem*>(arg);
    self->run();
    return nullptr;
}

void* ComputerSystem::radarThreadFunc(void* arg) {
    ComputerSystem* self = static_cast<ComputerSystem*>(arg);
    self->radarLoop();
    return nullptr;
}

void* ComputerSystem::operatorThreadFunc(void* arg) {
    ComputerSystem* self = static_cast<ComputerSystem*>(arg);
    self->operatorLoop();
    return nullptr;
}

void* ComputerSystem::dataDisplayThreadFunc(void* arg) {
    ComputerSystem* self = static_cast<ComputerSystem*>(arg);
    self->dataDisplayLoop();
    return nullptr;
}

void ComputerSystem::run() {
    while (running_) {
        // Perform separation checks
        checkForViolations();

        // Sleep for a short duration before next cycle
        usleep(100000); // 100 ms
    }
}

void ComputerSystem::radarLoop() {
    while (running_) {
        // Prepare to receive the maximum possible message size
        char msg_buffer[sizeof(RadarToComputerMsg)];
        int rcvid = MsgReceive(radar_chid_, &msg_buffer, sizeof(msg_buffer), NULL);
        if (rcvid == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("ComputerSystem: MsgReceive from Radar failed");
                break;
            }
        }
        if (rcvid == 0) {
            struct _pulse* pulse = (struct _pulse*)&msg_buffer;
            if (pulse->code == PULSE_CODE_EXIT) {
                break;
            }
        } else if (rcvid > 0) {
            RadarToComputerMsg* radarMsg = (RadarToComputerMsg*)&msg_buffer;
            pthread_mutex_lock(&data_mutex_);
            aircraftStates_.assign(radarMsg->aircraftData, radarMsg->aircraftData + radarMsg->numAircraft);
            pthread_mutex_unlock(&data_mutex_);
            MsgReply(rcvid, EOK, nullptr, 0);

            // Add logging
            std::cout << "ComputerSystem: Received " << radarMsg->numAircraft << " aircraft from Radar.\n";
            for (int i = 0; i < radarMsg->numAircraft; ++i) {
                PlaneState& state = radarMsg->aircraftData[i];
                std::cout << "ComputerSystem: Aircraft " << state.id << " Position (" << state.position.x << ", " << state.position.y << ", " << state.position.z << ")\n";
            }
        }
    }
}


void ComputerSystem::operatorLoop() {
    while (running_) {
        // Receive commands from OperatorConsole
        char msg_buffer[sizeof(OperatorCommandMsg)];
        int rcvid = MsgReceive(operator_chid_, &msg_buffer, sizeof(msg_buffer), NULL);
        if (rcvid == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("ComputerSystem: MsgReceive from OperatorConsole failed");
                break;
            }
        }
        if (rcvid == 0) {
            // Pulse received
            struct _pulse* pulse = (struct _pulse*)&msg_buffer;
            if (pulse->code == PULSE_CODE_EXIT) {
                break;
            }
        } else if (rcvid > 0) {
            // Message received
            OperatorCommandMsg* opCmdMsg = (OperatorCommandMsg*)&msg_buffer;
            // Process operator command
            if (opCmdMsg->command.type == OperatorCommand::SET_LOOKAHEAD_TIME) {
                pthread_mutex_lock(&data_mutex_);
                lookaheadTime_ = opCmdMsg->command.lookaheadTime;
                pthread_mutex_unlock(&data_mutex_);
                std::cout << "ComputerSystem: Lookahead time set to " << lookaheadTime_ << " seconds.\n";
            }
            MsgReply(rcvid, EOK, nullptr, 0);
        }
    }
}

void ComputerSystem::checkForViolations() {
    pthread_mutex_lock(&data_mutex_);
    // Copy the aircraft states to a local variable to minimize lock time
    std::vector<PlaneState> aircraftStatesCopy = aircraftStates_;
    int lookaheadTime = lookaheadTime_;
    pthread_mutex_unlock(&data_mutex_);

    // Predict positions at current_time + n seconds and check for violations
    for (size_t i = 0; i < aircraftStatesCopy.size(); ++i) {
        for (size_t j = i + 1; j < aircraftStatesCopy.size(); ++j) {
            // Predict positions
            Vector pos1 = aircraftStatesCopy[i].position;
            Vector speed1 = aircraftStatesCopy[i].velocity;
            Vector futurePos1 = {
                pos1.x + speed1.x * lookaheadTime,
                pos1.y + speed1.y * lookaheadTime,
                pos1.z + speed1.z * lookaheadTime
            };

            Vector pos2 = aircraftStatesCopy[j].position;
            Vector speed2 = aircraftStatesCopy[j].velocity;
            Vector futurePos2 = {
                pos2.x + speed2.x * lookaheadTime,
                pos2.y + speed2.y * lookaheadTime,
                pos2.z + speed2.z * lookaheadTime
            };

            // Check separation
            double horizontalDist = sqrt(pow(futurePos1.x - futurePos2.x, 2) +
                                         pow(futurePos1.y - futurePos2.y, 2));
            double verticalDist = fabs(futurePos1.z - futurePos2.z);

            if (horizontalDist < 3.0 && verticalDist < 1.0) {
                // Violation detected
                std::string message = "Potential violation between ";
                message += aircraftStatesCopy[i].id;
                message += " and ";
                message += aircraftStatesCopy[j].id;
                emitAlert(message);

                Vector velocity = aircraftStatesCopy[i].velocity;
                velocity.z += 1;
                std::lock_guard<std::mutex> lock(mtx);
                sendCourseCorrection(aircraftStatesCopy[i].id, velocity,aircraftStatesCopy[i].coid);
            }
        }
    }
}

void ComputerSystem::dataDisplayLoop() {
    while (running_) {
        DataDisplayRequestMsg requestMsg;
        int rcvid = MsgReceive(dataDisplay_chid_, &requestMsg, sizeof(requestMsg), NULL);
        if (rcvid == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("ComputerSystem: MsgReceive from DataDisplay failed");
                break;
            }
        }
        if (rcvid == 0) {
            struct _pulse* pulse = (struct _pulse*)&requestMsg;
            if (pulse->code == PULSE_CODE_EXIT) {
                break;
            }
        } else if (rcvid > 0) {
            // Process data display request
            pthread_mutex_lock(&data_mutex_);
            std::vector<PlaneState> aircraftStatesCopy = aircraftStates_;
            pthread_mutex_unlock(&data_mutex_);

            // Define maximum number of aircraft to send
            const size_t MAX_AIRCRAFT = 50;
            if (aircraftStatesCopy.size() > MAX_AIRCRAFT) {
                aircraftStatesCopy.resize(MAX_AIRCRAFT);
            }

            // Prepare the reply message
            ComputerToDataDisplayMsg replyMsg;
            replyMsg.numAircraft = aircraftStatesCopy.size();
            for (size_t i = 0; i < replyMsg.numAircraft; ++i) {
                replyMsg.aircraftData[i] = aircraftStatesCopy[i];
            }

            // Send the data to DataDisplay
            size_t replySize = sizeof(int) + replyMsg.numAircraft * sizeof(PlaneState);
            int status = MsgReply(rcvid, EOK, &replyMsg, replySize);
            if (status == -1) {
                perror("ComputerSystem: Failed to send data to DataDisplay");
            } else {
                std::cout << "ComputerSystem: Sent " << replyMsg.numAircraft << " aircraft to DataDisplay.\n";
            }
        }
    }
}





void ComputerSystem::emitAlert(const std::string& message) {
    // Emit an alert (e.g., print to console)
    std::cout << "ALERT: " << message << std::endl;
}

int ComputerSystem::getRadarChannelId() const {
    return radar_chid_;
}

int ComputerSystem::getDataDisplayChannelId() const {
    return dataDisplay_chid_;
}

int ComputerSystem::getOperatorChannelId() const {
    return operator_chid_;
}

void ComputerSystem::sendCourseCorrection(const std::string& planeId, const Vector& velocity, int coid){
	courseCorrectionMsg msg;
	msg.id = planeId;
	msg.newVelocity = velocity;

	bool status = MsgSend(coid, &msg, sizeof(msg), nullptr, 0);
	if (status == -1){
		perror("ComputerSystem: Failed to send Course Correction to Radar");
	}else{
		std::cout << "ComputerSystem: Sent Course Correction to plane" << planeId <<"\n";
	}
}



