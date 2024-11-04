#ifndef RADAR_H
#define RADAR_H

#include <atomic>
#include <map>
#include "plane.h"

using std::string, std::pair, std::map, std::shared_ptr, std::vector, std::mutex;

struct timer_cfg {
    int chid;
    timer_t id;
    struct sigevent event;
    struct itimerspec itime;
    const int priority = 10;
};

class Radar {
  public:
    Radar();
    ~Radar();

    // plane management
    bool add_plane(const Plane& plane);
    bool remove_plane(const std::string& id);

    void start();
    void stop();
  private:
    string program_name; // acts as a logging tag
    static constexpr int POLLING_INTERVAL_MS = 1000;


    // thread management
    int thread_id;
    static void* polling_worker(void* arg);
    mutex planes_mtx;
    atomic<bool> running;


    // timer
    timer_cfg timer;
};
#endif //RADAR_H
