#include "timer.h"
#include <sys/neutrino.h>
#include <iostream>

using std::function;
Timer::Timer(int _interval, function<void()> callback)
    : interval(_interval), running(false), callback(callback) {
    init();
}

Timer::~Timer() {
    stop();
    timer_delete(timer_id);
    ConnectDetach(event.sigev_coid);
    ChannelDestroy(chid);
}

void Timer::init(){
  chid = ChannelCreate(0);
  if (chid == -1) {
    printf("Failed to create channel");
    exit(EXIT_FAILURE);
  }

  event.sigev_notify = SIGEV_PULSE;
  event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);

  if (event.sigev_coid == -1) {
    printf("Failed to attach channel");
    exit(EXIT_FAILURE);
  }

  event.sigev_priority = PRIORITY;
  event.sigev_code = _PULSE_CODE_MINAVAIL;

  timer_spec.it_value.tv_sec = interval;
  timer_spec.it_value.tv_nsec = 0;
  timer_spec.it_interval.tv_sec = interval;
  timer_spec.it_interval.tv_nsec = 0;

  int status = timer_create(CLOCK_REALTIME, &event, &timer_id);
  if (status == -1) {
    printf("Failed to create timer");
    exit(EXIT_FAILURE);
  }
}

void Timer::start() {
    if (running.exchange(true)) {return;}
    running = true;

    // set the timer
    int status = timer_settime(timer_id, 0, &timer_spec, NULL);
    if (status == -1) {
        printf("Failed to set timer");
        exit(EXIT_FAILURE);
    }

    thread_id = ThreadCreate(0, Timer::timer_thread, this, NULL);
    if (thread_id == -1) {
        printf("Failed to create thread");
        exit(EXIT_FAILURE);
    }
}
void Timer::stop() {
  if (!running.exchange(false)) {return;}
    running = false;
    ThreadJoin(thread_id, NULL);
    ThreadDestroy(thread_id, 0, NULL);
}

void* Timer::timer_thread(void* arg) {
    Timer* timer = static_cast<Timer*>(arg);
    timer->run();
    return NULL;
}

void Timer::run() {
  struct _pulse pulse;
  int rcvid;

    while (running) {
        rcvid = MsgReceive(chid, &pulse, sizeof(pulse), NULL);
        if (rcvid == 0 && pulse.code == _PULSE_CODE_MINAVAIL) {
          callback();
        }
    }
}

int Timer::get_chid() const {
    return chid;
}
