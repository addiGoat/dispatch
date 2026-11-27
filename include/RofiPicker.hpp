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

private:
    std::vector<std::string> rofi_action_names;
    inline static std::vector<std::string> args = {
        "-dmenu",
        "-format",
        "i"
    };

    PipeHandle input_pipe, output_pipe;
    pid_t pid = 0;
    posix_spawn_file_actions_t file_actions;

    bool InitFileActions();
};
