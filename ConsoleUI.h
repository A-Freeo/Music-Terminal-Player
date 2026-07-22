#ifndef CONSOLEUI_H
#define CONSOLEUI_H

#include <mutex>
#include "Song.h"
#include "AudioSettings.h"
#include "TrackList.h"
#include "Command.h"

extern std::mutex consoleMutex;

// ConsoleUI - Handles all console output, including the progress bar, song info, and settings display
namespace ConsoleUI {
    void displayPlaying(const Song* currentSong);
    void displaySettings(bool paused, const AudioSettings& audioSettings, bool isShuffled, RepeatMode repeatMode);
    void displayProgress(int start, int current);
    void displayAudioSettings(const AudioSettings& audioSettings);
    void displayTracks(const TrackList& trackList);
}

#endif
