#include "PipeHandle.hpp"
#include <unistd.h>
#include <iostream>

namespace {
    void error(std::string content) {
        std::cerr << "[ERR] - " << content << "\n";
    }
}

PipeHandle::PipeHandle() {

}

PipeHandle::~PipeHandle() { 
    close_read_end();
    close_write_end();
};

bool PipeHandle::init() {
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

void PipeHandle::close_read_end() {
    if (get_read_end() == -1) {
        return;
    }

    if (close(filedes[0]) == -1) {
        error("Couldn't close read end");
        return;
    }
    filedes[0] = -1;
}

void PipeHandle::close_write_end() {
    if (get_write_end() == -1) {
        return;
    }

    if (close(filedes[1]) == -1) {
        error("Couldn't close write end");
        return;
    }
    filedes[1] = -1;

}
