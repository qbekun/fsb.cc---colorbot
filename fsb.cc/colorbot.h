#ifndef COLORBOT_H
#define COLORBOT_H

#include <opencv2/opencv.hpp>
#include <random>
#include <thread>
#include <iostream>
#include "Capture.h"
#include "Mouse.h"
#include "Settings.h"

class Colorbot {
private:
    Capture* capturer;   // Pointer to Capture class
    Mouse* mouse;        // Pointer to Mouse class
    Settings* settings;  // Pointer to Settings class
    cv::Scalar lower_color;
    cv::Scalar upper_color;

    bool aim_enabled;
    int aim_key;
    float x_speed;
    float y_speed;
    int x_fov;
    int y_fov;
    float target_offset;
    bool trigger_enabled;
    int trigger_key;
    int min_delay;
    int max_delay;
    int x_range;
    int y_range;

    bool bunny_hop_enabled;
    int bunny_hop_key;
    bool bunny_hop_thread_running;

    bool magnet_enabled;
    int magnet_key;
    float magnet_smooth;
    int magnet_fov;


    cv::Mat kernel;
    cv::Point screen_center;
    cv::Point prev_center;  // Previous aim center for smoothing
    float smoothing_factor; // Smoothing factor for aim

    cv::Point smooth_aim(const cv::Point& current_center) {
        return cv::Point(
            static_cast<int>(smoothing_factor * current_center.x + (1 - smoothing_factor) * prev_center.x),
            static_cast<int>(smoothing_factor * current_center.y + (1 - smoothing_factor) * prev_center.y)
        );
    }

public:
    // Constructor
    Colorbot(int x, int y, int fov_x, int fov_y, Settings* settings)
        : settings(settings) {
        capturer = new Capture(x, y, fov_x, fov_y);
        mouse = new Mouse(*settings); // Pass settings by reference

        // Initialize color ranges
        lower_color = cv::Scalar(150, 76, 123);
        upper_color = cv::Scalar(160, 197, 255);

        capturer->setColorRanges(lower_color, upper_color);

        // Aimbot settings
        aim_enabled = settings->get_boolean("Aimbot", "Enabled");
        aim_key = std::stoi(settings->get("Aimbot", "toggleKey"), nullptr, 16);
        x_speed = settings->get_float("Aimbot", "xSpeed");
        y_speed = settings->get_float("Aimbot", "ySpeed");
        x_fov = settings->get_int("Aimbot", "xFov");
        y_fov = settings->get_int("Aimbot", "yFov");
        target_offset = settings->get_float("Aimbot", "targetOffset");
        smoothing_factor = settings->get_float("Aimbot", "smoothingFactor"); // Read smoothing factor from settings

        // Triggerbot settings
        trigger_enabled = settings->get_boolean("Triggerbot", "Enabled");
        trigger_key = std::stoi(settings->get("Triggerbot", "toggleKey"), nullptr, 16);
        min_delay = settings->get_int("Triggerbot", "minDelay");
        max_delay = settings->get_int("Triggerbot", "maxDelay");
        x_range = settings->get_int("Triggerbot", "xRange");
        y_range = settings->get_int("Triggerbot", "yRange");

        // Bunny hop settings
        bunny_hop_enabled = false;
        bunny_hop_key = VK_F2;  // F2 key to toggle bunny hop
        bunny_hop_thread_running = false;

        // Magnet Trigger settings
        magnet_enabled = settings->get_boolean("MagnetTrigger", "Enabled");
        magnet_fov = settings->get_int("MagnetTrigger", "Fov");
        magnet_smooth = settings->get_float("MagnetTrigger", "Smooth");
        magnet_key = std::stoi(settings->get("MagnetTrigger", "Key"), nullptr, 16);
        if (magnet_enabled) {
            std::thread(&Colorbot::listen_magnet_trigger, this).detach();
        }


        // Precomputed values
        kernel = cv::Mat::ones(3, 3, CV_8U);
        screen_center = cv::Point(x_fov / 2, y_fov / 2);
        prev_center = screen_center;  // Initialize previous aim center

        // Start the bunny hop listener
        std::thread(&Colorbot::listen_bunny_hop, this).detach();
    }

    // Destructor
    ~Colorbot() {
        delete capturer;  // Clean up the dynamically allocated memory
        delete mouse;     // Clean up the dynamically allocated memory
        // Don't delete settings; assume it is managed elsewhere
    }

  //  void show_hsv_window() {
   //     cv::namedWindow("HSV View", cv::WINDOW_AUTOSIZE); // Create a window for display
    //    cv::Mat hsv, mask, screen;

    //    while (true) {
    //        screen = capturer->get_screen();
     //       if (screen.empty()) continue;

    //        cv::cvtColor(screen, hsv, cv::COLOR_BGR2HSV); // Convert to HSV color space

