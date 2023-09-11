#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "gameWindow.h"
#include "textures.h"
using namespace std;

int main()
{
    // Reading files
    ifstream config("files/board_config.cfg");
    if (!config.is_open()) {
        cout << "Cannot open file." << endl;
    }

    int columns, rows, mines;
    config >> columns >> rows >> mines;

    int width = columns * 32;
    int height = (rows * 32) + 100;

    sf::RenderWindow welcomeWindow(sf::VideoMode(width, height), "Minesweeper");

    sf::Font font;
    font.loadFromFile("files/font.ttf");

    // Welcome Screen
    sf::Text welcomeText;
    welcomeText.setFont(font);
    welcomeText.setStyle(sf::Text::Bold | sf::Text::Underlined);
    welcomeText.setString("WELCOME TO MINESWEEPER!");
    welcomeText.setCharacterSize(24);
    welcomeText.setPosition(width / 2.0f, height / 2.0f - 150);
    welcomeText.setFillColor(sf::Color::White);

    // Enter Name Prompt
    sf::Text namePrompt;
    namePrompt.setFont(font);
    namePrompt.setStyle(sf::Text::Bold);
    namePrompt.setString("Enter your name: ");
    namePrompt.setCharacterSize(20);
    namePrompt.setPosition(width / 2.0f, height / 2.0f - 75);
    namePrompt.setFillColor(sf::Color::White);

    // User Input
    sf::Text userInput;
    userInput.setFont(font);
    userInput.setStyle(sf::Text::Bold);
    userInput.setCharacterSize(18);
    userInput.setPosition(width / 2.0f, height / 2.0f - 45);
    userInput.setFillColor(sf::Color::Yellow);

    // Center text
    sf::FloatRect textRect = welcomeText.getLocalBounds();
    welcomeText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top  + textRect.height/2.0f);
    sf::FloatRect nameRect = namePrompt.getLocalBounds();
    namePrompt.setOrigin(nameRect.left + nameRect.width/2.0f, nameRect.top  + nameRect.height/2.0f);

    string name = "";
    bool finishName = false;
    userInput.setString("|"); // initial cursor
    sf::FloatRect inputRect = userInput.getLocalBounds();
    userInput.setOrigin(inputRect.left + inputRect.width/2.0f, inputRect.top  + inputRect.height/2.0f);

    while (welcomeWindow.isOpen() && !finishName) {
        sf::Event event;
        while (welcomeWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                welcomeWindow.close();
            }
            else if (event.type == sf::Event::TextEntered) {
                if (((event.text.unicode >= 65 && event.text.unicode <= 90) || // letters A-Z
                     (event.text.unicode >= 97 && event.text.unicode <= 122) || // letters a-z
                     event.text.unicode == 32 || // Space
                     event.text.unicode == 127)) { // Delete or Backspace
                    if (name.size() <= 10) {
                        name += static_cast<char>(event.text.unicode);
                        userInput.setString(name + "|"); // Name + cursor
                        // Center text
                        sf::FloatRect inputRect = userInput.getLocalBounds();
                        userInput.setOrigin(inputRect.left + inputRect.width/2.0f, inputRect.top  + inputRect.height/2.0f);
                    }
                }
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::BackSpace) {
                    if (!name.empty()) {
                        name.pop_back();
                        userInput.setString(name + "|");
                        // Center text
                        sf::FloatRect inputRect = userInput.getLocalBounds();
                        userInput.setOrigin(inputRect.left + inputRect.width/2.0f, inputRect.top  + inputRect.height/2.0f);
                    }
                }
            }
            else if (event.key.code == sf::Keyboard::Enter) {
                if (!name.empty()) {
                    finishName = true;
                    welcomeWindow.close();
                }

            }
        }
        welcomeWindow.clear(sf::Color::Blue);
        welcomeWindow.draw(welcomeText);
        welcomeWindow.draw(namePrompt);
        welcomeWindow.draw(userInput);
        welcomeWindow.display();
    }

    //  Store name with correct case
    name[0] = toupper(name[0]);
    for (int i = 1; i < name.size(); i++) {
        name[i] = tolower(name[i]);
    }

    ifstream scores("files/leaderboard.txt");
    if (!scores.is_open()) {
        cout << "Cannot open file." << endl;
    }

    vector<string> scoresVec;
    string line;
    while (getline(scores, line)) {
        if (!line.empty()) {  // check if line is not empty
            scoresVec.push_back(line);
        }
    }

    ofstream outFile;
    outFile.open("files/leaderboard.txt", ios::app);
    if (!outFile.is_open()) {
        cout << "Cannot open file" << endl;
    }

    if (finishName) { // If name entered is valid and enter key is pressed
        sf::RenderWindow gameWindow(sf::VideoMode(width, height), "Minesweeper");
        Game game; // Class instance
        Tiles tile;
        while (gameWindow.isOpen()) {
            sf::Event event;
            while (gameWindow.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    gameWindow.close();
                }
            }
            game.generateMines();
            gameWindow.clear(sf::Color::White);
            game.checkWin();
            game.displayFace(gameWindow, event);
            game.displayTimer(gameWindow);
            game.displayCounter(gameWindow);
            game.displayBoard(gameWindow, event);
            game.displayLeaderButton(gameWindow, event);
            game.displayDebug(gameWindow,event);
            game.displayPlayPause(gameWindow, event);
            game.updateText(name, outFile, scoresVec);
            game.compareTimes(scoresVec);
            gameWindow.display();
            game.displayLeaderBoard(scoresVec);
        }
    }
}

