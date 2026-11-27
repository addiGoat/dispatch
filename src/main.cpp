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

int main() {
    auto action_list = GetExampleActions();
    FormatActions(&action_list);

    RofiPicker picker(action_list);

    picker.PrintActions();
    picker.InitializeProcess();
    picker.RunPicker();
}
