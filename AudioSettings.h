#ifndef AUDIOSETTINGS_H
#define AUDIOSETTINGS_H

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
    int getVolume() const;
    int getBass() const;
    int getMids() const;
    int getTreble() const;

    int getMaxVolume() const;

    void increaseVolume();
    void decreaseVolume();

    void setEQ(int bass, int mids, int treble);

    void resetEQ();
};

#endif
