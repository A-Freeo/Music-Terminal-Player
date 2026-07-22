#ifndef PLAYER_H
#define PLAYER_H

#include <stack>
#include "Song.h"
#include "AudioSettings.h"
#include "TrackList.h"
#include "Command.h"

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
    Player(TrackList& trackList);

    void update();
    bool skip();
    void previous();
    void cleanupHistory(int songId);

    void togglePause();
    bool isPaused() const;

    void startInput();
    void endInput();

    void setRepeatMode(RepeatMode mode);
    RepeatMode getRepeatMode() const;

    void increaseVolume();
    void decreaseVolume();
    void setEQ(int bass, int mids, int treble);

    Song* getCurrentSong() const;
    void setCurrentSong(Song* song);
    int getSecondsPassed() const;
    const AudioSettings& getAudioSettings() const;
};

#endif
