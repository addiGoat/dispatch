#include <cstring>
#include <iostream>
#include <vector>
#include <spawn.h>
#include <unistd.h>
#include <sys/wait.h>

void error(std::string content) {
    std::cerr << "[ERR] -" << content << "\n";
}

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

int main() {

    std::vector<Action> action_list = {
        {
            "test",
            "notify-send",
            { "", "hello", "hi" }
        },
        {
            "",
            "dolphin"
        },
        {
            "terminal",
            "kitty"
        }
    };

    for (Action& action : action_list) {
        if (action.name.empty()) {
            action.name = action.executable;
        }
    }

    pid_t pid = 0;
    posix_spawn_file_actions_t rofi_actions;
    
    // pipe[0] is the end you READ data FROM
    // pipe[1] is the end you WRITE data INTO
    int input_pipe[2], output_pipe[2];

    if (pipe(input_pipe) == -1) {
        error("Input pipe couldn't be created");
        return 1;
    }

    if (pipe(output_pipe) == -1) {
        error("Output pipe couldn't be created");
        return 1;
    }

    posix_spawn_file_actions_init(&rofi_actions);
    posix_spawn_file_actions_adddup2(&rofi_actions, input_pipe[0], 0);
    posix_spawn_file_actions_adddup2(&rofi_actions, output_pipe[1], 1);

    // this parent process does not need to read
    // from the input pipe, so we can close it.
    // then we assign the input to a non-array value,
    // for easy use later on

    std::vector<std::string> rofi_args = {
        "-dmenu",
        "-format",
        "i"
    };


    std::vector<char*> rofi_cargs;
    for (std::string& arg : rofi_args) {
        rofi_cargs.push_back(arg.data());
    }
    rofi_cargs.push_back(nullptr);

    posix_spawnp(
            &pid,
            "rofi",
            &rofi_actions,
            nullptr,
            rofi_cargs.data(),
            environ
        );

    close(input_pipe[0]);
    auto input = input_pipe[1];

    // it also does not need to write into 
    // the output pipe.
    close(output_pipe[1]);
    auto output = output_pipe[0];

    for (const auto& action : action_list) {
        std::string write_string = action.name + "\n";
        write(input, write_string.c_str(), write_string.size());
    }
    close(input);


    int rofi_status;
    waitpid(pid, &rofi_status, 0);

    if (rofi_status != 0) {
        std::printf("Rofi exited with code %u", WEXITSTATUS(rofi_status));
        return 1;
    }

    constexpr int buffer_size = 20;
    char rofi_buffer[buffer_size];
    int current_bytes;
    int total_bytes = 0;

    current_bytes = read(output, rofi_buffer, buffer_size);

    if (rofi_buffer[0] == '\0') {
        error("returned rofi index is empty string");
        return 1;
    }

    auto action_index = atoi(rofi_buffer);
    Action chosen_action = action_list.at(action_index);

    std::vector<char*> aargs;
    for (std::string& arg : chosen_action.arguments) {
        aargs.push_back(arg.data());
    }
    aargs.push_back(nullptr);


    posix_spawnp(
        &pid,
        chosen_action.executable.c_str(),
        nullptr,
        nullptr,
        aargs.data(),
        environ
    );

}

