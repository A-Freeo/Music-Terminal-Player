#include "Player.h"
#include "ConsoleUI.h"
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

Player::Player(TrackList& trackList)
    : trackList(trackList),
      currentSong(trackList.getFirstSong()),
      secondsPassed(0), secondTracker(0),
      paused(false), collectingInput(false),
      repeatMode(RepeatMode::Off) {}

// Playback
//--------------------
void Player::update() {
    if (currentSong == nullptr) return;
    constexpr int MILISECONDS_PER_UPDATE = 100;
    this_thread::sleep_for(chrono::milliseconds(MILISECONDS_PER_UPDATE));
    secondTracker++;
    constexpr int UPDATES_PER_SECOND = 10;
    if (secondTracker == UPDATES_PER_SECOND) { secondTracker = 0; secondsPassed++; }

    if (secondsPassed >= currentSong->getLength()) {
        if (!collectingInput) ConsoleUI::displayProgress(currentSong->getLength(), 0);
        if (!skip()) return;

        if (!collectingInput) {
            cout << "\n";
            ConsoleUI::displayPlaying(currentSong);
            ConsoleUI::displaySettings(paused, audioSettings, trackList.isShuffled(), repeatMode);
        }
        return;
    }

    if (!collectingInput)
        ConsoleUI::displayProgress(currentSong->getLength(), currentSong->getLength() - secondsPassed);
}

bool Player::skip() {
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
        if (trackList.isShuffled()) trackList.shuffle(currentSong);
        return true;
    }

    currentSong = nullptr;
    return false;
}

void Player::previous() {
    if (!history.empty()) {
        currentSong = history.top();
        history.pop();
        secondsPassed = 0;
    }
}

void Player::cleanupHistory(const int songId) {
    stack<Song*> temp;
    while (!history.empty()) {
        if (history.top()->getID() != songId) {
            temp.push(history.top());
        }
        history.pop();
    }
    while (!temp.empty()) {
        history.push(temp.top());
        temp.pop();
    }
}

//--------------------

// State
//--------------------
void Player::togglePause() { paused = !paused; }
bool Player::isPaused() const { return paused; }

void Player::startInput() { collectingInput = true; }
void Player::endInput() { collectingInput = false; }

//--------------------

// Repeat
//--------------------
void Player::setRepeatMode(RepeatMode mode) { repeatMode = mode; }
RepeatMode Player::getRepeatMode() const { return repeatMode; }

//--------------------

// Audio
//--------------------
void Player::increaseVolume() { audioSettings.increaseVolume(); }
void Player::decreaseVolume() { audioSettings.decreaseVolume(); }
void Player::setEQ(int bass, int mids, int treble) { audioSettings.setEQ(bass, mids, treble); }

//--------------------

// Getters / Setters
//--------------------
Song* Player::getCurrentSong() const { return currentSong; }
void Player::setCurrentSong(Song* song) { currentSong = song; secondsPassed = 0; }
int Player::getSecondsPassed() const { return secondsPassed; }
const AudioSettings& Player::getAudioSettings() const { return audioSettings; }