            // Create a binary mask where detected colors are white
   //         cv::inRange(hsv, lower_color, upper_color, mask);
   //         cv::imshow("HSV View", mask); // Show the mask
    //        if (cv::waitKey(30) >= 0) break; // Break the loop if any key is pressed
        }

   //     cv::destroyWindow("HSV View"); // Close the window
   // }


    void listen_bunny_hop() {
        while (true) {
            if (GetAsyncKeyState(bunny_hop_key) < 0) {
                bunny_hop_enabled = !bunny_hop_enabled;
                if (bunny_hop_enabled) {
                    if (!bunny_hop_thread_running) {
                        bunny_hop_thread_running = true;
                        std::thread(&Colorbot::perform_bunny_hop, this).detach();
                    }
                }
                else {
                    bunny_hop_thread_running = false;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));  // Debounce delay
        }
    }

    void perform_bunny_hop() {
        while (bunny_hop_enabled) {
            if (GetAsyncKeyState(VK_SPACE) < 0) {
                // Simulate a jump
                keybd_event(VK_SPACE, 0, 0, 0);  // Press space
                std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Delay between jumps
                keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);  // Release space
                std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Delay after releasing space
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        bunny_hop_thread_running = false;  // Mark thread as not running when finished
    }

    void listen_aimbot() {
        while (true) {
            if (GetAsyncKeyState(aim_key) < 0) {
                process("move");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Small sleep
        }
    }

    void listen_triggerbot() {
        while (true) {
            if (GetAsyncKeyState(trigger_key) < 0) {
                process("click");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Small sleep
        }
    }

    void listen_magnet_trigger() {
        while (true) {
            if (GetAsyncKeyState(magnet_key) < 0) {
                process("magnet");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void listen() {
        if (aim_enabled) {
            std::thread(&Colorbot::listen_aimbot, this).detach();
        }
        if (trigger_enabled) {
            std::thread(&Colorbot::listen_triggerbot, this).detach();
        }
        if (magnet_enabled) {
            std::thread(&Colorbot::listen_magnet_trigger, this).detach();
        }
    }

    void process(const std::string& action) {
        // Convert the captured screen to HSV color space
        cv::Mat hsv;
        cv::Mat screen = capturer->get_screen();
        if (screen.empty()) {
            return;
        }
        cv::cvtColor(screen, hsv, cv::COLOR_BGR2HSV);
        // Create a binary mask where detected colors are white
        cv::Mat mask;
        cv::inRange(hsv, lower_color, upper_color, mask);
        // Dilate the mask
        cv::Mat dilated;
        cv::dilate(mask, dilated, kernel, cv::Point(-1, -1), 5);
        // Thresholding
        cv::Mat thresh;
        cv::threshold(dilated, thresh, 60, 255, cv::THRESH_BINARY);
        // Find contours
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        if (!contours.empty()) {
            double min_distance = std::numeric_limits<double>::infinity();
            cv::Point closest_center;
            for (const auto& contour : contours) {
                // Find the contour closest to the center of the screen
                cv::Moments moments = cv::moments(contour);
                if (moments.m00 != 0) {  // Avoid division by zero
                    cv::Point center(static_cast<int>(moments.m10 / moments.m00), static_cast<int>(moments.m01 / moments.m00));
                    double distance = cv::norm(center - screen_center);
                    // Update the closest center if the distance is smaller
                    if (distance < min_distance) {
                        min_distance = distance;
                        closest_center = center;
                    }
                }
            }
            if (closest_center != cv::Point()) {
                closest_center = smooth_aim(closest_center);  // Apply smoothing to aim
                // Get the coordinates of the closest center and apply target offset
                int cX = closest_center.x;
                int cY = closest_center.y - static_cast<int>(target_offset);
                if (action == "move") {
                    // Calculate the difference between the center of the screen and the detected target
                    int x_diff = cX - screen_center.x;
                    int y_diff = cY - screen_center.y;
                    // Move the mouse towards the target
                    mouse->move(x_speed * x_diff, y_speed * y_diff);
                }
                else if (action == "click") {
                    // Check if the detected target is within the trigger
                    if (std::abs(cX - screen_center.x) <= x_range && std::abs(cY - screen_center.y) <= y_range) {
                        // Random delay before triggering a click
                        std::uniform_real_distribution<double> distribution(min_delay / 1000.0, max_delay / 1000.0);
                        std::default_random_engine generator;
                        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(distribution(generator) * 1000)));
                        mouse->click();
                    }
                }
                else if (action == "magnet") {
                    // Calculate the difference between the center of the screen and the detected target
                    int x_diff = closest_center.x - screen_center.x;
                    int y_diff = closest_center.y - screen_center.y;
                    // Move the mouse smoothly towards the target
                    mouse->move(magnet_smooth * x_diff, magnet_smooth * y_diff);

                    // Check if the target is within the trigger range (you can adjust the range values)
                    int trigger_range = 10;  // Example range value
                    if (std::abs(x_diff) <= trigger_range && std::abs(y_diff) <= trigger_range) {
                        mouse->click();  // Automatically fire if within range
                    }
                }
                prev_center = closest_center;  // Update the previous aim center
            }
        }
    }
};

#endif // COLORBOT_H
