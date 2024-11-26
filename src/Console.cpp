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
        	return Status::OK;
            break;
        case '0':
        	return listPlanes();
        case '1':
        	return displayPlaneData();
            break;
        case '2':
            return updatePlaneVelocity();
            break;
        case '3':
        	return Status::OK;
            break;
        case '4':
            running_ = false;
        	return Status::OK;
            break;
        default:
            LOG_WARNING("Console", "Invalid command");
            break;
    };
	return Status::OK;
}


Status Console::listPlanes() {
    OperatorCommandMsg msg;
    msg.type = ConsoleCommand::LIST_PLANES;

    // Buffer to receive plane list
    PlaneListMsg planesList;
    int status = MsgSend(computerSystemCoid_, &msg, sizeof(msg), &planesList, sizeof(planesList));
    if (status != EOK) {
        LOG_ERROR("Console", "Failed to get plane data");
        return Status::ERROR;
    }

    // Create formatted output
    std::stringstream ss;
    ss << "\nCurrent Planes in System:\n";
    ss << "ID     | Position (x,y,z)        | Velocity (x,y,z)\n";
    ss << "-----------------------------------------------------\n";

    for(int i = 0; i < planesList.numPlanes; i++) {
        ss << planesList.planes[i].id << " | ("
           << planesList.planes[i].position.x << ","
           << planesList.planes[i].position.y << ","
           << planesList.planes[i].position.z << ") | ("
           << planesList.planes[i].velocity.x << ","
           << planesList.planes[i].velocity.y << ","
           << planesList.planes[i].velocity.z << ")\n";
    }

    LOG_WARNING("Console", ss.str());
    return Status::OK;
}

Status Console::displayPlaneData(){
	std::string planeId;
	PlaneState state;

	LOG_WARNING("Console", "Enter plane ID: ");
	std::getline(std::cin, planeId);

	OperatorCommandMsg msg;
	msg.type = ConsoleCommand::DISPLAY_PLANE_DATA;
	strncpy(msg.planeId, planeId.c_str(), sizeof(msg.planeId) - 1);
	msg.planeId[sizeof(msg.planeId) - 1] = '\0';

	int status = MsgSend(computerSystemCoid_, &msg, sizeof(msg), nullptr, 0);
	if (status != EOK) {
	        LOG_ERROR("Console", "Failed to send velocity update command");
	        return Status::ERROR;
	}

	LOG_INFO("Console", "Request for plane data display sent");
	    return Status::OK;
}

Status Console::updatePlaneVelocity() {
    std::string planeId;
    double x, y, z;

    LOG_WARNING("Console", "Enter plane ID: ");
    std::getline(std::cin, planeId);

    LOG_WARNING("Console", "Enter new velocity (x y z): ");
    std::string velocityInput;
    std::getline(std::cin, velocityInput);

    std::stringstream ss(velocityInput);
    if (!(ss >> x >> y >> z)) {
        LOG_ERROR("Console", "Invalid velocity format. Use: x y z");
        return Status::ERROR;
    }

    OperatorCommandMsg msg;
    msg.type = ConsoleCommand::UPDATE_PLANE_VELOCITY;
    strncpy(msg.planeId, planeId.c_str(), sizeof(msg.planeId) - 1);
    msg.planeId[sizeof(msg.planeId) - 1] = '\0';
    msg.velocity = Vector(x, y, z);

    int status = MsgSend(computerSystemCoid_, &msg, sizeof(msg), nullptr, 0);
    if (status != EOK) {
        LOG_ERROR("Console", "Failed to send velocity update command");
        return Status::ERROR;
    }

    LOG_INFO("Console", "Velocity update command sent");
    return Status::OK;
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
