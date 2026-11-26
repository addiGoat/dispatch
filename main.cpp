#include <cstddef>
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


class PipeHandle {
public:
    PipeHandle();

    ~PipeHandle() { 
        close_read_end();
        close_write_end();
    };

    PipeHandle(const PipeHandle&) = delete;
    PipeHandle& operator=(const PipeHandle&) = delete;

    void error(std::string content) {
        std::cerr << "[ERR] - " << content << "\n";
    }

    bool init() {
        if (get_write_end() != -1 || get_read_end() != -1) {
            error("Duplicate initialization, file descriptors already in use.");
            return false;
        }

        if (pipe(filedes) == -1) {
            error("Pipe creation failed");
            return false;
        }

        return true;
    }

    int get_read_end() const { return filedes[0]; }
    int get_write_end() const { return filedes[1]; }

    void close_read_end() {
        if (get_read_end() == -1) {
            return;
        }

        if (close(filedes[0]) == -1) {
            error("Couldn't close read end");
            return;
        }
        filedes[0] = -1;
    }

    void close_write_end() {
        if (get_write_end() == -1) {
            return;
        }

        if (close(filedes[1]) == -1) {
            error("Couldn't close write end");
            return;
        }
        filedes[1] = -1;
        
    }

private:
    int filedes[2] = {-1, -1};
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

    bool InitFileActions() {


        return true;
    };

    bool InitializeProcess() {
        if (!input_pipe.init()) {
            error("Input pipe couldn't be created");
            return false;
        }

        if (!output_pipe.init()) {
            error("Output pipe couldn't be created");
            return false;
        }

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

        return true;
    }

private:
    pid_t pid = 0;
    posix_spawn_file_actions_t file_actions;
    std::vector<std::string> rofi_action_names;
    PipeHandle input_pipe, output_pipe;

    inline static std::vector<std::string> args = {
        "-dmenu",
        "-format",
        "i"
    };
};

int main() {
    auto action_list = GetExampleActions();
    FormatActions(&action_list);

    RofiPicker picker(action_list);

    picker.PrintActions();
    picker.InitializeProcess();
}
