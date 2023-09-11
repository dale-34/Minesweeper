#include "gameWindow.h"
#include "textures.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <random>
#include <cmath>
#include <sstream>
using namespace std;

Tiles::Tiles() {
    hidden.setTexture(TextureManager::GetTexture("tile_hidden"));
    reveal.setTexture(TextureManager::GetTexture("tile_revealed"));
    flag.setTexture(TextureManager::GetTexture("flag"));
    mine.setTexture(TextureManager::GetTexture("mine"));

    isFlagged = false;
    isRevealed = false;
    isHidden = true;
    isMine = false;

    for (int i = 0; i < 8; i++) {
        adjTiles.push_back(nullptr);
    }
}

void Tiles::displayTiles(sf::RenderWindow& window, float x, float y) {
    hidden.setPosition(x,y);
    reveal.setPosition(x,y);
    flag.setPosition(x,y);
    digit.setPosition(x,y);

    if(!isRevealed) {
        window.draw(hidden);
        if (isFlagged) {
            window.draw(flag);
        }
    }
    else {
        window.draw(reveal);
        window.draw(digit);
    }
}

Game::Game () {
    ifstream config("files/board_config.cfg");
    if (!config.is_open()) {
        cout << "Cannot open file." << endl;
    }
    int columns, rows, mines;
    config >> columns >> rows >> mines;
    this->rows = rows;
    this->columns = columns;
    this->minesAmount = mines;
    this->origMines = mines;
    this->width = columns * 32;
    this->height = (rows * 32) + 100;
    this->numMines = 0;
    this->timeAtPause = 0;
    this->pauseElapsed = 0;
    this->totalPauseTime = 0;
    this->getTime = 0;

    loseGame = false;
    winGame = false;
    pauseGame = false;
    debugMode = false;
    first_press = true;
    revealPress = true;
    facePressed = true;
    leaderPress = true;
    leaderPause = false;
    stopFunction = false;
    stopTime = false;
    hasWon = false;

    map<string, string> highestScore;
    board = vector<vector<Tiles>>(this->columns, vector<Tiles>(this->rows));
}

void Game::generateMines() {
    mt19937 random(time(0));
    while (numMines < minesAmount && numMines <= columns * rows) {
        uniform_int_distribution<int> distRows(0, rows - 1);
        uniform_int_distribution<int> distCols(0, columns - 1);
        int row = distRows(random);
        int col = distCols(random);
        if (!this->board[col][row].isMine) { // check if mine already exists at this location
            this->board[col][row].isMine = true; // mine is set
            numMines++;
        }
    }
}

