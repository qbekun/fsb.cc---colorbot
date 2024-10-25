#include <iostream>
#include <Windows.h>
#include <thread>
#include "Colorbot.h"
#include "Settings.h"

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ToggleConsoleWindow();
void ShowConsoleWindow(bool show);
void DisplayMessage(const std::string& message);

class Main {
public:
    Main() {
        try {
            settings = new Settings("settings.cfg"); // Create Settings with the filename
            // Get the screen size
            RECT desktop;
            GetWindowRect(GetDesktopWindow(), &desktop);
            monitor_width = desktop.right;
            monitor_height = desktop.bottom;
            center_x = monitor_width / 2;
            center_y = monitor_height / 2;
            try {
                x_fov = settings->get_int("Aimbot", "xFov");
                y_fov = settings->get_int("Aimbot", "yFov");
            }
            catch (const std::exception& e) {
                throw std::runtime_error("Failed to load FOV settings: " + std::string(e.what()));
            }
            colorbot = new Colorbot(center_x - x_fov / 2, center_y - y_fov / 2, x_fov, y_fov, settings); // Pass settings
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Initialization error: " + std::string(e.what()));
        }
    }
    ~Main() {
        delete settings; // Clean up
        delete colorbot; // Clean up
    }
    void run() {
        try {
            system("cls");
            SetConsoleTitle(L"fsb.cc");
            FreeConsole(); // Detach console window


            //hsv color view

            //std::thread(&Colorbot::show_hsv_window, colorbot).detach(); // Start the HSV window in a separate thread


            colorbot->listen(); // Call listen on the pointer
            while (true) {
                if (GetAsyncKeyState(VK_HOME) & 0x8000) { // Check if Home key is pressed
                    ToggleConsoleWindow(); // Toggle the console window visibility
                }
                // Keep the program running until manually closed
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simple wait loop
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error during run: " << e.what() << std::endl;
        }
    }
private:
    Settings* settings; // Change to pointer
    Colorbot* colorbot; // Change to pointer
    int monitor_width;
    int monitor_height;
    int center_x;
    int center_y;
    int x_fov;
    int y_fov;
};

int main() {
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Hidden Window Class";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // Create the window.
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Hidden Window",               // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,       // Parent window
        NULL,       // Menu
        GetModuleHandle(NULL), // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    // Hide the window.
    ShowWindow(hwnd, SW_HIDE);

    // Add the window to the system tray.
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1001;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy_s(nid.szTip, L"fsb.cc Hidden");
    Shell_NotifyIcon(NIM_ADD, &nid);

    // Register hotkey for Home key
    RegisterHotKey(hwnd, 1, MOD_NOREPEAT, VK_HOME);

    // Run the main loop.
    try {
        Main mainApp; // Ensure the class name matches 'Main'
        mainApp.run(); // Call the run method on the mainApp object
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

// Window procedure to handle messages
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_APP + 1:
        switch (lParam) {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_CONTEXTMENU:
            ToggleConsoleWindow();
            break;
        }
        break;
    case WM_HOTKEY:
        if (wParam == 1) {
            ToggleConsoleWindow();
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void DisplayMessage() {
    freopen("CONOUT$", "w", stdout); // Redirect stdout to console
    std::cout << R"( 
   ___         __                            
 /'___\       /\ \                           
/\ \__/   ____\ \ \____        ___     ___   
\ \ ,__\ /',__\\ \ '__`\      /'___\  /'___\ 
 \ \ \_//\__, `\\ \ \L\ \ __ /\ \__/ /\ \__/ 
  \ \_\ \/\____/ \ \_,__//\_\\ \____\\ \____\
   \/_/  \/___/   \/___/ \/_/ \/____/ \/____/
    )" << std::endl;

    std::cout << "                     - Enemy Outline Color: Purple" << std::endl;
}

// Function to show/hide the console window
void ToggleConsoleWindow() {
    static bool consoleVisible = false;
    consoleVisible = !consoleVisible;
    if (consoleVisible) {
        AllocConsole();
        ShowConsoleWindow(true);
        DisplayMessage();
    }
    else {
        FreeConsole();
    }
}

// Function to show/hide the console window
void ShowConsoleWindow(bool show) {
    HWND hwndConsole = GetConsoleWindow();
    if (hwndConsole) {
        ShowWindow(hwndConsole, show ? SW_SHOW : SW_HIDE);
        if (show) {
            SetForegroundWindow(hwndConsole);
        }
    }
}

// Function to display a message in the console
void DisplayMessage(const std::string& message) {
    freopen("CONOUT$", "w", stdout); // Redirect stdout to console
    std::cout << message << std::endl;
}
