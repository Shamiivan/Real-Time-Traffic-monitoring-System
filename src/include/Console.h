#ifndef CONSOLE_H
#define CONSOLE_H
#include <iostream>
#include <Config.h>

class Console {
  public :
    Console();
    Console(int computerSystemCoid);
    ~Console();

    void start();
    void stop();

    Status processUserInput();
    Status updatePlaneVelocity();
    Status listPlanes();
    void displayHelp();

   private:
     static void *threadFunc(void *arg); //
     void run();
   private:
     pthread_t thread_;
     const int computerSystemCoid_;
     std::atomic<bool> running_;
};



#endif //CONSOLE_H
