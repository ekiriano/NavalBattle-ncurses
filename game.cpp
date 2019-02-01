#include "window.h"
#include "player.h"
#include "map.h"
#include "ship.h"
#include "menu.h"
#include "frameCell.h"
#include "move.h"
#include "score.h"
#include <time.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <string>
#include <sstream>
#include <algorithm>

/*
The gear of the game is this:

We have a FrameCell class which consists of a char and Color. Then we have the Map class 
which has a bidimensional matrix of cells. The main purpose of the maps are to 
code the ship's shape, and their position on the water. They also save the info
required for map colorising.

The Ship class has info about the shape (using the Map class above) and coordinates.

The Player class is the most important, as it makes use of 3 Maps.

The 3 maps are these:
map - which is the real map and hold info about the player's ship distribution
tempMap - used when we want to place a ship on the map and must check for bounds, etc
its content it copied on the map when all tests are passed.
mask - which tells us if a cell in the map has been revealed during the game
It is described in the class header how it works.

When the game starts, the info is read from the config file. 
Ship models are loaded, and then we enter the main menu.
We have options for:
    -start a new game
    -see records (last 5)
    -load a game
    -quit

When we start a new game, we enter the ship selection mode where each player
selects ships and adds them on the map. 
Then the game starts and we have alternate turns between the 2.
There is an option for undo, but it will affect the score.
The score is initially  =  (map dimension) ^ 2, and is decremented by 1 for each move.

We can quit the game at anytime, and the progress will be saved in the log file.
It can be loaded later.

We also have a Move class which is designed to keep track of every step in the game.
We have an array of Moves, where each Move holds info about the mobing player and his
decision in the game. It works like a stack.

IMPORTANT - I DESIGNED A MECHANISM TO MAKE FLAWLESS FRAME RENDERING.
Before writing to the screen, I compare the last frame with the current one
and update only the different cells to the background.

*/

class Game
{
    //Window n;
    Window *background = NULL;
    Window *battlefront = NULL;
    Window *enemyBattlefront = NULL;
    Window *shipSelector = NULL;
    Window *shipView = NULL;
    Window *currentWindow = NULL;    


    Menu menu;
    Player player[2], *currentPlayer;
    Ship ship[5]; Ship modelShip[5];
    //usefull when updating frames
    FrameCell frameCell[50][50], oldFrameCell[50][50];
    FrameCell enemyFrameCell[50][50], enemyOldFrameCell[50][50];

    //could have also used a stack, but i don't know if i am allowed..
    Move move[10000]; // the maximum number of moves would be 2 * windowDim^2

    char fullChar;
    bool selectionActive;
    int windowDim, selectTime, moveTime;
    int currentNumberOfMoves;


    void updateScreen();
    void initialSetup();
    void readPlayerData(Player *currentPlayer, std::string defaultName);
    void shipSelection(Player *currentPlayer); 
    void placePieceOnWindow(Player *currentPlayer); 
    void play(Player player[2], bool load); 
    void loadShipModels();
    void saveScore(std::string playerName, int newScore);
    void showScores();
    void clear();

    int checkGameEnd();
    int opponentID(int ID);
    int getMax(int a, int b);
    int getMin(int a, int b);
    int validateSelectionPosition(int pos); 

    bool isWholeShipRevealed(Map &map , Map &mask, int dim,int shipID);
    bool validatePieceInBounds(int x, int y, int dimX, int dimY);  

    public:        
        void startGame(bool showHelp);
        Game();    
        ~Game();          
};

void Game::startGame(bool showHelp)
{
    short int input; 
    initialSetup(); 

    if(showHelp)  {
        menu.CMDInfo(); 
        while((input = getch()) != 'c');
    }

    loadShipModels();  
    menu.CMDMainMenu();     
    updateScreen(); 

    while((input = getch()) != 'q') {   
        switch (input) {
            case 32:  
                
                player[0].init(windowDim); player[1].init(windowDim);                          
                readPlayerData(&player[0], "Player 1"); readPlayerData(&player[1], "Player2");
                for(int i=0; i<2; i++) {                                                              
                    currentPlayer = &player[i];
                    shipSelection(currentPlayer);
                    currentPlayer->generateMask();
                    updateScreen();  
                }  
                currentNumberOfMoves = 0; //first move has the ID equal to 0
                play(player, false); 
                clear();                    
                break;

            case 's':                 
                showScores();
                menu.advance();
                menu.CMDMainMenu();
                break;   
            case 'l':
                player[0].init(windowDim); player[1].init(windowDim);           
                updateScreen();player[0].generateMask();
                updateScreen(); player[1].generateMask();  
                updateScreen();          
                currentNumberOfMoves = 0;
                play(player, true);  
                clear();           
                break;        
        }
    }
}

