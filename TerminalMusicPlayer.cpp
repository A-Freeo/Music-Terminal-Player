#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <stack>
#include <queue>
#include <mutex>
#include <chrono>
#include <thread>
#include <memory>

void clearProgressLine();
std::mutex consoleMutex;

enum class RepeatMode
{
    Off,
    One,
    All
};


// Song - Just holds song data
class Song {
private:
    int id;
    std::string artist;
    std::string album;
    std::string name;
    int length;

public:
    Song(int id, const std::string& artist, const std::string& album, const std::string& name, int length)
        : id(id), artist(artist), album(album), name(name), length(length) {}

    int getID() const { return id; }
    const std::string& getArtist() const { return artist; }
    const std::string& getAlbum()  const { return album; }
    const std::string& getName()   const { return name; }
    int getLength() const { return length; }

    void setLength(int length) { this->length = length; }
};

// AudioSettings - Holds volume and equalizer settings, functions to modify them, and constants for max values
class AudioSettings {
private:
    static constexpr int MAX_VOLUME = 10;
    static constexpr int MAX_EQ = 10;
    static constexpr int DEFAULT_VOLUME = 5;
    static constexpr int DEFAULT_EQ = 5;

    int volume = DEFAULT_VOLUME;
    int bass = DEFAULT_EQ;
    int mids = DEFAULT_EQ;
    int treble = DEFAULT_EQ;

public:
    int getVolume() const { return volume; }
    int getBass() const { return bass; }
    int getMids() const { return mids; }
    int getTreble() const { return treble; }
    int getMaxVolume() const { return MAX_VOLUME; }

    void increaseVolume() { if (volume < MAX_VOLUME) volume++; }
    void decreaseVolume() { if (volume > 0) volume--; }

    void setEQ(int bass, int mids, int treble) {
        if (bass > MAX_EQ || mids > MAX_EQ || treble > MAX_EQ) return;
        if (bass < 0 || mids < 0 || treble < 0) return;
        this->bass = bass;
        this->mids = mids;
        this->treble = treble;
    }

    void resetEQ() {
        bass = DEFAULT_EQ;
        mids = DEFAULT_EQ;
        treble = DEFAULT_EQ;
    }
};

// TrackList - Manages the collection of songs and the playback queue

// library — Main list
// playbackQueue — The order songs are actually played in.
class TrackList {
private:
    std::vector<std::unique_ptr<Song>> library;
    std::vector<Song*> playbackQueue;
    bool shuffled;
    int nextId = 0;

    void rebuildQueueFromLibrary() {
        playbackQueue.clear();
        for (const auto& song : library) {
            playbackQueue.push_back(song.get());
        }
    }

public:
    TrackList() : shuffled(false) {}


    //-----------------------------
    // Song Management
    Song* addSong(const std::string& artist, const std::string& album, const std::string& name, int length) {
        auto song = std::make_unique<Song>(nextId++, artist, album, name, length);
        Song* raw = song.get();
        library.push_back(std::move(song)); // transfers ownership
        playbackQueue.push_back(raw);
        return raw;
    }

    bool removeSong(const int id) {
        auto it = find_if(library.begin(), library.end(),
                        [&](const std::unique_ptr<Song>& s) { return s->getID() == id; });
    
        if (it == library.end())
            return false;
        Song* target = it->get(); 
        library.erase(it);
        playbackQueue.erase(
            remove(playbackQueue.begin(), playbackQueue.end(), target),
            playbackQueue.end());

        return true;
    }

    Song* searchSong(const int id) const {
        for (const auto& song : library)
            if (song->getID() == id)
                return song.get();
        return nullptr;
    }

    /*
    // maybe in ConsoleUI, but for now it's here
    void listSongs() const {
        cout << "\n";
        for (Song* song : library) {
            cout << song->getArtist() << " - " << song->getAlbum() << " - "
                 << song->getName() << " (" << song->getLength() << " seconds)\n";
        }
    }
    */

    //-----------------------------

    // Playback Queue / Shuffle
    //-----------------------------
    void shuffle(Song* currentSong) {
        rebuildQueueFromLibrary();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(playbackQueue.begin(), playbackQueue.end(), gen);

        playbackQueue.erase(
            remove(playbackQueue.begin(), playbackQueue.end(), currentSong),
            playbackQueue.end());
        playbackQueue.insert(playbackQueue.begin(), currentSong);

        shuffled = true;
    }

    void unshuffle() {
        rebuildQueueFromLibrary();
        shuffled = false;
    }

    bool isShuffled() const { return shuffled; }

