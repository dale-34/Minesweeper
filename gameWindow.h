#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>
using namespace std;

class Tiles {
protected:
    sf::Sprite reveal, hidden, flag, mine;
public:
    Tiles();
    sf::Sprite digit;
    vector<Tiles*> adjTiles;
    bool isRevealed;
    bool isHidden;
    bool isMine;
    bool isFlagged;
    void displayTiles(sf::RenderWindow& window, float x, float y);
};

class Game: public Tiles {
private:
    int width;
    int height;
    int rows;
    int columns;
    int origMines;
    int minesAmount;
    int numMines;
    int timeAtPause;
    int pauseElapsed;
    int totalPauseTime;
    int getTime;

    bool loseGame;
    bool winGame;
    bool pauseGame;
    bool debugMode;
    bool facePressed;
    bool first_press;
    bool revealPress;
    bool leaderPress;
    bool leaderPause;
    bool stopFunction;
    bool stopTime;
    bool hasWon;

    vector<vector<Tiles>> board;
    map<string, string> highestScore;
    sf::Time pausedTime;
public:
    Game();
    sf::Clock clock;
    sf::Clock clock2;
    void generateMines();
    void displayFace(sf::RenderWindow& window, sf::Event event);
    void displayCounter(sf::RenderWindow& window);
    void displayTimer(sf::RenderWindow& window);
    void displayDebug(sf::RenderWindow& window, sf::Event event);
    void displayPlayPause(sf::RenderWindow& window, sf::Event event);
    void displayLeaderButton(sf::RenderWindow& window, sf::Event event);
    void displayLeaderBoard(vector<string> &scoresVec);
    void displayBoard(sf::RenderWindow& window, sf::Event event);
    void revealTiles(sf::RenderWindow& window, Tiles* tile);
    string compareTimes(vector<string> &scoresVec);
    void checkAdjTiles();
    void Restart(Tiles* tile);
    void setText(sf::Text &text, float x, float y);
    void updateText(string name, ofstream &file, vector<string> &scoresVec);
    void checkWin();
};


