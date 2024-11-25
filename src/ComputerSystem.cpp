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
#include "Logger.h"

ComputerSystem::ComputerSystem() : running_(false), lookaheadTime_(3) { // Default 'n' is 180 seconds
    pthread_mutex_init(&data_mutex_, nullptr);

    // Create channels for receiving messages
    radar_chid_ = ChannelCreate(0);
    if (radar_chid_ == -1) {
      LOG_ERROR("ComputerSystem", "Failed to create radar channel");
        exit(EXIT_FAILURE);
    }

    operator_chid_ = ChannelCreate(0);
    if (operator_chid_ == -1) {
      LOG_ERROR("ComputerSystem", "Failed to create operator channel");
        exit(EXIT_FAILURE);
    }

    dataDisplay_chid_ = ChannelCreate(0);
    if (dataDisplay_chid_ == -1) {
      LOG_ERROR("ComputerSystem", "Failed to create DataDisplay channel");
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
      LOG_ERROR("ComputerSystem", "Failed to create main thread");
      exit(EXIT_FAILURE);
    }
    LOG_INFO("ComputerSystem", "Main thread started");

    // Start radar thread
    ret = pthread_create(&radar_thread_, nullptr, ComputerSystem::radarThreadFunc, this);
    if (ret != 0) {
        LOG_ERROR("ComputerSystem", "Failed to create radar thread");
        exit(EXIT_FAILURE);
    }
    LOG_INFO("ComputerSystem", "Radar thread started");

    // Start operator thread
    ret = pthread_create(&operator_thread_, nullptr, ComputerSystem::operatorThreadFunc, this);
    if (ret != 0) {
        LOG_ERROR("ComputerSystem", "Failed to create operator thread");
        exit(EXIT_FAILURE);
    }
    LOG_INFO("ComputerSystem", "Operator thread started");


    // Start DataDisplay thread
    ret = pthread_create(&dataDisplay_thread_, nullptr, ComputerSystem::dataDisplayThreadFunc, this);
    if (ret != 0) {
        LOG_ERROR("ComputerSystem", "Failed to create DataDisplay thread");
        exit(EXIT_FAILURE);
    } else {
        LOG_INFO("ComputerSystem", "DataDisplay thread started");
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

        //destroy channels
        ChannelDestroy(radar_chid_);
        ChannelDestroy(operator_chid_);
        ChannelDestroy(dataDisplay_chid_);

        pthread_join(thread_, nullptr);
        pthread_join(radar_thread_, nullptr);
        pthread_join(operator_thread_, nullptr);
        pthread_join(dataDisplay_thread_, nullptr);
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
                LOG_ERROR("ComputerSystem", "MsgReceive from Radar failed");
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
                LOG_ERROR("ComputerSystem", "MsgReceive from OperatorConsole failed");
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
            OperatorCommandMsg* msg = (OperatorCommandMsg*)&msg_buffer;
            // Process operator command
            switch(msg->type) {
                case ConsoleCommand::LIST_PLANES:
//                    sendPlaneDataToConsole(opCmdMsg->planeId);
                    LOG_WARNING("ComputerSystem", "Operator command to get plane data not implemented");
                    break;
                case ConsoleCommand::UPDATE_PLANE_VELOCITY:
                    // Update plane velocity
                    pthread_mutex_lock(&data_mutex_);
                      for (auto& plane : aircraftStates_) {
                          if (strcmp(plane.id,msg->planeId) == 0) {
                              LOG_INFO("ComputerSystem", std::string("Found plane ") + msg->planeId);
                              sendCourseCorrection(plane.id, msg->velocity, plane.coid_comp);
                              break;
                          }
                      }
                      pthread_mutex_unlock(&data_mutex_);

                    LOG_INFO("ComputerSystem", std::string("Updated velocity for plane ") + msg->planeId);
                    break;
                case ConsoleCommand::UPDATE_PLANE_POSITION:
                    // Update plane position
                    LOG_WARNING("ComputerSystem", "Operator command to update plane position not implemented");
                    break;
                default:
                    LOG_WARNING("ComputerSystem", "Unknown operator command");
                    break;
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
                LOG_WARNING("ComputerSystem", message);
                Vector velocity = aircraftStatesCopy[i].velocity;
                velocity.z += 1000;
                std::lock_guard<std::mutex> lock(mtx);
                sendCourseCorrection(aircraftStatesCopy[i].id, velocity,aircraftStatesCopy[i].coid_comp);
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
                LOG_ERROR("ComputerSystem", "MsgReceive from DataDisplay failed");
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
            for (int i = 0; i < replyMsg.numAircraft; ++i) {
                replyMsg.aircraftData[i] = aircraftStatesCopy[i];
            }

            // Send the data to DataDisplay
            size_t replySize = sizeof(int) + replyMsg.numAircraft * sizeof(PlaneState);
            int status = MsgReply(rcvid, EOK, &replyMsg, replySize);
            if (status == -1) {
                LOG_ERROR("ComputerSystem", "Failed to send data to DataDisplay");
            }
        }
    }
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

	int con = ConnectAttach(ND_LOCAL_NODE, 0, coid, _NTO_SIDE_CHANNEL, 0);
	bool status = MsgSend(con, &msg, sizeof(msg), nullptr, 0);
	if (status == -1){
        LOG_ERROR("ComputerSystem", "Failed to send Course Correction to Radar");
	}
    LOG_WARNING("ComputerSystem", "Sent Course Correction to plane" + planeId);
}



void ComputerSystem::sendPlaneDataToConsole(const std::string& planeId){
	pthread_mutex_lock(&data_mutex_);
	std::vector<PlaneState> aircraftStatesCopy = aircraftStates_;
	pthread_mutex_unlock(&data_mutex_);

	for(size_t i = 0; i < aircraftStatesCopy.size(); ++i){
		if (aircraftStatesCopy[i].id == planeId){
			//if Id at index equals ID desired by Operator
			//send back to operator
			PlaneState state = aircraftStatesCopy[i];
			bool status = MsgSend(operator_chid_, &state, sizeof(state), nullptr, 0);
			if (status == -1){
			        LOG_ERROR("ComputerSystem", "Failed to send plane " + planeId + " data to console");
				}
			    LOG_WARNING("ComputerSystem", "Sent plane " + planeId + " data to console");
		}
	}
}