    //-----------------------------

    // Queue Access
    //-----------------------------
    Song* getSong(size_t index) const { return playbackQueue[index]; }
    Song* getFirstSong() const { return playbackQueue.empty() ? nullptr : playbackQueue[0]; }
    size_t size() const { return playbackQueue.size(); }

    // THIS 
    static constexpr size_t NOT_FOUND = static_cast<size_t>(-1);
    size_t indexOf(Song* song) const {
        auto it = std::find(playbackQueue.begin(), playbackQueue.end(), song);
        if (it == playbackQueue.end()) return NOT_FOUND;
        return static_cast<size_t>(std::distance(playbackQueue.begin(), it));
    }
};


// ConsoleUI - Handles all console output, including the progress bar, song info, and settings display
namespace ConsoleUI {
    void displayPlaying(const Song* currentSong) {
        if (currentSong == nullptr) {
            std::lock_guard<std::mutex> lock(consoleMutex);
            std::cout << "No song loaded.\n";
            return;
        }

        std::lock_guard<std::mutex> lock(consoleMutex);
        int time = currentSong->getLength();
        int minutes = time / 60;
        int seconds = time % 60;

        std::cout << "\nNow playing: ";
        std::cout << currentSong->getArtist() << " - " << currentSong->getAlbum() << " - "
             << currentSong->getName() << " (" << minutes << ":"
             << (seconds > 9 ? "" : "0") << seconds << ")\n";
    }

    void displaySettings(bool paused, const AudioSettings& audioSettings,
                          bool isShuffled, RepeatMode repeatMode) {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << (paused ? "|| Paused\n" : "> Playing\n");
        std::cout << "Volume: " << audioSettings.getVolume() << "/" << audioSettings.getMaxVolume() << "\n";

        switch (repeatMode) {
            case RepeatMode::Off: std::cout << "Repeat: Off\n"; break;
            case RepeatMode::One: std::cout << "Repeat: One\n"; break;
            case RepeatMode::All: std::cout << "Repeat: All\n"; break;
        }

        std::cout << "Shuffle: " << (isShuffled ? "On\n" : "Off\n");
    }

    void displayProgress(int start, int current) {
        double percentage = static_cast<double>(start - current) / start;
        constexpr int totalBars = 30;
        int barsFilled = static_cast<int>(percentage * totalBars);

        int elapsedTime = start - current;
        int elapsedMinutes = elapsedTime / 60;
        int elapsedSeconds = elapsedTime % 60;

        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "\r" << elapsedMinutes << ":" << (elapsedSeconds > 9 ? "" : "0") << elapsedSeconds << "  ";

        if (current == 0) barsFilled = totalBars - 1;
        for (int i = 0; i < totalBars; i++)
            std::cout << (i == barsFilled ? "*" : "-");

        int minutes = current / 60;
        int seconds = current % 60;
        std::cout << "  " << minutes << ":" << (seconds > 9 ? "" : "0") << seconds << " " << std::flush;
    }

    void displayAudioSettings(const AudioSettings& audioSettings) {
        std::lock_guard<std::mutex> lock(consoleMutex);
        for (int i = 10; i >= 1; i--) {
            std::cout << (audioSettings.getBass()   == i ? "[=]\t" : "||\t");
            std::cout << (audioSettings.getMids()   == i ? "[=]\t" : "||\t");
            std::cout << (audioSettings.getTreble() == i ? "[=]"   : "||\t");
            std::cout << "\n";
        }
        std::cout << "Bass\tMids\tTreble\t\n";
        std::cout << "Volume[";
        for (int i = 1; i <= audioSettings.getMaxVolume(); i++)
            std::cout << (i == audioSettings.getVolume() ? "[=]" : "=");
        std::cout << "]\n";
    }

    void displayTracks(const TrackList& trackList){
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "\n";
        for (size_t i = 0; i < trackList.size(); ++i) {
            Song* song = trackList.getSong(i);
            std::cout << "ID: " << song->getID() << " - " << song->getArtist() << " - " << song->getAlbum() << " - "
                      << song->getName() << " (" << song->getLength() << " seconds)\n";
        }
    }
};


// Player - Manages playback, including the current song, history, audio settings, and repeat mode
class Player {
private:
    TrackList& trackList;
    Song* currentSong = nullptr;
    std::stack<Song*> history;

    AudioSettings audioSettings;

    int secondsPassed;
    int secondTracker;