void Game::checkAdjTiles() { // x is columns y is rows
    for (int i = 0; i < columns; i++) {
        for (int j = 0; j < rows; j++) {
            for (int k = 0; k < 8; k++) {
                board[i][j].adjTiles.push_back(nullptr);
            }
            if (i == 0 && j == 0) { // Top Left Corner
                board[i][j].adjTiles[4] = &board[i][j+1]; // right tile
                board[i][j].adjTiles[6] = &board[i+1][j]; // bottom tile
                board[i][j].adjTiles[7] = &board[i+1][j+1]; // right diagonal tile
            }
            else if (i == columns-1 && j == 0) { // Top Right Corner
                board[i][j].adjTiles[1] = &board[i-1][j]; // left tile
                board[i][j].adjTiles[2] = &board[i-1][j+1]; // left diagonal tile
                board[i][j].adjTiles[4] = &board[i][j+1]; // bottom tile
            }
            else if (i == 0 && j == rows-1) { // Lower Left Corner
                board[i][j].adjTiles[3] = &board[i][j-1]; // top tile
                board[i][j].adjTiles[5] = &board[i+1][j-1]; // right diagonal tile
                board[i][j].adjTiles[6] = &board[i+1][j]; // right tile
            }
            else if (i == columns-1 && j == rows-1) { // Lower Right Corner
                board[i][j].adjTiles[0] = &board[i-1][j-1]; // left diagonal tile
                board[i][j].adjTiles[1] = &board[i-1][j]; // top tile
                board[i][j].adjTiles[3] = &board[i][j-1]; // left tile
            }
            else if (i != 0 && i != columns-1 && j == 0) { // Top Tiles
                board[i][j].adjTiles[1] = &board[i-1][j]; // left tile
                board[i][j].adjTiles[2] = &board[i-1][j+1]; // right tile
                board[i][j].adjTiles[4] = &board[i][j+1]; // left diagonal tile
                board[i][j].adjTiles[6] = &board[i+1][j]; // bottom tile
                board[i][j].adjTiles[7] = &board[i+1][j+1]; // right diagonal tile
            }
            else if (i != columns - 1 && i != 0 && j == rows - 1) { // Bottom Tiles
                board[i][j].adjTiles[0] = &board[i-1][j-1]; // left diagonal tile
                board[i][j].adjTiles[1] = &board[i-1][j]; // top tile
                board[i][j].adjTiles[5] = &board[i+1][j-1]; // right diagonal tile
                board[i][j].adjTiles[6] = &board[i+1][j]; // left tile
                board[i][j].adjTiles[3] = &board[i][j-1]; // right tile
            }
            else if (i == 0 && j != 0 && j != rows - 1) { // Left Tiles
                board[i][j].adjTiles[3] = &board[i][j-1]; // top tile
                board[i][j].adjTiles[5] = &board[i+1][j-1]; // top right diagonal tile
                board[i][j].adjTiles[4] = &board[i][j+1]; // bottom tile
                board[i][j].adjTiles[6] = &board[i+1][j]; // right tile
                board[i][j].adjTiles[7] = &board[i+1][j+1]; // lower right diagonal tile
            }
            else if (i == columns - 1 && j != 0 && j != rows - 1) { // Right Tiles
                board[i][j].adjTiles[0] = &board[i-1][j-1]; // top left diagonal tile
                board[i][j].adjTiles[1] = &board[i-1][j]; // top tile
                board[i][j].adjTiles[2] = &board[i-1][j+1]; // left tile
                board[i][j].adjTiles[3] = &board[i][j-1]; // lower left diagonal tile
                board[i][j].adjTiles[4] = &board[i][j+1]; // bottom tile
            }
            else {
                board[i][j].adjTiles[0] = &board[i-1][j-1]; // top tile
                board[i][j].adjTiles[1] = &board[i-1][j]; // top middle tile
                board[i][j].adjTiles[2] = &board[i-1][j+1]; // top right tile
                board[i][j].adjTiles[3] = &board[i][j-1]; // mid left tile
                board[i][j].adjTiles[4] = &board[i][j+1]; // mid right tile
                board[i][j].adjTiles[5] = &board[i+1][j-1]; // bottom left tile
                board[i][j].adjTiles[6] = &board[i+1][j]; // bottom middle tile
                board[i][j].adjTiles[7] = &board[i+1][j+1]; // bottom right tile
            }
        }
    }
}

void Game::revealTiles(sf::RenderWindow& window, Tiles* tile) {
    if (!stopFunction) {
        if (tile->isMine) {
            loseGame = true;
        }
        if (!tile->isFlagged && !tile->isMine) {
            tile->isRevealed = true;
            tile->isHidden = false;
            int mineCount = 0;
            for (int i = 0; i < tile->adjTiles.size(); i++) {
                if (tile->adjTiles[i] != nullptr) {
                    if (tile->adjTiles[i]->isMine) {
                        mineCount++;
                    }
                }
            }
            for (int i = 0; i < tile->adjTiles.size(); i++) {
                if (tile->adjTiles[i] != nullptr) {
                    if (mineCount == 0 && tile->adjTiles[i]->isHidden) {
                        revealTiles(window, tile->adjTiles[i]);
                    }
                }
            }
            switch (mineCount) {
                case 1: tile->digit.setTexture(TextureManager::GetTexture("number_1")); break;
                case 2: tile->digit.setTexture(TextureManager::GetTexture("number_2")); break;
                case 3: tile->digit.setTexture(TextureManager::GetTexture("number_3")); break;
                case 4: tile->digit.setTexture(TextureManager::GetTexture("number_4")); break;
                case 5: tile->digit.setTexture(TextureManager::GetTexture("number_5")); break;
                case 6: tile->digit.setTexture(TextureManager::GetTexture("number_6")); break;
                case 7: tile->digit.setTexture(TextureManager::GetTexture("number_7")); break;
                case 8: tile->digit.setTexture(TextureManager::GetTexture("number_8")); break;
            }
        }
    }
}