void Game::clear() {
    selectionActive = false;
    menu.advance();
    battlefront->clear();
    enemyBattlefront->clear();
    menu.CMDMainMenu();        
}

void Game::play(Player player[2], bool load) {


    int x, y;
    int playerID = 1; //for currentPlayer's turn; can be 0 or 1.
    short int input;
    int lastValue;
    int score;
    bool ready;
    bool moved;
    bool undo = false;
    bool alreadySelected;
    bool playOption;
    std::ostringstream ss;
    std::string playerName;


    //loading the game if required
    if(load)
    {

        std::fstream f("log.txt", std::ios::in);
        int value;
        int pozX, pozY;

        f>>windowDim;

        for(int i=0; i<2; i++) {
            f>>playerName; f>> playOption ; f>>score;
            player[i].setPlayerName(playerName);
            player[i].setComputerControlled(playOption);
            player[i].setScore(score);
        }

        //reading initial map for each player
        for(int i=0; i<2; i++) {
            for(int x=0; x<windowDim; x++) {

                for(int y=0; y<windowDim; y++){
                    f>>value;
                    player[i].getMap().setCell(x, y, value);
                }                
            }
        }
        
        f>>currentNumberOfMoves;
        ss << currentNumberOfMoves;

        for(int i=0; i<currentNumberOfMoves; i++) {
            f >> playerID >> pozX >> pozY;
            player[opponentID(playerID)].getMask().setCell(pozX, pozY, -1);
            move[i].setMove(playerID, pozX, pozY);            
        }
        f.close();     
    }
    bool saveAndQuit = false;

    //MAIN GAME LOOP

    while(!checkGameEnd() && (saveAndQuit == false)) {

        //switching player's turn
        if(playerID == 1) {
            playerID = 0;       
        }
        else {
            playerID = 1; 
        }

        menu.cmdMessage(1, player[playerID].getPlayerName() +  " chose a cell from " +
            player[opponentID(playerID)].getPlayerName() +"'s battlefront! Good luck :p", true);  
        if(undo)
            menu.cmdMessage(0, "UNDID MOVE!! POINTS NOT RESTORED :P", false);
        
        x=0, y=0;
        if(!player[playerID].getComputerControlled()){
            lastValue = player[opponentID(playerID)].getMask().getCell(x, y);

            if(player[opponentID(playerID)].getMask().getCell(x, y) == -1)
                alreadySelected = true;
            else
                alreadySelected = false;
            player[opponentID(playerID)].getMask().setCell(x, y, 2);
        }

        updateScreen();

        while(1) {

            //if the player is computer controlled we randomly chose for him
            if(!player[playerID].getComputerControlled()) { 
                input = getch();
            }
            else  {
                do{
                    x = rand() % windowDim;;
                    y = rand() % windowDim;;
                }while(player[opponentID(playerID)].getMask().getCell(x, y) == -1);
                player[opponentID(playerID)].getMask().setCell(x, y, -1);
                usleep(moveTime);
                move[currentNumberOfMoves].setMove(playerID, x, y);
                player[playerID].setScore(player[playerID].getScore()-1);
                currentNumberOfMoves++;
                break;
            }

            //at each iteration of the loop, we will possibly make one of these actions

            ready = false;
            moved = false;
            undo = false;

            //they are treaded separately below

            if(!player[playerID].getComputerControlled()){
                switch(input) {
                    case KEY_LEFT:
                        
                        x--;
                        if(validatePieceInBounds(x, y, 1, 0)) {
                            x++;  player[opponentID(playerID)].getMask().setCell(x, y, lastValue);
                            x--;
                            lastValue = player[opponentID(playerID)].getMask().getCell(x, y);
                            moved = true;
                        }
                        else
                            x++;                   
                        break;
                    case KEY_RIGHT:
                        x++;
                        if(validatePieceInBounds(x, y, 1, 0)) {
                            x--;  player[opponentID(playerID)].getMask().setCell(x, y, lastValue);
                            x++;
                            lastValue = player[opponentID(playerID)].getMask().getCell(x, y);
                            moved = true;
                        }
                        else
                            x--;                   
                        break;

                    case KEY_UP:
                        y--;
                        if(validatePieceInBounds(x, y, 0, 1)) {
                            y++;  player[opponentID(playerID)].getMask().setCell(x, y, lastValue);
                            y--;
                            lastValue = player[opponentID(playerID)].getMask().getCell(x, y);
                            moved = true;
                        }
                        else
                            y++;                   
                        break;

                    case KEY_DOWN:                
                        y++;
                        if(validatePieceInBounds(x, y, 0, 1))  {
                            y--;  player[opponentID(playerID)].getMask().setCell(x, y, lastValue);
                            y++;
                            lastValue = player[opponentID(playerID)].getMask().getCell(x, y);
                            moved = true;
                        }
                        else
                            y--;                   
                        break;              
                    case 32:
                        ready = true;
                        break;
                    case 'u':                        
                        undo = true;
                        break;
                    case 'q':
                        menu.cmdMessage(1, "You chose to save and quit! Load the game later! CYA!", true);
                        saveAndQuit = true;
                        break;
                }
            }

            if(moved)  {
                if(player[opponentID(playerID)].getMask().getCell(x, y) == -1)
                    alreadySelected = true;
                else
                    alreadySelected = false;
                player[opponentID(playerID)].getMask().setCell(x, y, 2);
                updateScreen();
            }

            if(ready == true) {
                if(!alreadySelected) {
                    player[opponentID(playerID)].getMask().setCell(x, y, -1);
                    move[currentNumberOfMoves].setMove(playerID, x, y);
                    currentNumberOfMoves++;
                    player[playerID].setScore(player[playerID].getScore()-1);

                    if(isWholeShipRevealed( player[opponentID(playerID)].getMap(), 
                                           player[opponentID(playerID)].getMask(),
                                           windowDim, player[opponentID(playerID)].getMap().getCell(x, y)))
                    {
                        menu.CMDFoundShip(); 
                        while((input = getch()) != 'c');
                    }



                    break;
                }
                else
                {
                    menu.cmdMessage(1, "This cell is already selected!", true);
                }
            }

            if(undo == true)
            {

                int numberOfTimes = 1;
                if(player[opponentID(playerID)].getComputerControlled())
                    numberOfTimes = 2;

                
                player[playerID].setScore(player[playerID].getScore()-1);
                if(currentNumberOfMoves > 0)
                {               

                    player[0].generateMask();
                    player[1].generateMask();
                    currentNumberOfMoves-=numberOfTimes;
                    if(currentNumberOfMoves < 0) currentNumberOfMoves = 0;
                    for(int i=0; i<currentNumberOfMoves; i++)
                    {
                        player[opponentID(move[i].getPlayerTurn())].getMask().setCell(move[i].getX(), move[i].getY(), -1);                        

                    }
                    updateScreen();
                    break;
                }
                
            }

            if(saveAndQuit)
                break;
        }         
    }

    updateScreen();    
    int result = checkGameEnd();
    ss.str("");
    if(result == 1)
    {

        ss << player[1].getScore();
        menu.cmdMessage(0, "GAME END!",true);
        menu.cmdMessage(1, player[1].getPlayerName() + ", you WON. Your score is " + ss.str() +"!!", false);
        saveScore(player[1].getPlayerName(), player[1].getScore());

    }
    if(result == 2)
    {
        ss << player[0].getScore();
        menu.cmdMessage(0, "GAME END!",true);
        menu.cmdMessage(1, player[0].getPlayerName() + ", you WON. Your score is " + ss.str() +"!!", false);
        saveScore(player[0].getPlayerName(), player[0].getScore());
    }  

    //saving to log.txt;
    std::fstream f("log.txt", std::ios::out);


    f<<windowDim<<"\n";

    for(int i=0; i<2; i++)
    {
        
        f << player[i].getPlayerName() <<" ";
        f << player[i].getComputerControlled() <<" ";
        f << player[i].getScore() <<"\n";
    }
    //saving initial map for each player
    for(int i=0; i<2; i++)
    {
        for(int x=0; x<windowDim; x++)
        {
            for(int y=0; y<windowDim; y++)
            {
                f<<player[i].getMap().getCell(x, y)<<" ";
            }
            f<<"\n";
        }
    }

    f<<currentNumberOfMoves<<"\n";

    //saving list of moves
    for(int i=0; i<currentNumberOfMoves; i++)
    {
        f << move[i].getPlayerTurn() << " " <<
        move[i].getX() << " " <<move[i].getY() << "\n";
    }   

    f.close();      
}

