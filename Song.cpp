#include "Song.h"
using namespace std;

Song::Song(int id, const string& artist, const string& album, const string& name, int length)
    : id(id), artist(artist), album(album), name(name), length(length) {}

int Song::getID() const { return id; }
const string& Song::getArtist() const { return artist; }
const string& Song::getAlbum() const { return album; }
const string& Song::getName() const { return name; }
int Song::getLength() const { return length; }

void Song::setLength(int length) { this->length = length; }
