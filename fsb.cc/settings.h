#ifndef SETTINGS_H
#define SETTINGS_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

class Settings {
private:
    std::string filename;
    std::map<std::string, std::map<std::string, std::string>> config;

    void load() {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open settings file: " + filename);
        }

        // Skip BOM if present
        if (file.peek() == 0xEF) {
            file.get(); file.get(); file.get();
        }

        std::string line;
        std::string section;
        while (getline(file, line)) {
            if (line.empty() || line[0] == ';') continue;

            if (line[0] == '[' && line.back() == ']') {
                section = line.substr(1, line.size() - 2);
                std::cout << "Current section: " << section << std::endl; // Debugging
                continue;
            }

            std::istringstream ss(line);
            std::string key, value;
            if (std::getline(ss, key, '=') && std::getline(ss, value)) {
                key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
                value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
                config[section][key] = value;
                std::cout << "Loaded: [" << section << "] " << key << " = " << value << std::endl; // Debugging
            }
            else {
                std::cerr << "Invalid key-value pair: " << line << std::endl;
            }
        }
    }

public:
    explicit Settings(const std::string& filename) : filename(filename) {
        try {
            load();
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    std::string get(const std::string& section, const std::string& key) const {
        try {
            return config.at(section).at(key);
        }
        catch (const std::out_of_range&) {
            std::cerr << "Key not found in section [" << section << "]: " << key << std::endl;
            return "0"; // Return default value if not found
        }
    }

    int get_int(const std::string& section, const std::string& key) const {
        try {
            return std::stoi(get(section, key));
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Invalid argument for get_int: [" << section << "] " << key << ". Error: " << e.what() << std::endl;
            return 0;
        }
        catch (const std::out_of_range& e) {
            std::cerr << "Value out of range for get_int: [" << section << "] " << key << ". Error: " << e.what() << std::endl;
            return 0;
        }
    }

    float get_float(const std::string& section, const std::string& key) const {
        try {
            return std::stof(get(section, key));
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Invalid argument for get_float: [" << section << "] " << key << ". Error: " << e.what() << std::endl;
            return 0.0f;
        }
        catch (const std::out_of_range& e) {
            std::cerr << "Value out of range for get_float: [" << section << "] " << key << ". Error: " << e.what() << std::endl;
            return 0.0f;
        }
    }

    bool get_boolean(const std::string& section, const std::string& key) const {
        std::string value = get(section, key);
        return value == "true" || value == "1";
    }

    std::vector<float> get_float_list(const std::string& section, const std::string& key) const {
        std::vector<float> values;
        std::string value_str = get(section, key);
        std::istringstream ss(value_str);
        std::string value;
        while (std::getline(ss, value, ',')) {
            try {
                values.push_back(std::stof(value));
            }
            catch (const std::invalid_argument& e) {
                std::cerr << "Invalid float value in list for [" << section << "] " << key << ": " << value << std::endl;
            }
            catch (const std::out_of_range& e) {
                std::cerr << "Value out of range in list for [" << section << "] " << key << ": " << value << std::endl;
            }
        }
        return values;
    }

    void save() const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open settings file for writing: " << filename << std::endl;
            return;
        }
        for (const auto& section : config) {
            file << "[" << section.first << "]" << std::endl;
            for (const auto& keyValue : section.second) {
                file << keyValue.first << " = " << keyValue.second << std::endl;
            }
            file << std::endl; // Empty line after each section
        }
    }

    void set(const std::string& section, const std::string& key, const std::string& value) {
        config[section][key] = value;
    }
};

#endif // SETTINGS_H
