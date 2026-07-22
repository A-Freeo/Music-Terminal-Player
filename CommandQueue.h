#ifndef COMMANDQUEUE_H
#define COMMANDQUEUE_H

#include <queue>
#include <mutex>
#include "Command.h"

class CommandQueue {
private:
    std::queue<Command> commands;
    mutable std::mutex queueMutex;

public:
    void addCommand(Command command);
    bool hasCommand() const;
    Command getNextCommand();
};

#endif