void Game::displayLeaderBoard(vector<string> &scoresVec) {
    sf::Font font;
    font.loadFromFile("files/font.ttf");
    float leaderWidth = width / 2;
    float leaderHeight = height / 2;

    sf::Text boardTitle;
    boardTitle.setFont(font);
    boardTitle.setStyle(sf::Text::Bold | sf::Text::Underlined);
    boardTitle.setString("LEADERBOARD");
    boardTitle.setCharacterSize(20);
    boardTitle.setPosition(leaderWidth / 2.0f, leaderHeight / 2.0f - 120);
    sf::FloatRect textRect = boardTitle.getLocalBounds();
    boardTitle.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);


    sf::Text ranks;
    ranks.setFont(font);
    ranks.setStyle(sf::Text::Bold);
    ranks.setString(compareTimes(scoresVec));
    ranks.setCharacterSize(18);
    ranks.setPosition(leaderWidth/2.0f, leaderHeight/2.0f + 20);
    setText(ranks, leaderWidth/2.0f, leaderHeight/2.0f + 20);

    sf::Clock leaderboardClock;
    if (leaderPause) {
        leaderboardClock.restart();
        sf::RenderWindow leaderWindow(sf::VideoMode(16 * columns, (rows*16) + 50), "Minesweeper");
        while (leaderWindow.isOpen()) {
            sf::Event newEvent;
            while (leaderWindow.pollEvent(newEvent)) {
                if (newEvent.type == sf::Event::Closed) {
                    leaderPause = false;
                    stopFunction = false;
                    totalPauseTime += floor(leaderboardClock.getElapsedTime().asSeconds());
                    leaderWindow.close();
                }
                leaderWindow.clear(sf::Color::Blue);
                leaderWindow.draw(boardTitle);
                leaderWindow.draw(ranks);
                leaderWindow.display();
            }
        }
    }
}

void Game::displayLeaderButton(sf::RenderWindow &window, sf::Event event) {
    sf::Sprite leaderboard;
    leaderboard.setTexture(TextureManager::GetTexture("leaderboard"));
    leaderboard.setPosition((columns * 32) - 176, 32 * (rows + 0.5f));
    window.draw(leaderboard);

    // Checks if leaderboard button is pressed
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        if (leaderPress) {
            if (leaderboard.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                leaderPause = true;
                stopFunction = true;
            }
            leaderPress = false;
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased) {
        leaderPress = true;
    }
    if (leaderPause) {
        for (int i = 0; i < columns; i++) {
            for (int j = 0; j < rows; j++) {
                reveal.setPosition(i * 32, j * 32);
                window.draw(reveal);
            }
        }
    }
}