void Game::loadShipModels() {

    //we code the info into each ship's modelMatrix

    int lines, cols;
    std::string line;
    std::fstream f("config.txt", std::ios::in);
    f>>windowDim;
    for(int i=0; i<5; i++) {
        f>>lines>>cols;
        ship[i].setDim(lines, cols);
        modelShip[i].setDim(lines, cols);
        for(int k=0; k<lines; k++) {            
            f>>line;
            for(int j=0; j<cols; j++) {
                ship[i].getModelMatrix().setCell(k, j, line[j]-'0');
                modelShip[i].getModelMatrix().setCell(k, j, line[j]-'0');
            }
        }
    }  
    f.close();
}

int Game::opponentID(int ID) {
    if(ID == 0)
        return 1;
    return 0;
}

int Game::checkGameEnd() {

    /*
     *It returns
     *0 - if game not finished yet
     *1 - if player 1 lost
     *2   if player 2 lost.
    */
    bool end[2];
    for(int k=0; k<=1; k++) {
        end[k] = true;
        for(int i=0; i<windowDim; i++)  {
            for(int j=0; j<windowDim; j++)
            {
                if((player[k].getMap().getCell(i, j) != -1) && (player[k].getMask().getCell(i, j) != -1))
                {
                    end[k] = false; break;
                }

            }
        }
        if(end[k])
            return k+1;
    }

    return 0;
}

