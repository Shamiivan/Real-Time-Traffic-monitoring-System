// radar.h
#ifndef RADAR_H
#define RADAR_H

#include <vector>
#include <string>
#include <mutex>
#include <pthread.h>
#include "plane.h"
#include "messages.h"

struct PlaneConnection {
   Plane* plane;
   int coid; // Connection ID to the Plane's channel
   int coid_comp; //connection ID for computer to plane's computer channel
};

struct Bounds {
        static constexpr double MIN_X = 0.0;
        static constexpr double MAX_X = 100000.0;
        static constexpr double MIN_Y = 0.0;
        static constexpr double MAX_Y = 100000.0;
        static constexpr double MIN_Z = 0.0;
        static constexpr double MAX_Z = 20000.0;

        bool contains(const Vector& position) const {
            return position.x >= MIN_X && position.x <= MAX_X &&
                   position.y >= MIN_Y && position.y <= MAX_Y &&
                   position.z >= MIN_Z && position.z <= MAX_Z;
        }
    };

class Radar {
public:
    Radar(int computerSystemCoid);
    ~Radar();

    void start();
    void stop();

    int add_plane(std::string id, Vector position, Vector speed);
    int getPlaneCount() { return planes_.size(); }

private:
    static void* threadFunc(void* arg);
    void run();
    void update_planes();
    int remove_plane(std::string id);
    bool query_plane(const PlaneConnection& conn, PlaneResponseMsg& responseMsg);
private :
    std::vector<Plane*> planes_;
    std::vector<PlaneConnection> planeConnections_;
    pthread_t thread_;
    bool running_;
    std::mutex mtx;
    std::mutex planeMtx;
    int computerSystemCoid_;
    const Bounds radarBounds{};  // Using default initialization with constants
};

#endif // RADAR_H
