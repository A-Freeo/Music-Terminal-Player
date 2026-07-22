#include "ConsoleUI.h"
#include <iostream>
using namespace std;

mutex consoleMutex;

namespace ConsoleUI {
    void displayPlaying(const Song* currentSong) {
        if (currentSong == nullptr) {
            lock_guard<mutex> lock(consoleMutex);
            cout << "No song loaded.\n";
            return;
        }

        lock_guard<mutex> lock(consoleMutex);
        int time = currentSong->getLength();
        int minutes = time / 60;
        int seconds = time % 60;

        cout << "\nNow playing: ";
        cout << currentSong->getArtist() << " - " << currentSong->getAlbum() << " - "
             << currentSong->getName() << " (" << minutes << ":"
             << (seconds > 9 ? "" : "0") << seconds << ")\n";
    }

    void displaySettings(bool paused, const AudioSettings& audioSettings,
                          bool isShuffled, RepeatMode repeatMode) {
        lock_guard<mutex> lock(consoleMutex);
        cout << (paused ? "|| Paused\n" : "> Playing\n");
        cout << "Volume: " << audioSettings.getVolume() << "/" << audioSettings.getMaxVolume() << "\n";

        switch (repeatMode) {
            case RepeatMode::Off: cout << "Repeat: Off\n"; break;
            case RepeatMode::One: cout << "Repeat: One\n"; break;
            case RepeatMode::All: cout << "Repeat: All\n"; break;
        }

        cout << "Shuffle: " << (isShuffled ? "On\n" : "Off\n");
    }

    void displayProgress(int start, int current) {
        double percentage = static_cast<double>(start - current) / start;
        constexpr int totalBars = 30;
        int barsFilled = static_cast<int>(percentage * totalBars);

        int elapsedTime = start - current;
        int elapsedMinutes = elapsedTime / 60;
        int elapsedSeconds = elapsedTime % 60;

        lock_guard<mutex> lock(consoleMutex);
        cout << "\r" << elapsedMinutes << ":" << (elapsedSeconds > 9 ? "" : "0") << elapsedSeconds << "  ";

        if (current == 0) barsFilled = totalBars - 1;
        for (int i = 0; i < totalBars; i++)
            cout << (i == barsFilled ? "*" : "-");

        int minutes = current / 60;
        int seconds = current % 60;
        cout << "  " << minutes << ":" << (seconds > 9 ? "" : "0") << seconds << " " << flush;
    }

    void displayAudioSettings(const AudioSettings& audioSettings) {
        lock_guard<mutex> lock(consoleMutex);
        for (int i = 10; i >= 1; i--) {
            cout << (audioSettings.getBass()   == i ? "[=]\t" : "||\t");
            cout << (audioSettings.getMids()   == i ? "[=]\t" : "||\t");
            cout << (audioSettings.getTreble() == i ? "[=]"   : "||\t");
            cout << "\n";
        }
        cout << "Bass\tMids\tTreble\t\n";
        cout << "Volume[";
        for (int i = 1; i <= audioSettings.getMaxVolume(); i++)
            cout << (i == audioSettings.getVolume() ? "[=]" : "=");
        cout << "]\n";
    }

    void displayTracks(const TrackList& trackList) {
        lock_guard<mutex> lock(consoleMutex);
        cout << "\n";
        for (size_t i = 0; i < trackList.size(); ++i) {
            Song* song = trackList.getSong(i);
            cout << "ID: " << song->getID() << " - " << song->getArtist() << " - " << song->getAlbum() << " - "
                 << song->getName() << " (" << song->getLength() << " seconds)\n";
        }
    }
}
