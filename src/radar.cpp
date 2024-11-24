// radar.cpp
#include "radar.h"
#include "messages.h"
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include "Logger.h"

Radar::Radar(int computerSystemCoid)
    : running_(false), computerSystemCoid_(computerSystemCoid) {
}

Radar::~Radar() {
    stop();
}

void Radar::start() {
    running_ = true;
    int ret = pthread_create(&thread_, nullptr, Radar::threadFunc, this);
    if (ret != 0) {
        perror("Radar: Failed to create thread");
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Radar thread started.\n";
    }
}

void Radar::stop() {
    if (running_) {
        running_ = false;
        pthread_join(thread_, nullptr);

        // Clean up connections and planes
        std::lock_guard<std::mutex> lock(mtx);
        for (const auto& conn : planeConnections_) {
            ConnectDetach(conn.coid);
            conn.plane->stop();
            delete conn.plane;
        }
        planeConnections_.clear();
        planes_.clear();
    }
}

void* Radar::threadFunc(void* arg) {
    Radar* self = static_cast<Radar*>(arg);
    self->run();
    return nullptr;
}

void Radar::run() {
    while (running_) {
        update_planes();
        // Delay for 1 second before next update
        sleep(1);
    }
}

int Radar::add_plane(std::string id, Vector position, Vector velocity) {
    Plane* plane = new Plane(id, position, velocity);
    plane->start();

    // Connect to the Plane's channel
    int coid = ConnectAttach(ND_LOCAL_NODE, 0, plane->getChannelId(), _NTO_SIDE_CHANNEL, 0);
    int coid_comp = plane->getChannelIdComp();
    if (coid == -1) {
        LOG_ERROR("Radar", "Failed to connect to Plane channel");
        plane->stop();
        delete plane;
        return -1;
    }

    {
        std::lock_guard<std::mutex> lock(planeMtx);
        planes_.push_back(plane);
        PlaneConnection conn = { plane, coid, coid_comp };
        planeConnections_.push_back(conn);
    }

    return 0;
}

int Radar::remove_plane(std::string id) {
    Plane *planeToRemove = nullptr;
    int coidToRemove = -1;
    {
    std::lock_guard<std::mutex> lock(mtx);

    // First find the plane in planeConnections_
    auto connIt = std::find_if(planeConnections_.begin(), planeConnections_.end(),
        [&id](const PlaneConnection& conn) {
            return conn.plane->get_id() == id;
        });

    if (connIt == planeConnections_.end()) {
        LOG_ERROR("Radar", "Plane " + id + " not found");
        return -1;
    }

   	// store connection id and plane pointer before removing connection
    coidToRemove = connIt->coid;
    planeToRemove = connIt->plane;

    // Find and remove from planes_ vector
    auto planeIt = std::find_if(planes_.begin(), planes_.end(),
        [&id](const Plane* plane) {
            return plane->get_id() == id;
        });

    if (planeIt == planes_.end()) {
        LOG_ERROR("Radar", "Plane " + id + " not found");
        return -1;
    }

    // Stop the plane thread
    planeToRemove->stop();
    ConnectDetach(coidToRemove);
    usleep(10000); // Sleep for 1ms to allow the plane to stop

    // remove plane from planes_ vector and planeConnections_ vector
   	planeConnections_.erase(connIt);
    planes_.erase(planeIt);

    delete planeToRemove;
	return 0;
    }
}

void Radar::update_planes() {
    std::vector<PlaneState> aircraftData;
    std::vector<std::string> planesToRemove;


    std::lock_guard<std::mutex> lock(planeMtx);
    for (auto& conn : planeConnections_) {
        PlaneResponseMsg responseMsg;
        if(!query_plane(conn, responseMsg)) {
            LOG_ERROR("Radar", "Failed to query plane " + conn.plane->get_id());
            continue;
        }

            PlaneState state;
            // Copy ID using strncpy
            strncpy(state.id, responseMsg.data.id, sizeof(state.id));
            state.id[sizeof(state.id) - 1] = '\0'; // Ensure null termination

            state.position = Vector(responseMsg.data.x, responseMsg.data.y, responseMsg.data.z);
            state.velocity = Vector(responseMsg.data.speedX, responseMsg.data.speedY, responseMsg.data.speedZ);
            state.coid_comp = conn.coid_comp;

//             check if plane is in bounds
            if (!radarBounds.contains(state.position)) {
               LOG_WARNING("Radar",
                "Plane " + std::string(state.id) + " out of bounds at (" +
                std::to_string(state.position.x) + ", " +
                std::to_string(state.position.y) + ", " +
                std::to_string(state.position.z) + ")");
                planesToRemove.push_back(state.id);
                continue;
            }

            state.coid = conn.coid;
            aircraftData.push_back(state);
//        LOG_INFO("Radar", "Plane " + state.id + " is at position (" + std::to_string(state.position.x) + ", " +
//                 std::to_string(state.position.y) + ", " + std::to_string(state.position.z) + ")");
       }




    // Send data to ComputerSystem
    RadarToComputerMsg radarMsg;
    radarMsg.numAircraft = aircraftData.size();
    if (radarMsg.numAircraft < 0) {
        LOG_ERROR("Radar", "Invalid number of aircraft: " + std::to_string(radarMsg.numAircraft));
        return;
    }
    for (int i = 0; i < radarMsg.numAircraft; ++i) {
        radarMsg.aircraftData[i] = aircraftData[i];
    }

    int status = MsgSend(computerSystemCoid_, &radarMsg, sizeof(radarMsg), nullptr, 0);
    if (status == -1) {
        LOG_ERROR("Radar", "Failed to send data to ComputerSystem: " + std::string(strerror(errno)));
    }
    LOG_INFO("Radar", "Sent " + std::to_string(radarMsg.numAircraft) + " aircraft to ComputerSystem");

    for (const auto& id : planesToRemove) {
      remove_plane(id);
  	}
}

bool Radar::query_plane(const PlaneConnection& conn, PlaneResponseMsg& responseMsg) {
    RadarQueryMsg queryMsg;
    int status = MsgSend(conn.coid, &queryMsg, sizeof(queryMsg), &responseMsg, sizeof(responseMsg));
    if (status == -1) {
        LOG_ERROR("Radar", "MsgSend to Plane " + conn.plane->get_id() + " failed: " + strerror(errno));
        return false;
    }
    return true;
}
