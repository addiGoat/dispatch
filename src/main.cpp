#include <unistd.h>
#include <vector>
#include <RofiPicker.hpp>

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


Action GetActionByIndex(const int index, const std::vector<Action>& actions) {
    return actions.at(index);
}

int main() {
    auto action_list = GetExampleActions();
    FormatActions(&action_list);

    RofiPicker picker(action_list);

    picker.PrintActions();

    if (!picker.InitializeProcess()) {
        std::cout << "Couldn't initialize rofi\n";
        return 1;
    }

    int action_index;

    // Todo: handle intentional exits differently from broken executions
    // for proper error reporting
    if ((action_index = picker.RunPicker()) == -1) {
        std::cout << "No action selected, exiting\n";
        return 1;
    }

    Action chosen_action = GetActionByIndex(action_index, action_list);

    std::vector<char*> args;
    for (std::string& arg : chosen_action.arguments) {
        args.push_back(arg.data());
    }
    args.push_back(nullptr);


    pid_t pid;
    posix_spawnp(
        &pid,
        chosen_action.executable.c_str(),
        nullptr,
        nullptr,
        args.data(),
        environ
    );

}
