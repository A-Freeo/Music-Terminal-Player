#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "TrackList.h"
#include <string>

class FileManager {
private:
    std::string fileName;

public:
    FileManager(const std::string& fileName);

    bool saveFile(const TrackList& track);
    bool loadFile(TrackList& track);
    bool fileExists();
    void setPlaylist(const std::string& fileName);
};

#endif
