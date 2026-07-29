//g++ -std=c++17 -pthread Song.cpp TrackList.cpp AudioSettings.cpp ConsoleUI.cpp Player.cpp CommandQueue.cpp Utilities.cpp main.cpp -o musicplayer
//./musicplayer
// maybe termios in the future
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include "Song.h"
#include "TrackList.h"
#include "AudioSettings.h"
#include "Player.h"
#include "ConsoleUI.h"
#include "Command.h"
#include "CommandQueue.h"
#include "Utilities.h"
using namespace std;

// g++ -std=c++17 -pthread Song.cpp AudioSettings.cpp TrackList.cpp ConsoleUI.cpp Player.cpp CommandQueue.cpp Utilities.cpp main.cpp -o MusicPlayer
int main() {
    CommandQueue commands;

    TrackList currentPlaylist;
    currentPlaylist.addSong("Radiohead", "OK Computer", "Paranoid Android", 383);
    currentPlaylist.addSong("Pink Floyd", "The Wall", "Comfortably Numb", 384);
    currentPlaylist.addSong("Nirvana", "Nevermind", "Lithium", 257);
    currentPlaylist.addSong("test", "test", "test1", 30);
    currentPlaylist.addSong("test", "test", "test2", 30);
    currentPlaylist.addSong("test", "test", "test3", 30);
    currentPlaylist.addSong("test", "test", "test4", 30);
    currentPlaylist.addSong("test", "test", "test5", 30);
    currentPlaylist.addSong("test", "test", "test6", 30);

    Player player(currentPlaylist);
    bool running = true;

     cout << "\n> - increase volume\n";
                        cout << "< - decrease volume\n";
                        cout << "e - view EQ settings\n";
                        cout << "E - adjust EQ settings\n";
                        cout << "s - skip\n";
                        cout << "p - pause/resume\n";
                        cout << "S - toggle shuffle\n";
                        cout << "q - quit\n";
                        cout << "l - list songs\n";
                        cout << "f - find song\n";
                        cout << "a - add song\n";
                        cout << "r - remove song\n";
                        cout << "R all - repeat all songs\n";
                        cout << "R one - repeat one song\n";
                        cout << "b - previous song \n";
                        cout << "h - help\n";


    thread inputThread(listenForInput, ref(commands), ref(player), ref(running));
    ConsoleUI::displayPlaying(player.getCurrentSong());
    ConsoleUI::displaySettings(player.isPaused(), player.getAudioSettings(), currentPlaylist.isShuffled(), player.getRepeatMode());
    while (running) {
        if (player.getCurrentSong() != nullptr && !player.isPaused()) {
            player.update();
        }else{
            this_thread::sleep_for(chrono::milliseconds(50));
        }
        while (commands.hasCommand()) {
            Command cmd = commands.getNextCommand();
            switch (cmd.type) {
                case CommandType::skip: {
                    clearProgressLine();
                    if (player.skip()) refreshUI(player, currentPlaylist);
                    else cout << "\nNo songs left... Add another song (a) or exit (q): " << flush;
                    break;
                }

                case CommandType::playPause: {
                    player.togglePause();
                    clearProgressLine();
                    cout << (player.isPaused() ? "\nPausing\n" : "\nResuming\n");
                    refreshUI(player, currentPlaylist);
                    if (player.isPaused() && player.getCurrentSong() != nullptr) {
                        ConsoleUI::displayProgress(player.getCurrentSong()->getLength(),
                                                    player.getCurrentSong()->getLength() - player.getSecondsPassed());
                    }
                    break;
                }

                case CommandType::previous:
                    clearProgressLine();
                    player.previous();
                    refreshUI(player, currentPlaylist);
                    break;

                case CommandType::addSong: {
                    clearProgressLine();
                    currentPlaylist.addSong(cmd.song.artist, cmd.song.album, cmd.song.songName, cmd.song.length);
                    if (player.getCurrentSong() == nullptr)
                        player.setCurrentSong(currentPlaylist.getLastSong());
                    refreshUI(player, currentPlaylist);
                    break;
                }

                case CommandType::removeSong: {
                    clearProgressLine();
                    Song* current = player.getCurrentSong();
                    bool removingCurrent = current != nullptr && current->getID() == cmd.song.id;

                    currentPlaylist.removeSong(cmd.song.id);
                    player.cleanupHistory(cmd.song.id);

                    if (removingCurrent) {
                        player.setCurrentSong(currentPlaylist.getFirstSong());
                    }
                    refreshUI(player, currentPlaylist);
                    break;
                }

                case CommandType::listSongs:
                    clearProgressLine();
                    ConsoleUI::displayTracks(currentPlaylist);
                    refreshUI(player, currentPlaylist);
                    break;

                case CommandType::searchSong: {
                    clearProgressLine();
                    Song* found = currentPlaylist.searchSong(cmd.song.id);
                    {
                        lock_guard<mutex> lock(consoleMutex);
                        cout << (found ? "Found!\n" : "Song not found.\n");
                    }
                    refreshUI(player, currentPlaylist);
                    break;
                }

                case CommandType::shuffle:
                    clearProgressLine();
                    if (currentPlaylist.isShuffled())
                        currentPlaylist.unshuffle();
                    else
                        currentPlaylist.shuffle(player.getCurrentSong());
                    refreshUI(player, currentPlaylist);
                    break;

                case CommandType::repeat:
                    clearProgressLine();
                    if (cmd.repeatMode == "all") player.setRepeatMode(RepeatMode::All);
                    else if (cmd.repeatMode == "one") player.setRepeatMode(RepeatMode::One);
                    else player.setRepeatMode(RepeatMode::Off);
                    refreshUI(player, currentPlaylist);
                    break;

                case CommandType::displayVolumeSettings:
                    clearProgressLine();
                    ConsoleUI::displayAudioSettings(player.getAudioSettings());
                    refreshUI(player, currentPlaylist);
                    break;

                case CommandType::increaseVolume:
                    clearProgressLine();
                    player.increaseVolume();
                    refreshUI(player, currentPlaylist);
                    break;

                case CommandType::decreaseVolume:
                    clearProgressLine();
                    player.decreaseVolume();
                    refreshUI(player, currentPlaylist);
                    break;

                case CommandType::setEQ:
                    clearProgressLine();
                    player.setEQ(cmd.bass, cmd.mids, cmd.treble);
                    refreshUI(player, currentPlaylist);
                    break;

                case CommandType::help: {
                    clearProgressLine();
                    {
                        lock_guard<mutex> lock(consoleMutex);
                        cout << "\n> - increase volume\n";
                        cout << "< - decrease volume\n";
                        cout << "e - view EQ settings\n";
                        cout << "E - adjust EQ settings\n";
                        cout << "s - skip\n";
                        cout << "p - pause/resume\n";
                        cout << "S - toggle shuffle\n";
                        cout << "q - quit\n";
                        cout << "l - list songs\n";
                        cout << "f - find song\n";
                        cout << "a - add song\n";
                        cout << "r - remove song\n";
                        cout << "R all - repeat all songs\n";
                        cout << "R one - repeat one song\n";
                        cout << "b - previous song \n";
                        cout << "h - help\n";
                    }
                    refreshUI(player, currentPlaylist);
                    break;
                }

                case CommandType::quit:
                    clearProgressLine();
                    cout << "\n";
                    running = false;
                    break;

                default:
                    break;
            }
        }
    }

    cout << "Quitting player...\n";
    inputThread.join();
    return 0;
}