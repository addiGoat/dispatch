#pragma once

#include <iostream>
#include <spawn.h>
#include <vector>
#include <string>
#include "PipeHandle.hpp"


struct Action {
    std::string name;
    std::string executable;
    std::vector<std::string> arguments;

    void Print() const {
        std::printf("Name: %s\nExecutable: %s\nArguments: ", name.c_str(), executable.c_str());
        for (const std::string& arg : arguments) {
            std::printf("%s,", arg.c_str());
        }
        std::cout << "\n";
    }
};

class RofiPicker {
public:
    RofiPicker(const std::vector<Action>& actions);

    void PrintActions() const;
    bool InitializeProcess();
    bool InitFileActions();

    // Returns the index of the chosen menu item
    // if no item is chosen, returns -1. This is
    // either because no choice was made, or rofi
    // couldn't open. in either case, exit immediately
    int RunPicker();

private:
    std::vector<std::string> action_names;

    PipeHandle input_pipe, output_pipe;
    pid_t pid = 0;
    posix_spawn_file_actions_t file_actions;

    const static int buffer_size = 10;
    char response_buffer[buffer_size];



    // Rofi Arguments
    static inline char rofi_exec[] = "rofi";
    static inline char mode[] = "-dmenu";
    static inline char flag[] = "-format";
    static inline char format[] = "i";
    static inline char* const args[] = {
        rofi_exec,
        mode,
        flag,
        format,
        nullptr
    }; 
};
