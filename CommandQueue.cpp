#include "CommandQueue.h"
using namespace std;

void CommandQueue::addCommand(Command command) {
    lock_guard<mutex> lock(queueMutex);
    commands.push(std::move(command));
}

bool CommandQueue::hasCommand() const {
    lock_guard<mutex> lock(queueMutex);
    return !commands.empty();
}

Command CommandQueue::getNextCommand() {
    lock_guard<mutex> lock(queueMutex);
    Command cmd = commands.front();
    commands.pop();
    return cmd;
}
