#include "FileManager.h"
#include "Song.h"
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

FileManager::FileManager(const string& fileName) : fileName(fileName + ".csv") {}

bool FileManager::fileExists() {
    ifstream file(fileName);
    file.close();
    return file.good();
}

bool FileManager::saveFile(const TrackList& track) {
    if (fileExists()) {
        return false;
    }

    ofstream outputFile(fileName);
    if (!outputFile) {
        return false;
    }

    int size = track.size();
    for (int i = 0; i < size; i++) {
        Song* song = track.getSong(i);
        const string& artist = song->getArtist();
        const string& album = song->getAlbum();
        const string& name = song->getName();
        int length = song->getLength();

        outputFile << artist << "," << album << "," << name << "," << length;
        if (i < size - 1) outputFile << "\n"; // no trailing newline after the last song
    }

    outputFile.close();

    if (outputFile.fail()) {
        return false;
    }

    return true;
}

bool FileManager::loadFile(TrackList& track) {
    if (!fileExists()) {
        return false;
    }

    ifstream inputFile(fileName);
    if (!track.clear()) {
        return false;
    }

    string line;
    while (getline(inputFile, line)) {
        stringstream ss(line);
        string artist, album, name, lengthStr;

        if (getline(ss, artist, ',') && getline(ss, album, ',') && getline(ss, name, ',')
            && getline(ss, lengthStr, ',')) {

            int length;
            try {
                length = stoi(lengthStr);
            } catch (...) {
                return false;
            }

            track.addSong(artist, album, name, length);
        } else {
            return false;
        }
    }
    inputFile.close();

    track.rebuildQueueFromLibrary();

    return true;
}

void FileManager::setPlaylist(const string& fileName) {
    this->fileName = fileName + ".csv"; // append ".csv" to match the constructor
}
