#include "RofiPicker.hpp"
#include <unistd.h>
namespace {
    void error(std::string content) {
        std::cerr << "[ERR] - " << content << "\n";
    }
}

RofiPicker::RofiPicker(const std::vector<Action>& actions) {
    for (const auto& action : actions) {
        rofi_action_names.push_back(action.name + "\n");
    }
};

void RofiPicker::PrintActions() const {
    for (const auto& action : rofi_action_names) {
        std::cout << action;
    }
}

bool RofiPicker::InitFileActions() {
    if (posix_spawn_file_actions_init(&file_actions) != 0) {
        error("file actions initialization failed");
        return false;
    }

    if (posix_spawn_file_actions_adddup2(
                &file_actions, 
                input_pipe.get_read_end(), 
                0) != 0) {
        error("failed to duplicate input pipe");
        return false;
    }

    if (posix_spawn_file_actions_adddup2(
                &file_actions, 
                output_pipe.get_write_end(), 
                1) != 0) {
        error("failed to duplicate output pipe");
        return false;
    }

    return true;
};

bool RofiPicker::InitializeProcess() {
    if (!input_pipe.init()) {
        error("Input pipe couldn't be created");
        return false;
    }

    if (!output_pipe.init()) {
        error("Output pipe couldn't be created");
        return false;
    }

    if (!InitFileActions())
        return false;



    std::vector<char*> cargs;
    for (auto& arg : args) {
        cargs.push_back(arg.data());
    }
    cargs.push_back(nullptr);

    posix_spawnp(
        &pid,
        "rofi",
        &file_actions,
        nullptr,
        cargs.data(),
        environ
    );


    // this parent process does not need to read
    // from the input pipe, so we can close it.
    input_pipe.close_read_end();

    // it also does not need to write into 
    // the output pipe.
    output_pipe.close_write_end();

    posix_spawn_file_actions_destroy(&file_actions);

    return true;
}
