//#include "radar.h"
//#include <sys/neutrino.h>
//
//Radar::Radar() : running(false), thread_id(-1) {
// // setup timer for polling planes
// string msg = "Successfully created radar";
//    timer.chid = ChannelCreate(0);
//    if (timer.chid == -1) {
//        msg = "Failed to create channel";
//        printf("%s: %s: %s\n", program_name,msg, strerror(errno));
//        exit(EXIT_FAILURE);
//    }
//
//    timer.event.sigev_notify = SIGEV_PULSE;
//    timer.event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, timer.chid, _NTO_SIDE_CHANNEL, 0);
//
//    if (timer.event.sigev_coid == -1) {
//        msg = "Failed to attach channel";
//        printf("%s: %s: %s\n", program_name,msg, strerror(errno));
//        exit(EXIT_FAILURE);
//    }
//
//    timer.event.sigev_priority = timer.priority;
//    timer.event.sigev_code = _PULSE_CODE_MINAVAIL;
//
//    int status = timer_create(CLOCK_MONOTONIC, &timer.event, &timer.id);
//    if (status == -1) {
//        msg = "Failed to create timer";
//        printf("%s: %s: %s\n", program_name,msg, strerror(errno));
//        exit(EXIT_FAILURE);
//    }
//
//    timer.itime.it_value.tv_sec = POLLING_INTERVAL_MS / 1000;
//    timer.itime.it_value.tv_nsec = 0;
//    timer.itime.it_interval.tv_sec = POLLING_INTERVAL_MS / 1000;
//    timer.itime.it_interval.tv_nsec = 0;
//
//    status = timer_settime(timer.id, 0, &timer.itime, NULL);
//    if (status == -1) {
//        msg = "Failed to set timer";
//        printf("%s: %s: %s\n", program_name,msg, strerror(errno));
//        exit(EXIT_FAILURE);
//    }
//
//}
//
//Radar::~Radar() {
//    stop();
//    timer_delete(timer.id);
//    ConnectDetach(timer.event.sigev_coid);
//    ChannelDestroy(timer.chid);
//}
//
//void Radar::start() {
//  string msg = "Successfully created thread";
//  // check if running is true, using atomic set and check
//    if (running.exchange(true)) {return;}
//    thread_id = ThreadCreate(0, Radar::polling_worker, this, NULL);
//    program_name = "Radar" + to_string(thread_id);
//    if (thread_id == -1) {
//        msg = "Failed to create thread";
//        printf("%s: %s: %s\n", program_name,msg, strerror(errno));
//        exit(EXIT_FAILURE);
//    }
//    printf("%s: %s: %s\n", program_name,msg, strerror(errno));
//}
//
//
//void Radar::stop() {
//    string msg = "Successfully destroyed thread";
//    // check if running is false, using atomic set and check
//    if (!running.exchange(false)) {return;}
//    if (thread_id != 0) {
//        ThreadJoin(thread_id, NULL);
//        ThreadDestroy(thread_id, 0, NULL);
//        thread_id = -1;
//    }
//    printf("%s: %s: %s\n", program_name,msg, strerror(errno));
//}
//
//
//void *Radar::polling_worker(void *arg) {
//    Radar *radar = static_cast<Radar *>(arg);
//    string msg = "Polling worker started";
//    printf("%s: %s: %s\n", radar->program_name,msg, strerror(errno));
//    while (radar->running) {
//        printf("Polling worker running\n");
//    }
//    printf("Polling worker stopped\n");
//    return NULL;
//}
//
//int add_plane(const Plane& plane) {
//    return 0;
//}
