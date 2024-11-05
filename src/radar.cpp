#include "radar.h"
#include <sys/netmgr.h>
#include <sys/neutrino.h>

#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL
Radar::Radar() : running(false), thread_id(-1) {
 // setup timer for polling planes
 string msg = "Successfully created radar";
    timer.chid = ChannelCreate(0);
    if (timer.chid == -1) {
        printf("Failed to create channel");
        exit(EXIT_FAILURE);
    }

    timer.event.sigev_notify = SIGEV_PULSE;
    timer.event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, timer.chid, _NTO_SIDE_CHANNEL, 0);

    if (timer.event.sigev_coid == -1) {
        msg = "Failed to attach channel";
        printf("%s: %s: %s\n", program_name,msg, strerror(errno));
        exit(EXIT_FAILURE);
    }

    timer.event.sigev_priority = timer.priority;
    timer.event.sigev_code = MY_PULSE_CODE;

    int status = timer_create(CLOCK_REALTIME, &timer.event, &timer.id);
    if (status == -1) {
        msg = "Failed to create timer";
        printf("%s: %s: %s\n", program_name,msg, strerror(errno));
        exit(EXIT_FAILURE);
    }

    timer.itime.it_value.tv_sec = 1;
    timer.itime.it_value.tv_nsec = 0;
    timer.itime.it_interval.tv_sec = 1;
    timer.itime.it_interval.tv_nsec = 0;

    status = timer_settime(timer.id, 0, &timer.itime, NULL);
    if (status == -1) {
        msg = "Failed to set timer";
        printf("%s: %s: %s\n", program_name,msg, strerror(errno));
        exit(EXIT_FAILURE);
    }

}

Radar::~Radar() {
    stop();
    timer_delete(timer.id);
    ConnectDetach(timer.event.sigev_coid);
    ChannelDestroy(timer.chid);
}

void Radar::start() {
  string msg = "Successfully created thread";
  // check if running is true, using atomic set and check
    if (running.exchange(true)) {return;}
    running = true;
    thread_id = ThreadCreate(0, Radar::polling_worker, this, NULL);
    program_name = "Radar " + to_string(thread_id);
    if (thread_id == -1) {
        msg = "Failed to create thread";
        printf("%s: %s: %s\n", program_name,msg, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Radar is created and channel ID is %d\n", timer.chid);
}


void Radar::stop() {
    string msg = "Successfully destroyed thread";
    // check if running is false, using atomic set and check
    if (!running.exchange(false)) {return;}
    if (thread_id != 0) {
        ThreadJoin(thread_id, NULL);
        ThreadDestroy(thread_id, 0, NULL);
        thread_id = -1;
    }
    printf("%s: %s: %s\n", program_name,msg, strerror(errno));
}

void *Radar::polling_worker(void *arg) {
    Radar *radar = static_cast<Radar *>(arg);
    radar->run();
    return NULL;
}


void Radar::run() {
    struct _pulse pulse;
    int rcvid;

    printf("Polling worker started\n");
    while (running) {
        rcvid = MsgReceive(timer.chid, &pulse, sizeof(pulse), NULL);
        if (rcvid == -1) {
            printf("%s: Failed to receive message: %s\n", program_name.c_str(), strerror(errno));
            break;
        }
        if(pulse.code == MY_PULSE_CODE) {
          printf("Polling worker received pulse\n");
            lock_guard<mutex> lock(mtx);
            for (auto& plane : planes) {
            	plane->update_position();
                printf("Plane %s: (%f, %f, %f)\n",
                       plane->get_id().c_str(),
                       plane->get_pos().x,
                       plane->get_pos().y,
                       plane->get_pos().z);
            }
        }

    }
    printf("%s: Polling worker stopped\n", program_name.c_str());
}

int Radar::add_plane(string id, Vector position, Vector speed) {

    lock_guard<mutex> lock(mtx);
    planes.emplace_back(new Plane(move(id), position, speed));
    return 0;
 }
