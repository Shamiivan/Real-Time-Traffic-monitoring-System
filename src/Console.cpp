#include "Console.h"
#include "messages.h"
#include <sys/neutrino.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include "Logger.h"


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
    while (running_) {
        ConsoleCommand command = getCommand();
        if (command == ConsoleCommand::DISPLAY_PLANE_DATA){
          LOG_WARNING("Console", "Please  enter the plane id");
        }
    }
}
Console::dispayHelp() {
    LOG_INFO("Console", "Displaying help menu");
    std::stringstream helpMenu;

    helpMenu << "Console Commands:\n";
    helpMenu << "1. Display Plane Data\n";
    helpMenu << "2. Update Plane Velocity\n";
    helpMenu << "3. Update Plane Position\n";
    helpMenu << "4. Exit\n";
    LOG_WARNING("Console", helpMenu.str());
}