    bool paused;
    bool collectingInput;
    RepeatMode repeatMode;

public:
    Player(TrackList& trackList)
        : trackList(trackList),
          currentSong(trackList.getFirstSong()),
          secondsPassed(0), secondTracker(0),
          paused(false), collectingInput(false),
          repeatMode(RepeatMode::Off) {}

    // Playback
    //--------------------
    void update() {
        if (currentSong == nullptr) return;
        constexpr int MILISECONDS_PER_UPDATE = 100;
        std::this_thread::sleep_for(std::chrono::milliseconds(MILISECONDS_PER_UPDATE));
        secondTracker++;
        constexpr int UPDATES_PER_SECOND = 10;
        if (secondTracker == UPDATES_PER_SECOND) { secondTracker = 0; secondsPassed++; }

        if (secondsPassed >= currentSong->getLength()) {
            if (!collectingInput) ConsoleUI::displayProgress(currentSong->getLength(), 0);
            if (!skip()) return;

            if (!collectingInput) {
                std::cout << "\n";
                ConsoleUI::displayPlaying(currentSong);
                ConsoleUI::displaySettings(paused, audioSettings, trackList.isShuffled(), repeatMode);
            }
            return;
        }

        if (!collectingInput)
            ConsoleUI::displayProgress(currentSong->getLength(), currentSong->getLength() - secondsPassed);
    }

    bool skip() {
        if (currentSong == nullptr) return false;

        if (repeatMode == RepeatMode::One) {
            secondsPassed = 0;
            return true;
        }

        size_t idx = trackList.indexOf(currentSong);
        if (idx == TrackList::NOT_FOUND) return false;

        if (idx + 1 < trackList.size()) {
            history.push(currentSong);
            currentSong = trackList.getSong(idx + 1);
            secondsPassed = 0;
            return true;
        }

        if (repeatMode == RepeatMode::All && trackList.size() > 0) {
            history.push(currentSong);
            currentSong = trackList.getSong(0);
            secondsPassed = 0;
            if(trackList.isShuffled()) trackList.shuffle(currentSong);
            return true;
        }

        currentSong = nullptr;
        return false;
    }

    void previous() {
        if (!history.empty()) {
            currentSong = history.top();
            history.pop();
            secondsPassed = 0;
        }
    }

    void cleanupHistory(const int songId) {
        std::stack<Song*> temp;
        while(!history.empty()){
            if(history.top()->getID() != songId){
                temp.push(history.top());
            }
            history.pop();
        }
        while(!temp.empty()){
            history.push(temp.top());
            temp.pop();
        }
    }

    //--------------------

    // State
    //--------------------
    void togglePause() { paused = !paused; }
    bool isPaused() const { return paused; }

    void startInput() { collectingInput = true; }
    void endInput() { collectingInput = false; }

    //--------------------

    // Repeat
    //--------------------
    void setRepeatMode(RepeatMode mode) { repeatMode = mode; }
    RepeatMode getRepeatMode() const { return repeatMode; }

    //--------------------

    // Audio
    //--------------------
    void increaseVolume() { audioSettings.increaseVolume(); }
    void decreaseVolume() { audioSettings.decreaseVolume(); }
    void setEQ(int bass, int mids, int treble) { audioSettings.setEQ(bass, mids, treble); }

    //--------------------

    // Getters / Setters
    //--------------------
    Song* getCurrentSong() const { return currentSong; }
    void setCurrentSong(Song* song) { currentSong = song; secondsPassed = 0; }
    int getSecondsPassed() const { return secondsPassed; }
    const AudioSettings& getAudioSettings() const { return audioSettings; }
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

class CommandQueue {
private:
    std::queue<Command> commands;
    mutable std::mutex queueMutex;

public:
    void addCommand(Command command) {
        std::lock_guard<std::mutex> lock(queueMutex);
        commands.push(std::move(command));
    }
    bool hasCommand() const {
        std::lock_guard<std::mutex> lock(queueMutex);
        return !commands.empty();
    }
    Command getNextCommand() {
        std::lock_guard<std::mutex> lock(queueMutex);
        Command cmd = commands.front();
        commands.pop();
        return cmd;
    }
};

void listenForInput(CommandQueue& handler, Player& player, bool& running);
void refreshUI(Player& player, const TrackList& trackList);

// main
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

    std::thread inputThread(listenForInput, std::ref(commands), std::ref(player), std::ref(running));
    ConsoleUI::displayPlaying(player.getCurrentSong());
    ConsoleUI::displaySettings(player.isPaused(), player.getAudioSettings(), currentPlaylist.isShuffled(), player.getRepeatMode());