void Game::initialSetup() {

    //We setup the windows. Their position is mostly dependent on the windowDim, provided in config

    //the caracter 32 is the space, which will provide us a fully colored cell.
    fullChar = 32;

    //Setting the ingame windows
    std::fstream f("config.txt", std::ios::in);
    f>>windowDim;
    f.close();
    //background->print(0,0, frameCell[5][19].getCh());
    
    background = new Window(45, 120, 0, 0);
    background->setCouleurFenetre(WBLACK);
    background->print(windowDim * 1.5, 0, "BATTLESHIPS", BRED);
    background->print(windowDim * 0.3, 1, "BATTLEGROUNDS", BRED);
    background->print(2 * windowDim + 16, 1, "SHIP SELECTOR", BRED);
    background->print(2 * windowDim + 16, 7, "SHIP MODEL VIEW", BRED);

    for(int i=3; i<=windowDim + 4; i++) {
        background->print(2 * windowDim + 13, i, fullChar, BRED);

    }
    
    battlefront = new Window(windowDim ,windowDim , 1 , 4);
    battlefront->setCouleurFenetre(WCYAN);
    battlefront->setCouleurBordure(BWHITE);   

    enemyBattlefront = new Window(windowDim,windowDim , windowDim+11 , 4);
    enemyBattlefront->setCouleurFenetre(WCYAN);
    enemyBattlefront->setCouleurBordure(BWHITE);

    shipSelector = new Window(1,5, 2 * windowDim + 21, 4);
    shipSelector->setCouleurFenetre(WCYAN);
    shipSelector->setCouleurBordure(BWHITE);

    shipView = new Window(5,5, 2 * windowDim + 21,10);
    shipView->setCouleurFenetre(WCYAN);
    shipView->setCouleurBordure(BWHITE);

    ship[0].setSize(2); ship[0].setColor(BMAGENTA);
    ship[1].setSize(3); ship[1].setColor(BBLUE);
    ship[2].setSize(3); ship[2].setColor(BYELLOW);
    ship[3].setSize(4); ship[3].setColor(BGREEN);
    ship[4].setSize(5); ship[4].setColor(BRED);     

    selectionActive = false;
    
    menu.init(getMax(19,6+windowDim+1));
}

