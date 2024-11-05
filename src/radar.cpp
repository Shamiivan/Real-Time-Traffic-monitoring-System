#include "radar.h"
#include <sys/netmgr.h>
#include <sys/neutrino.h>

Radar::Radar() : running(false), thread_id(-1) {
    timer = make_unique<Timer>(POLLING_INTERVAL, [this](){
        update_planes();
    });
}

Radar::~Radar() {
    stop();
}

void Radar::start() {
  timer->start();
}


void Radar::stop() {
}

void *Radar::polling_worker(void *arg) {
    Radar *radar = static_cast<Radar *>(arg);
    radar->run();
    return NULL;
}


void Radar::run() {

}

int Radar::add_plane(string id, Vector position, Vector speed) {

    lock_guard<mutex> lock(mtx);
    planes.emplace_back(new Plane(move(id), position, speed));
    return 0;
 }

 void Radar::update_planes() {
   printf("Updating planes\n");
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto& plane : planes) {
        Vector pos = plane->update_position();
        std::cout << "Plane " << plane->get_id() << ": ("
                  << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
    }
}
