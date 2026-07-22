#ifndef COMMAND_H
#define COMMAND_H

#include <string>

enum class RepeatMode
{
    Off,
    One,
    All
};

// Commands - Defines the types of commands that can be used by the user, and the data associated with each command
enum class CommandType
{
    None,
    playPause,
    skip,
    previous,
    addSong,
    removeSong,
    listSongs,
    searchSong,
    shuffle,
    displayVolumeSettings,
    increaseVolume,
    decreaseVolume,
    setEQ,
    help,
    quit,
    repeat,
};

struct SongInfo {
    int id{};
    std::string artist;
    std::string album;
    std::string songName;
    int length{};
};

struct Command {
    CommandType type = CommandType::None;
    std::string songName;
    SongInfo song;
    int volume = 0;
    int bass = 5;
    int mids = 5;
    int treble = 5;
    std::string repeatMode;
};

#endif
