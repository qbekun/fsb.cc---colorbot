#ifndef MOUSE_H
#define MOUSE_H

#pragma execution_character_set("utf-8")

#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>
#include "settings.h" // Assuming settings.h exists for the Settings class
#include <serial.h>

class Mouse {
private:
    Settings settings; // Settings initialization
    std::mutex lock;
    serial::Serial serial_port;
    float remainder_x;
    float remainder_y;

    std::string find_serial_port() {
        std::string com_port = settings.get("Settings", "COM-Port");
        std::vector<serial::PortInfo> ports = serial::list_ports();
        auto it = std::find_if(ports.begin(), ports.end(),
            [&com_port](const serial::PortInfo& port) {
                return port.description.find(com_port) != std::string::npos ||
                    port.port.find(com_port) != std::string::npos; // Compare port as well
            });
        if (it != ports.end()) {
            return it->port;
        }
        else {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            std::exit(1);
        }
    }

public:
    Mouse(Settings& settings)
        : settings(settings), remainder_x(0.0), remainder_y(0.0) {
        serial_port.setBaudrate(115200);
        serial_port.setTimeout(serial::Timeout::max(), 0, 0, 0, 0);
        serial_port.setPort(find_serial_port());
        try {
            serial_port.open();
        }
        catch (const serial::SerialException& e) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            std::exit(1);
        }
    }

    void move(float x, float y) {
        x += remainder_x;
        y += remainder_y;
        int move_x = static_cast<int>(x);
        int move_y = static_cast<int>(y);
        remainder_x = x - move_x;
        remainder_y = y - move_y;
        if (move_x != 0 || move_y != 0) {
            std::lock_guard<std::mutex> guard(lock);
            serial_port.write("hentai" + std::to_string(move_x) + "," + std::to_string(move_y) + "\n");
        }
    }

    void click() {
        std::lock_guard<std::mutex> guard(lock);
        serial_port.write("C\n");
    }
};

#endif // MOUSE_H
