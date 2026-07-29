#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <vector>
#include <memory>
#include <string>
#include "Song.h"

// TrackList - Manages the collection of songs and the playback queue
// library — Main list
// playbackQueue — The order songs are actually played in.
class TrackList {
private:
    std::vector<std::unique_ptr<Song>> library;
    std::vector<Song*> playbackQueue;
    bool shuffled;
    int nextId = 0;

    void rebuildQueueFromLibrary();

public:
    TrackList();

    static constexpr size_t NOT_FOUND = static_cast<size_t>(-1);

    Song* addSong(const std::string& artist, const std::string& album, const std::string& name, int length);
    bool removeSong(int id);
    Song* searchSong(int id) const;

    void shuffle(Song* currentSong);
    void unshuffle();
    bool isShuffled() const;

    Song* getSong(size_t index) const;
    Song* getFirstSong() const;
    Song* getLastSong() const;
    size_t size() const;
    size_t indexOf(Song* song) const;
};

#endif
