#ifndef __PLAYER_H
#define __PLAYER_H

#include "window.h"
#include "map.h"
#include "score.h"

class Player
{
    std::string playerName;
    /*
     * SelectionState[2] == false means that player did not select
     * the ship2 yet. The ship are numbered from 0 to 4, 0 being the smallest one.
     */
    bool selectionState[5];
     //It will keep track of the ship a player would like to select;
    int selectionPosition;

    //The map used after selection(in the main game). It knows where the ship are stored.    
    Map map;
    //Map used when placing a ship in the selection stage.
    Map tempMap; 
    //Map used ingame to hide the ships. It will lose one cell at every step.
    Map mask;

    //It keeps track whether the player is controlled by computer or not.
    bool computerControlled;

    /*
     * It keeps track of the score. The socre is written to the log file when finishing the game
     * and read when loading
     */
    //int score;    
    Score score;

    public:   
        Player();
        void init(int windowDim);
        bool validateTempMap(int windowDim);
        bool isSelectionReady();
        void generateMask();

        void setPlayerName(std::string playerName);
        void setSelectionState(int pos, bool value);
        void setSelectionPosition(int selectionPosition);
        void setMap(Map &map);
        void setTempMap(Map &tempMap);
        void setMask(Map &mask);
        void setComputerControlled(bool computerControlled); 
        void setScore(int value);       

        std::string getPlayerName();
        int getSelectionState(int pos);
        int getSelectionPosition();
        Map& getMap();
        Map& getTempMap();
        Map& getMask();
        bool getComputerControlled();
        int getScore();
};
#endif