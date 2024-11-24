// radar.h
#ifndef RADAR_H
#define RADAR_H

#include <vector>
#include <string>
#include <mutex>
#include <pthread.h>
#include "plane.h"
#include "messages.h"

class Radar {
public:
    Radar(int computerSystemCoid);
    ~Radar();

    void start();
    void stop();

    int add_plane(std::string id, Vector position, Vector speed);

private:

    static void* threadFunc(void* arg);
    void run();

    void update_planes();

    struct PlaneConnection {
        Plane* plane;
        int coid; // Connection ID to the Plane's channel
        int coid_comp; //connection ID for computer to plane's computer channel
    };

    bool query_plane(const PlaneConnection& conn, PlaneResponseMsg& responseMsg);

    std::vector<Plane*> planes_;
    std::vector<PlaneConnection> planeConnections_;
    pthread_t thread_;
    bool running_;
    std::mutex mtx;
    std::mutex planeMtx;

    int computerSystemCoid_;
};

#endif // RADAR_H
