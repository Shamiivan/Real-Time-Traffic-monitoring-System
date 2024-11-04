#include <iostream>
#include "plane.h"
#include <vector>
#include <chrono>
#include <pthread.h>
#include <sys/neutrino.h>
#include <thread>
#include <memory>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>

int main() {
    // Process information
    pid_t pid = getpid();
    pid_t ppid = getppid();

    std::cout << "Process Information:" << std::endl;
    std::cout << "Process ID (PID): " << pid << std::endl;
    std::cout << "Parent Process ID (PPID): " << ppid << std::endl;
    const int PATH_MAX = 100;

    // Get current working directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current Working directory is : " << cwd << std::endl;

        // Get parent directory
        char* parent = dirname(cwd);
        std::cout << "Parent directory is : " << parent << std::endl;
    } else {
        perror("getcwd() error");
    }

    // Get node information
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        std::cout << "Running on node: " << hostname << std::endl;
    }

    // Show CPU information
    std::cout << "Running on CPU: " << ThreadCtl(_NTO_TCTL_RUNMASK, 0) << std::endl;

    std::cout << "\nStarting plane simulation...\n" << std::endl;

    std::vector<std::unique_ptr<Plane>> planes;
    planes.emplace_back(std::make_unique<Plane>("1", Vector{0, 0, 0}, Vector{1, 1, 1}));
    planes.emplace_back(std::make_unique<Plane>("2", Vector{0, 0, 0}, Vector{30, 20, 10}));
    planes.emplace_back(std::make_unique<Plane>("3", Vector{0, 0, 0}, Vector{5, 5, 5}));

    for (auto& plane : planes) {
        plane->start();
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "\nPlane positions after 5 seconds:\n" << std::endl;
    for (auto& plane : planes) {
        printf("Plane %s: (%f, %f, %f)\n",
               plane->get_id().c_str(),
               plane->get_pos().x,
               plane->get_pos().y,
               plane->get_pos().z);
    }

    for (auto& plane : planes) {
        plane->stop();
    }

    return 0;
}