void Game::updateScreen() {

    //We simply use the data provided by all the maps.

    shipSelector->clear();  

    for(int i = 0; i<2; i++){
        if(i==0) {
            currentWindow = battlefront;

        }
        else{
            currentWindow = enemyBattlefront;

        }

        for(int x=0; x<windowDim; x++) {
            for(int y=0; y<windowDim; y++) {
                if(player[i].getMap().getCell(x,y) != -1) {                  
                    
                    if(i==0) { 
                        frameCell[x][y].set(fullChar, ship[player[i].getMap().getCell(x, y)].getColor()); 
                    }
                    else {
                        enemyFrameCell[x][y].set(fullChar, ship[player[i].getMap().getCell(x, y)].getColor()); 
                    } 
                                   
                }
                else {
                    //currentWindow->print(x, y, fullChar, WCYAN);
                    if(i==0) { 
                        frameCell[x][y].set(fullChar, WCYAN); 
                    }
                    else {
                        enemyFrameCell[x][y].set(fullChar, WCYAN); 
                    } 
                }
            }
        }

        for(int x=0; x<windowDim; x++) {
            for(int y=0; y<windowDim; y++) {

                if(player[i].getTempMap().getCell(x,y) != -1) {
                    //currentWindow->print(x, y, fullChar, ship[player[i].getTempMap().getCell(x,y)].getColor());
                    if(i==0) { 
                        frameCell[x][y].set(fullChar, ship[player[i].getTempMap().getCell(x,y)].getColor()); 
                    }
                    else {
                        enemyFrameCell[x][y].set(fullChar, ship[player[i].getTempMap().getCell(x,y)].getColor()); 
                    } 

                }              
            }
        }

        for(int x=0; x<windowDim; x++) {
            for(int y=0; y<windowDim; y++) {

                if(player[i].getMask().getCell(x,y) == 0) {
                   //currentWindow->print(x, y, fullChar, BWHITE);
                   if(i==0) { 
                        frameCell[x][y].set(fullChar, BWHITE); 
                    }
                    else {
                        enemyFrameCell[x][y].set(fullChar, BWHITE); 
                    } 
                }       
                if(player[i].getMask().getCell(x,y) == 2) {
                   //currentWindow->print(x, y, 'X', BWHITE);
                    if(i==0) { 
                        frameCell[x][y].set('X', BWHITE); 
                    }
                    else {
                        enemyFrameCell[x][y].set('X', BWHITE); 
                    } 

                }        
            }
        }
    }

    for(int x=0; x<windowDim; x++){
        for(int y=0; y<windowDim; y++)  {
           if(( frameCell[x][y].getCh() != oldFrameCell[x][y].getCh() ) || (frameCell[x][y].getColor() != oldFrameCell[x][y].getColor()) )  {
               battlefront->print(x, y, frameCell[x][y].getCh(), frameCell[x][y].getColor());
           }                                
                
           oldFrameCell[x][y].set(frameCell[x][y].getCh(), frameCell[x][y].getColor());                         
        }

        for(int y=0; y<windowDim; y++) {
           if(( enemyFrameCell[x][y].getCh() != enemyOldFrameCell[x][y].getCh() ) || (enemyFrameCell[x][y].getColor() != enemyOldFrameCell[x][y].getColor()) ) {
               enemyBattlefront->print(x, y, enemyFrameCell[x][y].getCh(), enemyFrameCell[x][y].getColor());
           }                                
                
           enemyOldFrameCell[x][y].set(enemyFrameCell[x][y].getCh(), enemyFrameCell[x][y].getColor());                         
        }
    }


    if(selectionActive) {
        for(int i=0; i<5; i++)  {
            if(currentPlayer->getSelectionState(i) == false) {
                for(int j=0; j<ship[i].getSize(); j++)
                {
                    shipSelector->print(i, j, fullChar, ship[i].getColor());
                }
            }
        }
        shipSelector->print(currentPlayer->getSelectionPosition(), 0, 'X', ship[currentPlayer->getSelectionPosition()].getColor());


        shipView->clear();
        for(int i=0; i<5; i++){
            for(int j=0; j<5; j++) {
                if(modelShip[currentPlayer->getSelectionPosition()].getModelMatrix().getCell(i,j) == 1)
                {
                    shipView->print(j, i, fullChar, ship[currentPlayer->getSelectionPosition()].getColor());
                }
            }
        }
    }    
}