void Game::displayBoard(sf::RenderWindow &window, sf::Event event) {
    checkAdjTiles();
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2i position = sf::Mouse::getPosition(window);
        int x = position.x / 32;
        int y = position.y / 32;
        if (x >= 0 && x < columns && y >= 0 && y < rows) {
            // check in bounds of board
            if (position.x >= 0 && position.x <= width && position.y >= 0 && position.y <= height) {
                revealTiles(window, &board[x][y]);
            }
        }
    }
    if (event.type == event.MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
        sf::Vector2i position = sf::Mouse::getPosition(window);
        if (first_press) {
            int x = position.x / 32;
            int y = position.y / 32;
            if (x >= 0 && x < columns && y >= 0 && y < rows) {
                if (position.x >= 0 && position.x <= width && position.y >= 0 && position.y <= height &&
                    board[x][y].isHidden) {
                    board[x][y].isFlagged = !board[x][y].isFlagged;
                    if (!board[x][y].isFlagged) {
                        minesAmount++;
                    } else {
                        minesAmount--;
                    }
                }
            }
            first_press = false;
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased) {
        first_press = true;
    }
    for (int i = 0; i < columns; i++) {
        for (int j = 0; j < rows; j++) {
            if (winGame) {
                if (board[i][j].isMine) {
                    board[i][j].isFlagged = true;
                }
            }
            board[i][j].displayTiles(window, i * 32, j * 32);
        }
    }
}

void Game::displayDebug(sf::RenderWindow& window, sf::Event event) {
    sf::Sprite debug;
    debug.setTexture(TextureManager::GetTexture("debug"));
    debug.setPosition((columns * 32) - 304, 32 * (rows + 0.5f));
    window.draw(debug);
    if (!stopFunction && !winGame) {
        if (event.type == event.MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i position = sf::Mouse::getPosition(window);
            if (first_press) {
                if (debug.getGlobalBounds().contains(position.x, position.y)) {
                    debugMode = !debugMode;
                }
                first_press = false;
            }
        } else if (event.type == sf::Event::MouseButtonReleased) {
            first_press = true;
        }
        if (debugMode || loseGame) {
            for (int i = 0; i < columns; i++) {
                for (int j = 0; j < rows; j++) {
                    if (board[i][j].isMine) {
                        mine.setPosition(i * 32, j * 32);
                        window.draw(mine);
                    }
                }
            }
        }
    }
}

void Game::displayFace(sf::RenderWindow& window, sf::Event event) {
    sf::Sprite happySprite;
    happySprite.setTexture(TextureManager::GetTexture("face_happy"));
    happySprite.setPosition(((columns / 2.0) * 32) - 32, 32 * (rows + 0.5f));
    sf::Sprite winSprite;
    winSprite.setTexture(TextureManager::GetTexture("face_win"));
    winSprite.setPosition(((columns / 2.0) * 32) - 32, 32 * (rows + 0.5f));
    sf::Sprite sadSprite;
    sadSprite.setTexture(TextureManager::GetTexture("face_lose"));
    sadSprite.setPosition(((columns / 2.0) * 32) - 32, 32 * (rows + 0.5f));

    window.draw(happySprite);
    if (loseGame) {
        window.draw(sadSprite);
    }
    if (winGame) {
        window.draw(winSprite);
    }
    if(event.type == event.MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i position = sf::Mouse::getPosition(window);
        if(facePressed) {
            if (happySprite.getGlobalBounds().contains(position.x, position.y) ||
            winSprite.getGlobalBounds().contains(position.x, position.y) ||
            sadSprite.getGlobalBounds().contains(position.x, position.y)) {
                if (pauseGame) {
                    pauseGame = !pauseGame;
                }
                for (int i = 0; i < columns; i++) {
                    for (int j = 0; j < rows; j++) {
                        Restart(&board[i][j]);
                    }
                }
                numMines = 0;
                generateMines();
            }
        }
        facePressed = false;
    }
    else if(event.type == sf::Event::MouseButtonReleased) {
        facePressed = true;
    }
}

