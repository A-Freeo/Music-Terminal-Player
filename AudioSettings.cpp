#include "AudioSettings.h"

int AudioSettings::getVolume() const { return volume; }
int AudioSettings::getBass() const { return bass; }
int AudioSettings::getMids() const { return mids; }
int AudioSettings::getTreble() const { return treble; }

int AudioSettings::getMaxVolume() const { return MAX_VOLUME; }

void AudioSettings::increaseVolume() { if (volume < MAX_VOLUME) volume++; }
void AudioSettings::decreaseVolume() { if (volume > 0) volume--; }

void AudioSettings::setEQ(int bass, int mids, int treble) {
    if (bass > MAX_EQ || mids > MAX_EQ || treble > MAX_EQ) return;
    if (bass < 0 || mids < 0 || treble < 0) return;
    this->bass = bass;
    this->mids = mids;
    this->treble = treble;
}

void AudioSettings::resetEQ() {
    bass = DEFAULT_EQ;
    mids = DEFAULT_EQ;
    treble = DEFAULT_EQ;
}
