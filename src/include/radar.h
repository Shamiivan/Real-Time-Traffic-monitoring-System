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
#include "timer.h"

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

class Radar {
  public:
    Radar();
    ~Radar();

    // plane management
    int add_plane(string id, Vector position, Vector speed);

    void start();
    void stop();
  private:
    int interval;
    vector<unique_ptr<Plane>> planes; // planes in the radar

    // thread management
    atomic<bool> running;
    int thread_id;
    static void* polling_worker(void* arg);
    mutable mutex mtx;
    void run();
    void update_planes();

    unique_ptr<Timer> timer;
    const int POLLING_INTERVAL= 1;
};
#endif //RADAR_H