void Game::displayCounter(sf::RenderWindow &window) {
    sf::Sprite counterSprite;
    counterSprite.setTexture(TextureManager::GetTexture("digits"));
    int onesPlace = minesAmount % 10;
    int tensPlace = (minesAmount / 10) % 100;
    int hundredsPlace = minesAmount / 100;

    if (onesPlace < 0) {onesPlace *= -1;}
    if (tensPlace < 0) {tensPlace *= -1;}
    if (hundredsPlace < 0) {hundredsPlace *= -1;}
    if (minesAmount < 0) {
        sf::Sprite negSprite;
        negSprite.setTexture(TextureManager::GetTexture("digits"));
        negSprite.setTextureRect(sf::IntRect(210,0,21,32));
        negSprite.setPosition(12, 32 *((rows)+0.5f)+16);
        window.draw(negSprite);
    }
    if (winGame) {
        onesPlace = 0;
        tensPlace = 0;
        hundredsPlace = 0;
    }

    sf::Sprite hundredSprite;
    hundredSprite.setTexture(TextureManager::GetTexture("digits"));
    hundredSprite.setTextureRect(sf::IntRect(hundredsPlace * 21,0,21,32));
    hundredSprite.setPosition(33, 32 * ((rows)+0.5f)+16);

    sf::Sprite tenSprite;
    tenSprite.setTexture(TextureManager::GetTexture("digits"));
    tenSprite.setTextureRect(sf::IntRect(tensPlace * 21,0,21,32));
    tenSprite.setPosition(hundredSprite.getPosition().x + 21, hundredSprite.getPosition().y);

    sf::Sprite oneSprite;
    oneSprite.setTexture(TextureManager::GetTexture("digits"));
    oneSprite.setTextureRect(sf::IntRect(onesPlace * 21,0,21,32));
    oneSprite.setPosition(tenSprite.getPosition().x + 21, tenSprite.getPosition().y);

    window.draw(oneSprite);
    window.draw(tenSprite);
    window.draw(hundredSprite);
}

void Game::displayTimer(sf::RenderWindow &window) {
    sf::Sprite secondOne;
    sf::Sprite secondTwo;
    sf::Sprite minuteOne;
    sf::Sprite minuteTwo;
    sf::Time time = clock.getElapsedTime();
    sf::Time otherTime = clock2.getElapsedTime();

    int currentSeconds = floor(time.asSeconds());
    int otherSec = floor(otherTime.asSeconds());

    int onesPlace;
    int tensPlace;
    int minOnes;
    int minTens;
    if (pauseGame || leaderPause || winGame) {
        if (!stopTime) {
            clock2.restart();
            timeAtPause = currentSeconds - totalPauseTime;
            stopTime = true;
        }
        pauseElapsed = otherSec;
        onesPlace = timeAtPause % 10;
        tensPlace = (timeAtPause / 10) % 6;
        minOnes = (timeAtPause / 60) % 10;
        minTens = (timeAtPause / 360) % 100;
    }
    else {
        if (stopTime) {
        totalPauseTime += pauseElapsed;
        }
        stopTime = false;
        getTime = (currentSeconds - totalPauseTime);
        onesPlace = getTime % 10;
        tensPlace = ((getTime) / 10) % 6;
        minOnes = ((getTime) / 60) % 10;
        minTens = ((getTime) / 360) % 100;
    }
    secondOne.setTexture(TextureManager::GetTexture("digits"));
    secondTwo.setTexture(TextureManager::GetTexture("digits"));
    secondOne.setTextureRect(sf::IntRect(onesPlace*21,0,21,32));
    secondTwo.setTextureRect(sf::IntRect(tensPlace*21,0,21,32));

    minuteOne.setTexture(TextureManager::GetTexture("digits"));
    minuteTwo.setTexture(TextureManager::GetTexture("digits"));
    minuteOne.setTextureRect(sf::IntRect(minOnes*21,0,21,32));
    minuteTwo.setTextureRect(sf::IntRect(minTens*21,0,21,32));

    secondTwo.setPosition(((columns)*32)-54, 32*((rows)+0.5f)+16);
    secondOne.setPosition(secondTwo.getPosition().x + 21, secondTwo.getPosition().y);
    minuteTwo.setPosition(((columns)*32)-97, 32*((rows)+0.5f)+16);
    minuteOne.setPosition(minuteTwo.getPosition().x + 21, minuteTwo.getPosition().y);

    window.draw(secondOne);
    window.draw(secondTwo);
    window.draw(minuteOne);
    window.draw(minuteTwo);

}

