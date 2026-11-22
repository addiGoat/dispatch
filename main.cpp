#include <cstring>
#include <iostream>
#include <vector>
#include <spawn.h>
#include <unistd.h>
#include <sys/wait.h>

void error(std::string content) {
    std::cerr << "[ERR] - " << content << "\n";
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

std::vector<Action> GetExampleActions() {
    Action no_name;
    no_name.executable = "dolphin";

    return  {
        {
            "test",
            "notify-send",
            { "", "hello", "hi" }
        },
        no_name,
        {
            "terminal",
            "kitty"
        }
    };
}

void FormatActions(std::vector<Action> *actions) {
    std::vector<Action>::iterator it = actions->begin();
    while (it != actions->end()) {
        if (it->executable.empty()) {
            it = actions->erase(it);
        }

        if (it->name.empty()) {
            it->name = it->executable;
        }
        it++;
    }
}


// do this sometime numbnuts
class FileDescriptor {
public:
    FileDescriptor();

private:
        
};


class RofiPicker {
public:
    RofiPicker(const std::vector<Action>& actions) {
        for (const auto& action : actions) {
            rofi_action_names.push_back(action.name + "\n");
        }
    };

    void PrintActions() const {
        for (const auto& action : rofi_action_names) {
            std::cout << action;
        }
    }

    bool InitializeProcess() {

        // pipe[0] is the end you READ data FROM
        // pipe[1] is the end you WRITE data INTO
        int input[2], output[2];

        if (pipe(input) == -1) {
            error("Input pipe couldn't be created");
            return false;
        }

        if (pipe(output) == -1) {
            error("Output pipe couldn't be created");
            return false;
        }

        // need to add more robust error handling, instead of having to 
        // close all file descriptors manually every time
        if (posix_spawn_file_actions_init(&file_actions) != 0) {
            error("file actions initialization failed");
            return false;
        }

        if (posix_spawn_file_actions_adddup2(&file_actions, input[0], 0) != 0) {
            error("failed to duplicate input pipe");
            return false;
        }

        if (posix_spawn_file_actions_adddup2(&file_actions, output[1], 1) != 0) {
            error("failed to duplicate output pipe");
            return false;
        }


        std::vector<char*> cargs;
        for (auto& arg : args) {
            cargs.push_back(arg.data());
        }

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
        // then we assign the input to a non-array value,
        // for easy use later on
        close(input[0]);
        in_pipe = input[1];

        // it also does not need to write into 
        // the output pipe.
        close(output[1]);
        out_pipe = output[0];

        return true;
    }

private:
    pid_t pid = 0;
    posix_spawn_file_actions_t file_actions;
    std::vector<std::string> rofi_action_names;
    int in_pipe, out_pipe;

    inline static std::vector<std::string> args = {
        "-dmenu",
        "-format",
        "i",
        nullptr
    };

};

int main() {
    auto action_list = GetExampleActions();
    FormatActions(&action_list);

    RofiPicker picker(action_list);

    picker.PrintActions();
    picker.InitializeProcess();
    
    // for (const auto& action : action_list) {
    //     std::cout << "Name: " << action.name << ", Exec: " << action.executable << "\n";  
    // }
    std::vector<std::string> args = {
        "-dmenu",
        "-format",
        "i",
        nullptr
    };
    std::vector<char*> cargs;
    for (std::string& arg : args) {
        cargs.push_back(arg.data());
    }
    cargs.push_back(nullptr);
}
