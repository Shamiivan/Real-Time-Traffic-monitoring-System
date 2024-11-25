#include "Console.h"
#include "messages.h"
#include <sys/neutrino.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include "Logger.h"
#include <unistd.h>


Console::Console(int computerSystemCoid)
    : computerSystemCoid_(computerSystemCoid), running_(false) {}


Console::~Console() {
    stop();
}

void Console::start() {
    running_ = true;
    int ret = pthread_create(&thread_, nullptr, Console::threadFunc, this);
    if (ret != 0) {
        LOG_ERROR("Console", "Failed to create thread");
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Console", "Console thread started.");
}
void Console::stop() {
    if (running_) {
        running_ = false;
        pthread_join(thread_, nullptr);
    }
}

void* Console::threadFunc(void* arg) {
    Console* self = static_cast<Console*>(arg);
    self->run();
    return nullptr;
}

void Console::run() {
    displayHelp();
    while (running_) {
        Status status = processUserInput();
          if(status ==Status::ERROR){
              LOG_ERROR("Console", "Failed to process user input");
          }
          usleep(100000); // 100 ms
    }
}

Status Console::processUserInput(){
  std::string input;
    LOG_WARNING("Console", "Enter a command (h for help : ");
    std::getline(std::cin, input);
    if(input.empty()){
        return Status::ERROR;
    }
    char command = input[0];
    switch(command){
        case 'h':
            displayHelp();
            break;
        case '1':
            break;
        case '2':
            break;
        case '3':
            break;
        case '4':
            running_ = false;
            break;
        default:
            LOG_WARNING("Console", "Invalid command");
            break;
	return Status::OK;
    };
}


void Console::displayHelp() {
    LOG_INFO("Console", "Displaying help menu");
    std::stringstream helpMenu;

    helpMenu << "Console Commands:\n";
    helpMenu << "1. Display Plane Data\n";
    helpMenu << "2. Update Plane Velocity\n";
    helpMenu << "3. Update Plane Position\n";
    helpMenu << "4. Exit\n";
    LOG_WARNING("Console", helpMenu.str());
}