bool Game::isWholeShipRevealed(Map &map , Map &mask, int dim, int shipID) {

    //we find out by searching a piece of ship who is still unrevealed
    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++) {
            if(map.getCell(i, j) == shipID && mask.getCell(i, j) == 0)
                return false;
        }
    }
    return true;
}

void Game::shipSelection(Player *currentPlayer) {
    //We browse through the unselected ships. Next stage is placing the selected ship on to the map

    bool ready = false;
    short int input;  
    selectionActive = true;
    menu.CMDShipSelectMenu(currentPlayer->getPlayerName());
    updateScreen();

    if(!currentPlayer->getComputerControlled()) {

        while(ready != true) {            
            while((input = getch()))
            {  
               
                if(currentPlayer->isSelectionReady()) {
                    ready = true;
                    break;
                }
                switch (input) {
                    case KEY_LEFT:

                        currentPlayer->setSelectionPosition(currentPlayer->getSelectionPosition() -1 );
                        currentPlayer->setSelectionPosition(validateSelectionPosition(currentPlayer->getSelectionPosition())); 
                        menu.CMDShipSelectMenu(currentPlayer->getPlayerName());                  
                        updateScreen();          
                        break;
                    case KEY_RIGHT:
                        currentPlayer->setSelectionPosition(currentPlayer->getSelectionPosition() +1 );
                        currentPlayer->setSelectionPosition(validateSelectionPosition(currentPlayer->getSelectionPosition()));
                        menu.CMDShipSelectMenu(currentPlayer->getPlayerName());
                        updateScreen(); 
                        break;
                    case 32:

                        if(currentPlayer->getSelectionState(currentPlayer->getSelectionPosition()) == false) {
                            currentPlayer->setSelectionState(currentPlayer->getSelectionPosition(), true);                    
                            placePieceOnWindow(currentPlayer);
                            menu.CMDShipSelectMenu(currentPlayer->getPlayerName());
                            updateScreen();
                        }
                        break;
                    case 'c':
                        break;               
                }
            }      
        }
    }
    else
    {
        if(!currentPlayer->isSelectionReady()) {
            for(int i=0; i<5; i++) {
                currentPlayer->setSelectionPosition(i);
                currentPlayer->setSelectionState(i, true);
                placePieceOnWindow(currentPlayer);
            }
        }
    }    
}

int Game::validateSelectionPosition(int pos) {
    if(pos < 0)
        return 4;
    if(pos > 4)
        return 0;
    return pos;
}

