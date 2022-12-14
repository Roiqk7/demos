// Example of command to run this code. For more info visit official SFML website
//* g++ -std=c++17 moveQueen.cpp -o moveQueen -I/Users/roiqk/SFML/include -L/Users/roiqk/SFML/build/lib  -lsfml-graphics -lsfml-window -lsfml-system


#include <array>        
#include <iostream>
#include <SFML/Graphics.hpp>

// map related
#define N 8                 // size of one side of the square map
#define TILE_SIZE 100       // size of one tile on the map
#define QUEEN_SCALE 0.13    // scale used for queen to fit into one tile
#define DOT_SIZE 10         // size of dot which highlights tiles queen can move to

// window related
#define WIDTH N*TILE_SIZE   // window width
#define HEIGHT N*TILE_SIZE  // window height

// cosmetics
#define LIGHT_GREY sf::Color{192, 192, 192, 127}
#define PATH_TO_QUEEN_TEXTURE "/assets/goldQueen.png"

// return values
#define SUCCESS 0
#define ERROR -1

// create the window     
sf::RenderWindow window(sf::VideoMode(sf::Vector2u(WIDTH, HEIGHT)), "Move Queen");


/*
     ######  ##          ###     ######   ######  
    ##    ## ##         ## ##   ##    ## ##    ## 
    ##       ##        ##   ##  ##       ##       
    ##       ##       ##     ##  ######   ######  
    ##       ##       #########       ##       ## 
    ##    ## ##       ##     ## ##    ## ##    ## 
     ######  ######## ##     ##  ######   ######  
*/


// class which represents queen
class Queen {
    public:  
        sf::Texture goldQueenTexture;
        sf::Sprite queenSprite;
        int row;
        int col;

        Queen(int &row, int &col) {
            this->row = row;
            this->col = col;
            queenSprite.setScale(sf::Vector2f(QUEEN_SCALE, QUEEN_SCALE));
            if (!goldQueenTexture.loadFromFile(PATH_TO_QUEEN_TEXTURE)) std::exit(0);
            queenSprite.setTexture(goldQueenTexture);
        } 

        void render() {
            queenSprite.setPosition (sf::Vector2f(col*TILE_SIZE, row*TILE_SIZE));
            window.draw(queenSprite);
        }
};


// Board stores all important data and renders the actual board seen by the user
class Board {
    public:
        std::array<std::array<sf::RectangleShape, N>, N> boardTiles;     
        sf::Vector2i mousePos;
        sf::CircleShape dot;
        bool highlight;

        Board() {
            highlight = false;
            for (int row = 0; row < N; row++) {
                for (int col = 0; col < N; col++) { 
                    boardTiles[row][col].setPosition (sf::Vector2f(col*TILE_SIZE, row*TILE_SIZE));
                    boardTiles[row][col].setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                    dot.setRadius(DOT_SIZE);
                    dot.setFillColor(LIGHT_GREY);
                }
            }
        }

        // draws the board
        void render()
        {
            for (int row = 0; row < N; row++) {
                for (int col = 0; col < N; col++) {
                    if ((row + col) % 2) boardTiles[row][col].setFillColor(sf::Color::Black);
                    else boardTiles[row][col].setFillColor(sf::Color::White);
                    window.draw(boardTiles[row][col]);
                }
            }
        }

        // highlights tiles onto queen can move 
        void highlightTiles(Queen &queen)
        {
            for (int row = 0; row < N; row++) {
                for (int col = 0; col < N; col++) {
                    // prevents drawing dot over queen
                    if (row == queen.row && col == queen.col) continue;
                    //rows and cols
                    if (queen.row == row || queen.col == col) {
                        dot.setPosition(sf::Vector2f(col*TILE_SIZE+TILE_SIZE/2-DOT_SIZE, 
                                                     row*TILE_SIZE+TILE_SIZE/2-DOT_SIZE));
                    }
                    // diagonals
                    else if (checkQueenDiagonals(row, col, queen)) {
                        dot.setPosition(sf::Vector2f(col*TILE_SIZE+TILE_SIZE/2-DOT_SIZE, 
                                                     row*TILE_SIZE+TILE_SIZE/2-DOT_SIZE));
                    }
                    window.draw(dot);
                }
            }
        }

        // detects click and based on where user clicked (un)highlights tiles
        void clicked(Queen &queen)
        {
            mousePos = sf::Mouse::getPosition(window);
            if (clickedOnQueen(queen)) {
                highlight = true;
                return;
            }
            if (highlight) clickedOnHighlighted(queen);
            highlight = false;
        }

    private: 
        // checks if user clicked on queen
        bool clickedOnQueen(Queen &queen)
        {
            return (compareMousePosAndNum(mousePos.x, queen.col) && compareMousePosAndNum(mousePos.y, queen.row));
        }

        // checks if user clicked on tile queen can move to thus is highlighted
        void clickedOnHighlighted(Queen &queen)
        {
            // rows
            if (compareMousePosAndNum(mousePos.y, queen.row)) {
                queen.col = mousePos.x / 100;
                return;
            }
            // cols
            if (compareMousePosAndNum(mousePos.x, queen.col)) {
                queen.row = mousePos.y / 100;
                return;
            }
            // diagonals 
            for (int row = 0; row < N; row++) {
                for (int col = 0; col < N; col++) {
                    if (checkQueenDiagonals(row, col, queen)) {
                        if (compareMousePosAndNum(mousePos.x, col) && compareMousePosAndNum(mousePos.y, row)) {
                            queen.row = mousePos.y / 100;
                            queen.col = mousePos.x / 100;
                            return;
                        }
                    }
                }
            }
        }

        // decides if tile is on queen's diagonals
        bool checkQueenDiagonals(int &row, int &col, Queen &queen)
        {
            return (row + col == queen.row + queen.col || row - col == queen.row - queen.col);
        }   

        // decides if user clicked on certain tile
        bool compareMousePosAndNum(int &mousePos, int &num) {
            return (mousePos > num * TILE_SIZE && mousePos < num * TILE_SIZE + TILE_SIZE);
        }
};


void sfml(Board &board, Queen &queen);          // handles gui
void doEveryFrame(Board &board, Queen &queen);  // needs to happen every frame


/*
    ##     ##    ###    #### ##    ## 
    ###   ###   ## ##    ##  ###   ## 
    #### ####  ##   ##   ##  ####  ## 
    ## ### ## ##     ##  ##  ## ## ## 
    ##     ## #########  ##  ##  #### 
    ##     ## ##     ##  ##  ##   ### 
    ##     ## ##     ## #### ##    ## 
*/


// starts the program
int main()
{
    int startingRow = 0, startingCol = 4;
    Board board = Board();
    Queen queen = Queen(startingRow, startingCol);

    sfml(board, queen);
     
    return SUCCESS;
}


/*
     ######  ######## ##     ## ##       
    ##    ## ##       ###   ### ##       
    ##       ##       #### #### ##       
     ######  ######   ## ### ## ##       
          ## ##       ##     ## ##       
    ##    ## ##       ##     ## ##       
     ######  ##       ##     ## ######## 
*/


// visualizes the algorithm
void sfml(Board &board, Queen &queen)
{
    window.setFramerateLimit (60);
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) board.clicked(queen);
            }
        }

        doEveryFrame(board, queen);
    }
    return;
}


// helper function which is executed for every frame
void doEveryFrame(Board &board, Queen &queen)
{
    window.clear();
    board.render();
    queen.render();
    if (board.highlight) board.highlightTiles(queen);
    window.display();
}
