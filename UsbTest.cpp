// UsbTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "UsbActions.hpp"
#include <memory>
#include <thread>
#include <csignal>

namespace
{
    volatile std::sig_atomic_t gSignalStatus;
}

void signal_handler(int signal)
{
    gSignalStatus = 1;
    std::cerr << "exit this bitch" << std::endl;
}

int main()
{
    std::unique_ptr<UsbActions> usbActions = std::make_unique<UsbActions>();
    std::signal(SIGINT, signal_handler);
    for (int i = 0; i < 10000; i++) {
        if (gSignalStatus > 0) {
            break;
        }
        usbActions->getDDJWheelPosition();
        //std::cout << "waiting 3 seconds, i=" << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