void Game::displayPlayPause(sf::RenderWindow &window, sf::Event event) {
    sf::Sprite pause;
    pause.setTexture(TextureManager::GetTexture("pause"));
    pause.setPosition((columns * 32) - 240, 32 * (rows + 0.5f));
    window.draw(pause);

    if (!(winGame) && !loseGame) {
        if (event.type == event.MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i position = sf::Mouse::getPosition(window);
            if (revealPress) {
                if (pause.getGlobalBounds().contains(position.x, position.y)) {
                    pauseGame = !pauseGame;
                    stopFunction = !stopFunction;
                }
                revealPress = false;
            }
        } else if (event.type == sf::Event::MouseButtonReleased) {
            revealPress = true;
        }
        if (pauseGame) {
            sf::Sprite play;
            play.setTexture(TextureManager::GetTexture("play"));
            play.setPosition((columns * 32) - 240, 32 * (rows + 0.5f));
            window.draw(play);
            for (int i = 0; i < columns; i++) {
                for (int j = 0; j < rows; j++) {
                    reveal.setPosition(i * 32, j * 32);
                    window.draw(reveal);
                }
            }
        }
    }
}

void Game::Restart(Tiles* tile) {
    tile->digit.setTexture(TextureManager::GetTexture("tile_revealed"));
    tile->isMine = false;
    tile->isRevealed = false;
    tile->isHidden = true;
    tile->isFlagged = false;

    minesAmount = origMines;
    loseGame = false;
    winGame = false;
    hasWon = false;
    facePressed = true;
    clock.restart();
    stopTime = false;
    timeAtPause = 0;
    totalPauseTime = 0;
}

void Game::setText(sf::Text &text, float x, float y){
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
    text.setPosition(sf::Vector2f(x, y));
}

void Game::updateText(string name, ofstream &file, vector<string> &scoresVec) {
    int temp;
    if (winGame && !hasWon && stopTime) {
        temp = timeAtPause;
        stringstream ss;
        ss << setw(2) << setfill('0') << (temp / 60) << ":" << setw(2) << std::setfill('0') << (temp % 60);
        string formatted = ss.str() + "," + name;
        file << endl;
        file << formatted;

        for (int i = 0; i < scoresVec.size(); i++) { // removes asterisks from before
            int pos = scoresVec[i].find('*');
            if (pos != string::npos) {
                scoresVec[i].erase(pos, 1);
            }
        }
        scoresVec.push_back(formatted + "*");
        file.close();
        hasWon = true;
    }
}

void Game::checkWin() {
    int revealed = 0;
    for (int i = 0; i < columns; i++) {
        for (int j = 0; j < rows; j++) {
            if (board[i][j].isRevealed) {
                revealed += 1;
            }
        }
    }
    if ((rows*columns - revealed) == origMines) {
        winGame = true;
    }
}

string Game::compareTimes(vector<string> &scoresVec) {
    if (!scoresVec.empty()) {
        vector<string> sortedScores;
        sort(scoresVec.begin(), scoresVec.end(), [](const string& a, const string& b) {
            int minsA = stoi(a.substr(0, a.find(':')));
            int minsB = stoi(b.substr(0, b.find(':')));
            int secsA = stoi(a.substr(a.find(':') + 1));
            int secsB = stoi(b.substr(b.find(':') + 1));

            if (minsA != minsB) {
                return minsA < minsB;
            } else {
                return secsA < secsB;
            }
        });

        if (scoresVec.size() > 5) {
            scoresVec.pop_back();
        }
        for (int i = 0; i < scoresVec.size(); i++) {
            string score = scoresVec[i];
            string time = score.substr(0, score.find(','));
            string name = score.substr(score.find(',') + 1);
            string outputString = to_string(i+1) + "." + "\t" + time + "\t" + name;
            sortedScores.push_back(outputString);
        }
        string merged;
        for (const auto& score : sortedScores) {
            merged += score + "\n\n";
        }
        return merged;
    }
    else {
        return " ";
    }
}







