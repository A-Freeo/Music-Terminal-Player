#include "TrackList.h"
#include <algorithm>
#include <random>
using namespace std;

TrackList::TrackList() : shuffled(false) {}

void TrackList::rebuildQueueFromLibrary() {
    playbackQueue.clear();
    for (const auto& song : library) {
        playbackQueue.push_back(song.get());
    }
}

//-----------------------------
// Song Management
Song* TrackList::addSong(const string& artist, const string& album, const string& name, int length) {
    auto song = make_unique<Song>(++nextId, artist, album, name, length);
    Song* raw = song.get();
    library.push_back(std::move(song)); // transfers ownership
    playbackQueue.push_back(raw);
    return raw;
}

bool TrackList::removeSong(const int id) {
    auto it = find_if(library.begin(), library.end(),
                    [&](const unique_ptr<Song>& s) { return s->getID() == id; });

    if (it == library.end())
        return false;
    Song* target = it->get();
    library.erase(it);
    playbackQueue.erase(
        remove(playbackQueue.begin(), playbackQueue.end(), target),
        playbackQueue.end());

    return true;
}

Song* TrackList::searchSong(const int id) const {
    for (const auto& song : library)
        if (song->getID() == id)
            return song.get();
    return nullptr;
}

//-----------------------------

// Playback Queue / Shuffle
//-----------------------------
void TrackList::shuffle(Song* currentSong) {
    rebuildQueueFromLibrary();
    random_device rd;
    mt19937 gen(rd());
    std::shuffle(playbackQueue.begin(), playbackQueue.end(), gen);

    playbackQueue.erase(
        remove(playbackQueue.begin(), playbackQueue.end(), currentSong),
        playbackQueue.end());
    playbackQueue.insert(playbackQueue.begin(), currentSong);

    shuffled = true;
}

void TrackList::unshuffle() {
    rebuildQueueFromLibrary();
    shuffled = false;
}

bool TrackList::isShuffled() const { return shuffled; }

//-----------------------------

// Queue Access
//-----------------------------
Song* TrackList::getSong(size_t index) const { return playbackQueue[index]; }
Song* TrackList::getFirstSong() const { return playbackQueue.empty() ? nullptr : playbackQueue[0]; }
Song* TrackList::getLastSong() const { return playbackQueue.empty() ? nullptr : playbackQueue[library.size() - 1];}
size_t TrackList::size() const { return playbackQueue.size(); }

size_t TrackList::indexOf(Song* song) const {
    auto it = find(playbackQueue.begin(), playbackQueue.end(), song);
    if (it == playbackQueue.end()) return NOT_FOUND;
    return static_cast<size_t>(distance(playbackQueue.begin(), it));
}