void Game::placePieceOnWindow(Player *currentPlayer) {
    int x=0, y=0;
    bool ready = false;
    short int input;
    bool moved = true;
    int rotated;
    
    /* The 3 TESTS a ship needs to pass in order to be placed on the map are:
     * don't overlap with other ships (comparing the tempMap with the map)
     * dont cross the borders
     * do not rotate so that the ship crosses the borders ( for non square matrixModels)
     */

    if(!currentPlayer->getComputerControlled()) {
        while(ready == false){ 
            while((input = getch()) && !ready) {          
                
                if(moved){
           
                    currentPlayer->getTempMap().init(-1);
                                 
                    for(int i=0; i<5; i++) {
                        for(int j=0; j<5; j++) {
                            if(ship[currentPlayer->getSelectionPosition()].getModelMatrix().getCell(j,i) == 1) {
                                currentPlayer->getTempMap().setCell(x+i,y+j,currentPlayer->getSelectionPosition()); 
                            }
                        }
                    }                                        
                    updateScreen();
                } 
                moved = false;
                switch (input) {
                    case KEY_LEFT:
                        x--; 

                        if(validatePieceInBounds(x, y, ship[currentPlayer->getSelectionPosition()].getDimY(),
                            ship[currentPlayer->getSelectionPosition()].getDimX())) {
                            moved = true;
                        }    
                        else {
                            menu.cmdMessage(1, "Don't cross the borders, bro :c!", true);
                            x++;
                        }
                        break;
                    case KEY_RIGHT:
                        x++;
                        if(validatePieceInBounds(x, y, ship[currentPlayer->getSelectionPosition()].getDimY(),
                            ship[currentPlayer->getSelectionPosition()].getDimX())) {
                            moved = true;
                        }         
                        else{
                            menu.cmdMessage(1, "Don't cross the borders, bro :c!", true);
                            x--;
                        }
                        break;
                    case KEY_UP:
                        y--;
                        if(validatePieceInBounds(x, y, ship[currentPlayer->getSelectionPosition()].getDimY(),
                            ship[currentPlayer->getSelectionPosition()].getDimX())) {
                            moved = true;
                        }         
                        else{
                            menu.cmdMessage(1, "Don't cross the borders, bro :c!", true);
                            y++;
                        }
                        break;
                    case KEY_DOWN:
                        y++;
                        if(validatePieceInBounds(x, y, ship[currentPlayer->getSelectionPosition()].getDimY(),
                            ship[currentPlayer->getSelectionPosition()].getDimX())) {
                            moved = true;
                        }       
                        else{
                            menu.cmdMessage(1, "Don't cross the borders, bro :c!", true);
                            y--;
                        }
                        break;   

                    case 'r':
                        ship[currentPlayer->getSelectionPosition()].rotateRight();
                        if(validatePieceInBounds(x, y, ship[currentPlayer->getSelectionPosition()].getDimY(),
                            ship[currentPlayer->getSelectionPosition()].getDimX())) {
                            moved = true;
                        }
                        else {
                            ship[currentPlayer->getSelectionPosition()].rotateLeft();
                            menu.cmdMessage(1, "INVALID ROTATION! Don't cross the borders, bro :c!", true);}        
                        break;
                    case 'l':
                        ship[currentPlayer->getSelectionPosition()].rotateLeft();
                        if(validatePieceInBounds(x, y, ship[currentPlayer->getSelectionPosition()].getDimY(),
                            ship[currentPlayer->getSelectionPosition()].getDimX())) {
                            moved = true;
                        }
                        else {
                            ship[currentPlayer->getSelectionPosition()].rotateRight();
                            menu.cmdMessage(1, "INVALID ROTATION! Don't cross the borders, bro :c!", true);}        
                        break;             
                    case 32:    
                        ready = currentPlayer->validateTempMap(windowDim);
                        if(!ready) {menu.cmdMessage(1, "Do not overlap the ships, amigo :c!", true);}
                        break;
                }            
            }        
        }
    }

    else{
        while(ready == false)  {

            rotated = rand() % 4;
            for(int i=0; i<rotated; i++) {
                ship[currentPlayer->getSelectionPosition()].rotateLeft();
            }
            x = rand()%windowDim;;
            y = rand()%windowDim;;

            if(validatePieceInBounds(x, y, ship[currentPlayer->getSelectionPosition()].getDimX(), 
               ship[currentPlayer->getSelectionPosition()].getDimY() )) {

                currentPlayer->getTempMap().init(-1);                                 
                for(int i=0; i<5; i++){
                    for(int j=0; j<5; j++) {
                        if(ship[currentPlayer->getSelectionPosition()].getModelMatrix().getCell(i,j) == 1) {                            
                            currentPlayer->getTempMap().setCell(x+i,y+j,currentPlayer->getSelectionPosition());
                        }
                    }
                 }            

                ready = currentPlayer->validateTempMap(windowDim);
                usleep(selectTime);
            }
        }
        updateScreen();       
    }
}

bool Game::validatePieceInBounds(int x, int y, int dimX, int dimY) {

    if(x < 0 || x + dimX - 1 > windowDim-1 )
            return false;
    if(y < 0 || y + dimY -1 > windowDim-1)
            return false;
    return true;
}

