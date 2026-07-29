#include "Utilities.h"
#include "Player.h"
#include "TrackList.h"
#include "CommandQueue.h"
#include "ConsoleUI.h"
#include <iostream>
#include <string>
#include <mutex>
using namespace std;

// Safely parse an int from user input. Returns false instead of throwing on
// empty/non-numeric/out-of-range input, so a bad entry can't crash the program.
static bool tryParseInt(const string& s, int& out) {
    try {
        out = stoi(s);
        return true;
    } catch (...) {
        return false;
    }
}

// Clears the current line in the console, used to refresh the progress bar
void clearProgressLine() {
    constexpr int AMOUNT_OF_CONSOLE_SPACES = 80;
    cout << "\r" << string(AMOUNT_OF_CONSOLE_SPACES, ' ') << "\r";
}

void refreshUI(Player& player, const TrackList& trackList) {
    ConsoleUI::displayPlaying(player.getCurrentSong());

    ConsoleUI::displaySettings(
        player.isPaused(),
        player.getAudioSettings(),
        trackList.isShuffled(),
        player.getRepeatMode());
}

// Input thread - Listens for user input and adds commands to the CommandQueue
void listenForInput(CommandQueue& handler, Player& player, bool& running) {
    string line;

    while (running && getline(cin, line)) {
        if (line.empty()) continue;

        // When the playlist has run out (no current song), only allow adding a
        // song or quitting — every other command has nothing to act on.
        if (player.getCurrentSong() == nullptr && line[0] != 'a' && line[0] != 'q') {
            lock_guard<mutex> lock(consoleMutex);
            cout << "\nNo songs left. Add another song (a) or exit (q): " << flush;
            continue;
        }

        Command cmd;

        switch (line[0]) {
            case 'p':
                cmd.type = CommandType::playPause;
                handler.addCommand(cmd);
                break;

            case 's':
                cmd.type = CommandType::skip;
                handler.addCommand(cmd);
                break;

            case 'b':
                cmd.type = CommandType::previous;
                handler.addCommand(cmd);
                break;

            case 'S':
                cmd.type = CommandType::shuffle;
                handler.addCommand(cmd);
                break;

            case 'q':
                cmd.type = CommandType::quit;
                handler.addCommand(cmd);
                return;

            case 'h':
                cmd.type = CommandType::help;
                handler.addCommand(cmd);
                break;

            case 'a': {
                player.startInput();
                cmd.type = CommandType::addSong;
                bool valid = true;
                {
                    lock_guard<mutex> lock(consoleMutex);
                    cout << "Artist: ";
                    getline(cin, cmd.song.artist);
                    cout << "Album: ";
                    getline(cin, cmd.song.album);
                    cout << "Song Name: ";
                    getline(cin, cmd.song.songName);
                    cout << "Length (seconds): ";
                    getline(cin, line);
                    if (!tryParseInt(line, cmd.song.length) || cmd.song.length <= 0) {
                        cout << "Invalid length. Song not added.\n";
                        valid = false;
                    }
                }
                player.endInput();
                if (valid) handler.addCommand(cmd);
                break;
            }

            case 'r': {
                player.startInput();
                cmd.type = CommandType::removeSong;
                bool valid = true;
                {
                    lock_guard<mutex> lock(consoleMutex);
                    cout << "Song ID: ";
                    getline(cin, line);
                    if (!tryParseInt(line, cmd.song.id)) {
                        cout << "Invalid ID.\n";
                        valid = false;
                    }
                }
                player.endInput();
                if (valid) handler.addCommand(cmd);
                break;
            }

            case 'l':
                cmd.type = CommandType::listSongs;
                handler.addCommand(cmd);
                break;

            case 'f': {
                player.startInput();
                cmd.type = CommandType::searchSong;
                bool valid = true;
                {
                    lock_guard<mutex> lock(consoleMutex);
                    cout << "Song ID: ";
                    getline(cin, line);
                    if (!tryParseInt(line, cmd.song.id)) {
                        cout << "Invalid ID.\n";
                        valid = false;
                    }
                }
                player.endInput();
                if (valid) handler.addCommand(cmd);
                break;
            }

            case 'R': {
                player.startInput();
                cmd.type = CommandType::repeat;
                {
                    lock_guard<mutex> lock(consoleMutex);
                    cout << "Repeat Mode (one/all/off): ";
                    getline(cin, cmd.repeatMode);
                }
                player.endInput();
                handler.addCommand(cmd);
                break;
            }

            case '>':
                cmd.type = CommandType::increaseVolume;
                cmd.volume = 1;
                handler.addCommand(cmd);
                break;

            case '<':
                cmd.type = CommandType::decreaseVolume;
                cmd.volume = -1;
                handler.addCommand(cmd);
                break;

            case 'E': {
                player.startInput();
                cmd.type = CommandType::setEQ;
                bool valid = true;
                {
                    lock_guard<mutex> lock(consoleMutex);
                    cout << "Bass (0-10): ";
                    getline(cin, line);
                    if (!tryParseInt(line, cmd.bass)) valid = false;
                    cout << "Mid Range (0-10): ";
                    getline(cin, line);
                    if (!tryParseInt(line, cmd.mids)) valid = false;
                    cout << "Treble (0-10): ";
                    getline(cin, line);
                    if (!tryParseInt(line, cmd.treble)) valid = false;
                    if (!valid) cout << "Invalid EQ value. Settings unchanged.\n";
                }
                player.endInput();
                if (valid) handler.addCommand(cmd);
                break;
            }

            case 'e':
                cmd.type = CommandType::displayVolumeSettings;
                handler.addCommand(cmd);
                break;

            default: {
                lock_guard<mutex> lock(consoleMutex);
                cout << "Unknown command.\n";
                break;
            }
        }
    }
}
