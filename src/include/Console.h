#ifndef CONSOLE_H
#define CONSOLE_H
#include <iostream>

enum class ConsoleCommand {
  DIPLAY_PLANE_DATA = 1,
  UPDATE_PLANE_VELOCITY = 2,
  UPDATE_PLANE_POSITION = 3,
};

class Console {
  public :
    Console();
    Console(int computerSystemCoid);
    ~Console();

    void start();
    void stop();

    ConsoleCommand getCommand();
    void displayHep();
   private:
     void *threadFunc(void *arg); //
     void run();
   private:
     thread_t thread_;
     const int computerSystemCoid_;
     std::atomic<bool> running_;
};



#endif //CONSOLE_H