void Game::saveScore(std::string playerName, int newScore) {  
    /* IDEA 
     * we read the scores already registred(which are maximum 5), 
     * and store them in the score array. Then we add the score given
     * as parameter. Then we sort the whole array and wrtie to file 
     * the best 5 
    */
    
    int scores[6];
    int score;
    int numberOfScores;
    std::string name[6];
    std::fstream f("score.txt", std::ios::in);
   
    f >> numberOfScores;
    for(int i=0; i<numberOfScores; i++)  {
        f>>scores[i];
        f>>name[i];
    }

    f.close();
    numberOfScores++;
    
    scores[numberOfScores-1] = newScore;
    name[numberOfScores-1] = playerName;


    f.open("score.txt", std::ios::out);
    f<<getMin(numberOfScores, 5)<<"\n";
    for(int i=0; i<getMin(numberOfScores, 5); i++) {
        f<<scores[i] << " "<<name[i] <<"\n";
    }
    f.close();

}
void Game::showScores()
{
    int numberOfScores;
    int score = -1;
    std::string name;
    std::string line;
    std::fstream f("score.txt", std::ios::in);    

    f>>numberOfScores;
        
    for(int i=0; i<numberOfScores; i++) {    
         std::ostringstream ss;
         f >> score >> name;        
         ss << score;
         line.append("|" + name + " " + ss.str() +"| ");      
    }

    if(numberOfScores) {
        menu.cmdMessage(0, "BEST SCORES", true);
        menu.cmdMessage(1, line, false);
    }
    else {
        menu.cmdMessage(0, "NO RECORDS!", true);
    }


      
    f.close();    
}

void Game::readPlayerData(Player *currentPlayer, std::string defaultName) {
    std::string name;
    menu.CMDAskForName(defaultName);
    menu.cmdMessage(1, "Your namecode is: " + name, false);
    short int ch;


    while( 1) {
        ch = getch();
        if( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') ) {
            name += ch;      
            menu.cmdMessage(1, "Your namecode is: " + name, false);
        }
        if( ch == KEY_BACKSPACE) {
            if(name.size() > 0) { 
                name.erase(name.end() - 1);
                menu.CMDAskForName(defaultName);
                menu.cmdMessage(1, "Your namecode is: " + name, false);
            }
        }
        if( ch == 32)
            break;
    }
    currentPlayer->setPlayerName(name);

    menu.cmdMessage(1, "Is your mind controlled by a computer, " + name + "? (Press y/n) :", false);
    while( 1) {
        ch = getch();
        if(ch == 'y' || ch == 'Y') {
            currentPlayer->setComputerControlled(true);
            break;
        }
        if(ch == 'n' || ch == 'N') {
            currentPlayer->setComputerControlled(false);
            break;
        }
    }
}


int Game::getMax(int a, int b) {
    return a > b ? a: b;
}
int Game::getMin(int a, int b) {
    return a < b ? a: b;
}


int main(int argc, char *argv[]){

    Game game;
    startProgramX();
    //if arguemnt is provided, we show game details before proceeding to the main menu.
    if(argc >= 2)
        game.startGame(true);
    else
        game.startGame(false);
    stopProgramX();    
    exit(0);
}
Game::~Game() {
    delete background;
    delete battlefront;
    delete enemyBattlefront;
    delete shipSelector;
}


//the frame system used to avoid latency
Game::Game(){
    windowDim = -1;
    srand(time(NULL));
    for(int i=0; i<50; i++) {
        for(int j=0; j<50; j++) {
            frameCell[i][j].setCh(0);
            oldFrameCell[i][j].setCh(0);
            enemyFrameCell[i][j].setCh(0); 
            enemyOldFrameCell[i][j].setCh(0);
            frameCell[i][j].setColor(BRED);
            oldFrameCell[i][j].setColor(BRED);
            enemyFrameCell[i][j].setColor(BRED); 
            enemyOldFrameCell[i][j].setColor(BRED);
        }
    }
    //delay values we use for computer , to avoid instant looking moves.
    selectTime = 200000;
    moveTime = 500000;
}
