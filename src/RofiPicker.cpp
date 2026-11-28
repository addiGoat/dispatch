#include "RofiPicker.hpp"
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

namespace {
    void error(std::string content) {
        std::cerr << "[ERR] - " << content << "\n";
    }
}

RofiPicker::RofiPicker(const std::vector<Action>& actions) {
    for (const auto& action : actions) {
        action_names.push_back(action.name + "\n");
    }
};

void RofiPicker::PrintActions() const {
    for (const auto& action : action_names) {
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
    
    return true;
}

int RofiPicker::RunPicker() {
    if (posix_spawnp(
        &pid,
        rofi_exec,
        &file_actions,
        nullptr,
        args,
        environ
    ) == -1) {
        error("Rofi couldn't open");
        return -1;
    }
    
    // Cleanup now that rofi is launched
    input_pipe.close_read_end();
    output_pipe.close_write_end();
    posix_spawn_file_actions_destroy(&file_actions);

    // Print the action list
    for (const auto& action : action_names) {
        write(input_pipe.get_write_end(), action.c_str(), action.size());
    }
    input_pipe.close_write_end();

    // Check rofi's exit status
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        error("waitpid failed");
        return -1;
    }

    if (!WIFEXITED(status)) {
        error("Rofi did not exit normally");
        return -1;
    }

    int rofi_status = WEXITSTATUS(status);

    if (rofi_status != 0) {
        error("Rofi errored");
        return -1;
    }
    
    // Read bytes from the output pipe if rofi didn't error
    read(output_pipe.get_read_end(), response_buffer, buffer_size);

    if (response_buffer[0] == '\0') {
        error("returned rofi index is an empty string");
        return -1;
    }

    return atoi(response_buffer);
}
