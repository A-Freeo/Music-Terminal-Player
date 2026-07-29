#include "Utilities.h"
#include "Player.h"
#include "TrackList.h"
#include "CommandQueue.h"
#include "ConsoleUI.h"
#include <iostream>
#include <string>
#include <mutex>
using namespace std;

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
                    cmd.song.length = stoi(line);
                }
                player.endInput();
                handler.addCommand(cmd);
                break;
            }

            case 'r': {
                player.startInput();
                cmd.type = CommandType::removeSong;
                {
                    lock_guard<mutex> lock(consoleMutex);
                    cout << "Song ID: ";
                    getline(cin, line);
                    cmd.song.id = stoi(line);
                }
                player.endInput();
                handler.addCommand(cmd);
                break;
            }

            case 'l':
                cmd.type = CommandType::listSongs;
                handler.addCommand(cmd);
                break;

            case 'f': {
                player.startInput();
                cmd.type = CommandType::searchSong;
                {
                    lock_guard<mutex> lock(consoleMutex);
                    cout << "Song ID: ";
                    getline(cin, line);
                    cmd.song.id = stoi(line);
                }
                player.endInput();
                handler.addCommand(cmd);
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
                {
                    lock_guard<mutex> lock(consoleMutex);
                    cout << "Bass (0-10): ";
                    getline(cin, line);
                    cmd.bass = stoi(line);
                    cout << "Mid Range (0-10): ";
                    getline(cin, line);
                    cmd.mids = stoi(line);
                    cout << "Treble (0-10): ";
                    getline(cin, line);
                    cmd.treble = stoi(line);
                }
                player.endInput();
                handler.addCommand(cmd);
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
