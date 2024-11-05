#ifndef RADAR_H
#define RADAR_H

#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <string>
#include "plane.h"

using std::string;
using std::pair;
using std::map;
using std::shared_ptr;
using std::vector;
using std::mutex;
using std::unique_ptr;
using std::lock_guard;
using std::atomic;
using std::move;
using std::to_string;
using std::make_unique;
using std::find_if;

struct timer_cfg {
    int chid;
    timer_t id;
    struct sigevent event;
    struct itimerspec itime;
    int priority{10};
};

class Radar {
  public:
    Radar();
    ~Radar();

    // plane management
    int add_plane(string id, Vector position, Vector speed);

    void start();
    void stop();
  private:
    string program_name; // acts as a logging tag
    static constexpr int POLLING_INTERVAL_MS = 1000;
    vector<unique_ptr<Plane>> planes; // planes in the radar

    // thread management
    atomic<bool> running;
    int thread_id;
    static void* polling_worker(void* arg);
    mutable mutex mtx;
    void run();


    // timer
    timer_cfg timer;
};
#endif //RADAR_H
