#ifndef UTILITIES_H
#define UTILITIES_H

class Player;
class TrackList;
class CommandQueue;

void clearProgressLine();
void refreshUI(Player& player, const TrackList& trackList);
void listenForInput(CommandQueue& handler, Player& player, bool& running);

#endif
