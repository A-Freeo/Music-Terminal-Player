#ifndef SONG_H
#define SONG_H

#include <string>

class Song {
private:
    int id;
    std::string artist;
    std::string album;
    std::string name;
    int length;

public:
    Song(int id, const std::string& artist, const std::string& album, const std::string& name, int length);

    int getID() const;
    const std::string& getArtist() const;
    const std::string& getAlbum() const;
    const std::string& getName() const;
    int getLength() const;

    void setLength(int length);
};

#endif