    while (running) {
        if (player.getCurrentSong() != nullptr && !player.isPaused()) {
            player.update();
        }
        while (commands.hasCommand()) {
            Command cmd = commands.getNextCommand();
            switch (cmd.type) {
                case CommandType::skip: {
                    clearProgressLine();
                    if (player.skip()) refreshUI(player, currentPlaylist);
                    else std::cout << "\nNo songs left.\n";
                    break;
                }

                case CommandType::playPause: {
                    player.togglePause();
                    clearProgressLine();
                    std::cout << (player.isPaused() ? "\nPausing\n" : "\nResuming\n");
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
                        player.setCurrentSong(currentPlaylist.getFirstSong());
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
                        std::lock_guard<std::mutex> lock(consoleMutex);
                        std::cout << (found ? "Found!\n" : "Song not found.\n");
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
                        std::lock_guard<std::mutex> lock(consoleMutex);
                        std::cout << "\n";
                        std::cout << "> - increase volume\n";
                        std::cout << "< - decrease volume\n";
                        std::cout << "e - view EQ settings\n";
                        std::cout << "E - adjust EQ settings\n";
                        std::cout << "s - skip\n";
                        std::cout << "p - pause/resume\n";
                        std::cout << "S - toggle shuffle\n";
                        std::cout << "q - quit\n";
                        std::cout << "l - list songs\n";
                        std::cout << "f - find song\n";
                        std::cout << "a - add song\n";
                        std::cout << "r - remove song\n";
                        std::cout << "R all - repeat all songs\n";
                        std::cout << "R one - repeat one song\n";
                        std::cout << "b - previous song \n";
                        std::cout << "h - help\n";
                    }
                    refreshUI(player, currentPlaylist);
                    break;
                }

                case CommandType::quit:
                    clearProgressLine();
                    std::cout << "\n";
                    running = false;
                    break;

                default:
                    break;
            }
        }
    }

    std::cout << "Quitting player...\n";
    inputThread.join();
    return 0;
}


// Input thread - Listens for user input and adds commands to the CommandQueue
void listenForInput(CommandQueue& handler, Player& player, bool& running) {
    std::string line;

    while (running && std::getline(std::cin, line)) {
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
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    std::cout << "ID: ";
                    std::getline(std::cin, line);
                    cmd.song.id = std::stoi(line);
                    std::cout << "Artist: ";
                    std::getline(std::cin, cmd.song.artist);
                    std::cout << "Album: ";
                    std::getline(std::cin, cmd.song.album);
                    std::cout << "Song Name: ";
                    std::getline(std::cin, cmd.song.songName);
                    std::cout << "Length (seconds): ";
                    std::getline(std::cin, line);
                    cmd.song.length = std::stoi(line);
                }
                player.endInput();
                handler.addCommand(cmd);
                break;
            }

            case 'r': {
                player.startInput();
                cmd.type = CommandType::removeSong;
                {
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    std::cout << "Song ID: ";
                    std::getline(std::cin, line);
                    cmd.song.id = std::stoi(line);
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
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    std::cout << "Song ID: ";
                    std::getline(std::cin, line);
                    cmd.song.id = std::stoi(line);
                }
                player.endInput();
                handler.addCommand(cmd);
                break;
            }

            case 'R': {
                player.startInput();
                cmd.type = CommandType::repeat;
                {
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    std::cout << "Repeat Mode (one/all/off): ";
                    std::getline(std::cin, cmd.repeatMode);
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
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    std::cout << "Bass (0-10): ";
                    std::getline(std::cin, line);
                    cmd.bass = std::stoi(line);
                    std::cout << "Mid Range (0-10): ";
                    std::getline(std::cin, line);
                    cmd.mids = std::stoi(line);
                    std::cout << "Treble (0-10): ";
                    std::getline(std::cin, line);
                    cmd.treble = std::stoi(line);
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
                std::lock_guard<std::mutex> lock(consoleMutex);
                std::cout << "Unknown command.\n";
                break;
            }
        }
    }
}

// Clears the current line in the console, used to refresh the progress bar
void clearProgressLine() {
    constexpr int AMOUNT_OF_CONSOLE_SPACES = 80;
    std::cout << "\r" << std::string(AMOUNT_OF_CONSOLE_SPACES, ' ') << "\r";
}

void refreshUI(Player& player, const TrackList& trackList)
{
    ConsoleUI::displayPlaying(player.getCurrentSong());

    ConsoleUI::displaySettings(
        player.isPaused(),
        player.getAudioSettings(),
        trackList.isShuffled(),
        player.getRepeatMode());
}