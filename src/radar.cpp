// radar.cpp
#include "radar.h"
#include "messages.h"
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <iostream>
#include <unistd.h>
#include <cstring>

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

int Radar::add_plane(std::string id, Vector position, Vector speed) {
    Plane* plane = new Plane(id, position, speed);
    plane->start();

    // Connect to the Plane's channel
    int coid = ConnectAttach(ND_LOCAL_NODE, 0, plane->getChannelId(), _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) {
        perror("Radar: Failed to connect to Plane channel");
        plane->stop();
        delete plane;
        return -1;
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        planes_.push_back(plane);
        PlaneConnection conn = { plane, coid };
        planeConnections_.push_back(conn);
    }

    return 0;
}

void Radar::update_planes() {
    std::vector<PlaneState> aircraftData;

    std::lock_guard<std::mutex> lock(mtx);
    for (auto& conn : planeConnections_) {
        PlaneResponseMsg responseMsg;
        if (query_plane(conn, responseMsg)) {
            PlaneState state;
            // Copy ID using strncpy
            strncpy(state.id, responseMsg.data.id, sizeof(state.id));
            state.id[sizeof(state.id) - 1] = '\0'; // Ensure null termination

            state.position = Vector(responseMsg.data.x, responseMsg.data.y, responseMsg.data.z);
            state.velocity = Vector(responseMsg.data.speedX, responseMsg.data.speedY, responseMsg.data.speedZ);
            aircraftData.push_back(state);

            // Display updates
            std::cout << "Radar received data from Plane " << state.id << ":\n"
                      << "  Position: (" << state.position.x << ", " << state.position.y << ", " << state.position.z << ")\n"
                      << "  Velocity: (" << state.velocity.x << ", " << state.velocity.y << ", " << state.velocity.z << ")\n";
        } else {
            std::cerr << "Radar: Failed to query plane " << conn.plane->get_id() << "\n";
        }
    }

    // Send data to ComputerSystem
    RadarToComputerMsg radarMsg;
    radarMsg.numAircraft = aircraftData.size();
    for (int i = 0; i < radarMsg.numAircraft; ++i) {
        radarMsg.aircraftData[i] = aircraftData[i];
    }

    int status = MsgSend(computerSystemCoid_, &radarMsg, sizeof(radarMsg), nullptr, 0);
    if (status == -1) {
        perror("Radar: Failed to send data to ComputerSystem");
    }
}

bool Radar::query_plane(const PlaneConnection& conn, PlaneResponseMsg& responseMsg) {
    RadarQueryMsg queryMsg;
    int status = MsgSend(conn.coid, &queryMsg, sizeof(queryMsg), &responseMsg, sizeof(responseMsg));
    if (status == -1) {
        std::cerr << "Radar: MsgSend to Plane " << conn.plane->get_id() << " failed: " << strerror(errno) << "\n";
        return false;
    }
    return true;
}



