// This program lets user spawn random ball on click and watch it bounce with walls or other balls.
// Also user can shoot balls around, randomize visuals and see selected data life using other commands
// which are available in the console upon starting the program. 
// Enjoy!

// TODO let the user turn on gravity

#include <array>
#include <chrono>
#include <iostream>
#include <random>
#include <SFML/Graphics.hpp>
#include <vector>

// window
#define WIDTH 1400
#define HEIGHT 700

// balls
#define MAX_RADIUS 50
#define GRAVITY 0.981   
#define VELOCITY ((2*radius)*GRAVITY)
#define NO_BALL_SELECTED -1

// create the window     
sf::RenderWindow window(sf::VideoMode(sf::Vector2u(WIDTH, HEIGHT)), "Bounce Balls");


/*
     ######  ##          ###     ######   ######  
    ##    ## ##         ## ##   ##    ## ##    ## 
    ##       ##        ##   ##  ##       ##       
    ##       ##       ##     ##  ######   ######  
    ##       ##       #########       ##       ## 
    ##    ## ##       ##     ## ##    ## ##    ## 
     ######  ######## ##     ##  ######   ######  
*/


class Ball {
    public:
        float x, y;                     // coordinates of the center of the ball
        int radius;                     
        bool aim;                       // used to initiate aiming process.
        bool drag;                      // used to initiate dragging process     
        bool visualize;                 // is turned on by user

        // gives ball random color, size and puts it in the middle of where user clicked
        Ball() { 
            srand(clock()); 
            setRandomParameters();
            setPosition();
            aim = false;
            drag = false;
            visualize = false;
        }

        // tells the ball what to do
        void call(std::vector<Ball> &balls, size_t &ballCount)
        {
            if (checkIfOutOfWindow()) return;
            else {
                update(balls, ballCount);
                if (!stationary) {
                    checkBorder();
                    move();
                }
                if (aim) visualizeLine(cursorPos.x, cursorPos.y);
            }
        }

        // returns distance of 2 objects from their x and y coordinates
        template <typename T, typename U> float getDistanceFromCoordPairs(T &x1, U &x2, T &y1, U &y2)
        {
            float xDis, yDis;
            xDis = getCoordDistance(x1, x2);
            yDis = getCoordDistance(y1, y2);
            float distance;
            distance = getDistance(xDis, yDis);
            return distance;
        }

        // unleash chaos!
        void chaos()
        {
            setRandomParameters();
        }

        // shoot!
        void shoot()
        {
            xVel = (getCoordDistance(x, cursorPos.x)/2)*calculateSign(x, cursorPos.x);
            yVel = (getCoordDistance(y, cursorPos.y)/2)*calculateSign(y, cursorPos.y);
        }

    private:  
        // Ball()
        sf::CircleShape circle;
        sf::Vector2i cursorPos; 
        float cursorVel;  
        std::uint8_t r, g, b; 
        // update
        std::vector<std::array<float, 2>> closeBalls;   // holds indexes at [0] and distances at [1] of all nearby balls 
        int closeBallsCount;                            // number of nearby balls
        // movement
        bool stationary;        // used to decide what to do based on the state
        float xVel, yVel;       // holds the velocity
        float vel;              // holds overall velocity

        // removes the ball if the ball somehow gets completely out of the window
        bool checkIfOutOfWindow()
        {
            pushIntoWindow();
            if (x + radius < 0 || y + radius < 0 || x - radius > WIDTH || y - radius > HEIGHT) {
                x = -9999;
                y = -9999;
                return true;
            }
            else return false;
        }

        /*
            ######     #    #       #         ## ##   
            #     #   # #   #       #        #     #  
            #     #  #   #  #       #       #       # 
            ######  #     # #       #       #       # 
            #     # ####### #       #       #       # 
            #     # #     # #       #        #     #  
            ######  #     # ####### #######   ## ##   
            Functions called directly in the constructor                         
        */

        void setRandomParameters()
        {
            setRandomColor();
            setRandomSize();
            setRandomSpeed();
        } 

        // sets position of the ball to cursor
        void setPosition()
        {
            cursorPos = sf::Mouse::getPosition(window);
            x = cursorPos.x, y = cursorPos.y;
            circle.setPosition(sf::Vector2f(x, y));
        }

        /*                                           
            #    # #####  #####    ##   ##### ###### 
            #    # #    # #    #  #  #    #   #      
            #    # #    # #    # #    #   #   #####  
            #    # #####  #    # ######   #   #      
            #    # #      #    # #    #   #   #      
             ####  #      #####  #    #   #   ######     
            As title suggests updates all the values which need to be updated.    
        */

        // updates all the values that are vital to the whole process and also draws the ball
        void update(std::vector<Ball> &balls, size_t &ballCount)
        {
            render();
            checkStationary();
            vel = getDistance(xVel, yVel);
            if (ballCount > 1) {
                if (balls[0].visualize && !visualize) visualize = true;
                getCloseBalls(balls, ballCount);
                checkDistanceWithCloseBalls(balls);
            }
            if (aim || drag) handleCursor();
        }

        /*                                                          
            #    #  ####  #    # ###### #    # ###### #    # ##### 
            ##  ## #    # #    # #      ##  ## #      ##   #   #   
            # ## # #    # #    # #####  # ## # #####  # #  #   #   
            #    # #    # #    # #      #    # #      #  # #   #   
            #    # #    #  #  #  #      #    # #      #   ##   #   
            #    #  ####    ##   ###### #    # ###### #    #   #   
            Very simple yet very vital part of our program. 
        */

        // changes the coordinates and updates velocities with gravity
        void move()
        {
            x += xVel;
            y += yVel;
            xVel *= GRAVITY;
            yVel *= GRAVITY;
        }

        // checks if ball is about to crash into the border
        void checkBorder()
        {
            if (y + radius + yVel >= HEIGHT || y - radius + yVel < 0) changeDirection(yVel);
            if (x + radius + xVel >= WIDTH  || x - radius + xVel < 0) changeDirection(xVel);
        }

        /*
                                                                
             ####   ####  #      #      #  ####  #  ####  #    # 
            #    # #    # #      #      # #      # #    # ##   # 
            #      #    # #      #      #  ####  # #    # # #  # 
            #      #    # #      #      #      # # #    # #  # # 
            #    # #    # #      #      # #    # # #    # #   ## 
             ####   ####  ###### ###### #  ####  #  ####  #    # 
            If balls collide this changes their speeds accordingly.                             
        */  
        // closeBalls[i][1] - vel <= radius + balls[closeBalls[i][0]].radius
        bool willCollide(Ball &ball)
        {
            float nextX, nextY;
            nextX = x + xVel;
            nextY = y + yVel;
            float distance;
            distance = getDistanceFromCoordPairs(nextX, ball.x, nextY, ball.y);
            return distance < radius + ball.radius;
        }

        // first calculates new speeds and then assigns them
        void velAfterCollision(Ball &ball)
        {
            const float velMultiplier = 0.5;
            float newXVel, newYVel;
            newXVel = (abs(xVel) + abs(ball.xVel))*velMultiplier; 
            newYVel = (abs(yVel) + abs(ball.yVel))*velMultiplier; 

            xVel = newXVel * calculateSign(x, ball.x);
            ball.xVel = newXVel * calculateSign(ball.x, x);
            yVel = newYVel * calculateSign(y, ball.y);
            ball.yVel = newYVel * calculateSign(ball.y, y);
        }

        /*                        
            #####  #    #  ####  #    # 
            #    # #    # #      #    # 
            #    # #    #  ####  ###### 
            #####  #    #      # #    # 
            #      #    # #    # #    # 
            #       ####   ####  #    #         
            If ball is stationary and their distance < radius1 + radius2 then we locate side for each ball
            to which they are closer and they get pushed there.
        */

        // checks if balls are inside of each other
        bool isInsideOf(Ball &ball, float &distance)
        {
            return distance <= radius + ball.radius;
        }

        // pushes the ball out of each other
        void push(Ball &ball)
        {
            float xDis, yDis;
            xDis = getCoordDistance(x, ball.x);
            yDis = getCoordDistance(y, ball.y);
            if (xDis > yDis) {
                if (x < ball.x) subtractAndAdd(x, ball.x);
                else subtractAndAdd(ball.x, x);
            }
            else {
               if (y < ball.y) subtractAndAdd(y, ball.y);
               else subtractAndAdd(ball.y, y);
            }
        }

        // push back into window
        void pushIntoWindow()
        {
            while (x - radius <= 0) x++;
            while (x + radius >= WIDTH) x--;
            while (y - radius <= 0) y++;
            while (y + radius >= HEIGHT) y--;
        }

        /*                                              
             ####  #    # #####   ####   ####  #####  
            #    # #    # #    # #      #    # #    # 
            #      #    # #    #  ####  #    # #    # 
            #      #    # #####       # #    # #####  
            #    # #    # #   #  #    # #    # #   #  
             ####   ####  #    #  ####   ####  #    # 
            Everything related to cursor is handled here. 
            Well technically it handles action... but the 
            action is dependant on the cursor... hmmm...              
        */

        void handleCursor()
        {
            if (aim) cursorPos = sf::Mouse::getPosition(window);
            if (drag) {
                stationary = true;
                float oldCursorPosX, oldCursorPosY;
                oldCursorPosX = cursorPos.x, oldCursorPosY = cursorPos.y;
                setPosition();
                xVel = getCoordDistance(oldCursorPosX, x)*calculateSign(cursorPos.x, oldCursorPosX);
                yVel = getCoordDistance(oldCursorPosY, y)*calculateSign(cursorPos.y, oldCursorPosY);
            }
        }

        /*                                                                                    
            #    # #  ####  #    #   ##   #      # ###### ###### 
            #    # # #      #    #  #  #  #      #     #  #      
            #    # #  ####  #    # #    # #      #    #   #####  
            #    # #      # #    # ###### #      #   #    #      
             #  #  # #    # #    # #    # #      #  #     #      
              ##   #  ####   ####  #    # ###### # ###### ###### 
                                                                  
        */

        // draws line from center of the ball to entered position
        template <typename T, typename U> void visualizeLine(T &x2, U &y2)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(x, y)),
                sf::Vertex(sf::Vector2f(x2, y2))
            };
            window.draw(line, 2, sf::Lines);
        }

       /*
            #    # ###### #      #####  ###### #####   ####  
            #    # #      #      #    # #      #    # #      
            ###### #####  #      #    # #####  #    #  ####  
            #    # #      #      #####  #      #####       # 
            #    # #      #      #      #      #   #  #    # 
            #    # ###### ###### #      ###### #    #  ####  
            Helper functions. Small and mostly self explanatory.
        */

        void setRandomColor()
        {
            r = rand()%255, g = rand()%255, b = rand()%255;
            circle.setFillColor(sf::Color{r, g, b});
        }

        void setRandomSize()
        {
            radius = rand()%MAX_RADIUS+1;
            circle.setRadius(radius);
            circle.setOrigin(sf::Vector2f(radius, radius));
        }

        void setRandomSpeed()
        {
            xVel = rand()%(int) VELOCITY - VELOCITY/2;      
            yVel = rand()%(int) VELOCITY - VELOCITY/2;
        }

       // draws the ball on screen
        void render() 
        {
            circle.setPosition(sf::Vector2f(x, y));
            window.draw(circle);
        }

        // used to determine stationary state
        void checkStationary()
        {
            if (abs(xVel) < 0.1 && abs(yVel) < 0.1) stationary = true;
            else stationary = false;
        }

        // used in collision and push calculations
        void getCloseBalls(std::vector<Ball> &balls, size_t &ballCount)
        {
            const float isClose = vel + 10;
            closeBalls.resize(1);
            closeBallsCount = 0;

            // gets all balls that are considered close
            float distance;
            for (size_t i = 0; i < ballCount; i++) {
                if (x == balls[i].x && y == balls[i].y) continue;
                distance = getDistanceFromCoordPairs(x, balls[i].x, y, balls[i].y);
                if (distance - isClose <= radius + balls[i].radius) {
                    closeBalls[closeBallsCount] = {(float) i, distance};
                    closeBallsCount++;
                    closeBalls.resize(closeBallsCount+1);
                }
            }
        }

        // used to detect collision or need for push
        void checkDistanceWithCloseBalls(std::vector<Ball> &balls)
        {
            for (int i = 0; i < closeBallsCount; i++) {
                if (willCollide(balls[closeBalls[i][0]])) velAfterCollision(balls[closeBalls[i][0]]);
                if (isInsideOf(balls[closeBalls[i][0]], closeBalls[i][1])) push(balls[closeBalls[i][0]]);
                if (visualize) visualizeLine(balls[closeBalls[i][0]].x, balls[closeBalls[i][0]].y);
            }
        } 

        // gets distance of 2 points on the same axis
        template<typename T, typename U> float getCoordDistance(T &coord1, U &coord2)
        {
            return abs((coord1)-(coord2));
        }

        // gets distance based of 2 sides of the triangle
        float getDistance(float &side1, float &side2)
        {
            return sqrt((side1*side1)+(side2*side2));
        }

        // multiplies vel by -1
        void changeDirection(float &vel)
        {
            vel *= -1;
        }

        // subtracts 1 from the first number and adds 1 to the other one
        void subtractAndAdd(float &coord1, float &coord2)
        {
            coord1--;
            coord2++;
        }

        // returns sign based on size comparison (1st > 2nd ? 1: -1;)
        template<typename T, typename U> int calculateSign(T &coord1, U &coord2)
        {
            return coord1 > coord2 ? 1 : -1;
        }
};


void sfml();
void doEveryFrame(std::vector<Ball> &balls, size_t &ballCount, std::array<std::uint8_t, 3> &bgRGB);
void spawnBall(std::vector<Ball> &balls, size_t &ballCount);
void handleAction(Ball &ball, int &ballSelected, bool &action);
int getClickedBall(std::vector<Ball> &balls, size_t &ballCount, sf::Vector2i mousePos);


/*
    ##     ##    ###    #### ##    ## 
    ###   ###   ## ##    ##  ###   ## 
    #### ####  ##   ##   ##  ####  ## 
    ## ### ## ##     ##  ##  ## ## ## 
    ##     ## #########  ##  ##  #### 
    ##     ## ##     ##  ##  ##   ### 
    ##     ## ##     ## #### ##    ## 
*/


int main()
{
    const int numOfInputs = 6;
    std::array<char, numOfInputs> buttons = {'B', 'D', 'S', 'E', 'V', 'X'};
    std::array<std::string, numOfInputs> descriptions = {
        "to bulk spawn 5 balls",
        "to drag",
        "to aim and then again to shoot!",
        "for chaos",
        "for visualizations",
        "to reset"
    };
    std::cout << "Welcome to bounce balls!\n\n";
    std::cout << "Click to spawn a ball\n";
    for (int i = 0; i < numOfInputs; i++) {
        std::cout << "Press (" << buttons[i] << ") " << descriptions[i] << '\n';
    }
    sfml();
}


/*
     ######  ######## ##     ## ##       
    ##    ## ##       ###   ### ##       
    ##       ##       #### #### ##       
     ######  ######   ## ### ## ##       
          ## ##       ##     ## ##       
    ##    ## ##       ##     ## ##       
     ######  ##       ##     ## ######## 
    Draws on the window.
*/


void sfml()
{
    window.setFramerateLimit (60);
    window.setKeyRepeatEnabled(false);

    // background color
    std::array<std::uint8_t, 3> bgRGB;
    bgRGB[0] = 0, bgRGB[1] = 0, bgRGB[2] = 0;
    int ballSelected = NO_BALL_SELECTED;
    size_t ballCount = 0;
    std::vector<Ball> balls{1};

    // main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            // mouse events
            if (event.type == sf::Event::MouseButtonPressed) {
                // spawn
                if (event.mouseButton.button == sf::Mouse::Left) spawnBall(balls, ballCount);
            }
            // keyboard events
            if (event.type == sf::Event::KeyPressed) {
                // mass spawn
                if (event.key.code == sf::Keyboard::B) {
                    for (int i = 0; i < 5; i++) spawnBall(balls, ballCount);
                }
                // drag
                if (event.key.code == sf::Keyboard::D) {
                    if (ballSelected == NO_BALL_SELECTED) {
                        sf::Vector2i cursorPos = sf::Mouse::getPosition(window);
                        ballSelected = getClickedBall(balls, ballCount, cursorPos);
                    }
                    handleAction(balls[ballSelected], ballSelected, balls[ballSelected].drag);
                }
                // shoot
                if (event.key.code == sf::Keyboard::S) {
                    if (ballSelected == NO_BALL_SELECTED) {
                        sf::Vector2i cursorPos = sf::Mouse::getPosition(window);
                        ballSelected = getClickedBall(balls, ballCount, cursorPos);
                    }
                    handleAction(balls[ballSelected], ballSelected, balls[ballSelected].aim);
                }
                // chaos
                if (event.key.code == sf::Keyboard::E) {
                    for (size_t i = 0; i < ballCount; i++) balls[i].chaos();
                    bgRGB[0] = rand()%255, bgRGB[1] = rand()%255, bgRGB[2] = rand()%255;
                }
                // visualize
                if (event.key.code == sf::Keyboard::V) {
                    for (size_t i = 0; i < ballCount; i++) balls[i].visualize = true;
                }
                // clear
                if (event.key.code == sf::Keyboard::X) return sfml();
            }
        }
        doEveryFrame(balls, ballCount, bgRGB);
    }
}


// helper function which is executed every frame
void doEveryFrame(std::vector<Ball> &balls, size_t &ballCount, std::array<std::uint8_t, 3> &bgRGB)
{
    window.clear(sf::Color{bgRGB[0], bgRGB[1], bgRGB[2]});
    for (size_t i = 0; i < ballCount; i++) {
        balls[i].call(balls, ballCount);
    }
    window.display();
}


// handles spawning of a new ball
void spawnBall(std::vector<Ball> &balls, size_t &ballCount)
{
    balls[ballCount] = Ball();
    ballCount++;
    balls.resize(ballCount+1);
}


// handles shooting and dragging of the ball
void handleAction(Ball &ball, int &ballSelected, bool &action)
{
    if (!action && ballSelected != NO_BALL_SELECTED) action = true;
    else if (action) {
        if (ball.aim) ball.shoot();
        action = false;
        ballSelected = NO_BALL_SELECTED;
    }
}


// goes linearly though balls to find ball which user clicked on. Returns index if ball is found. Else -1.
int getClickedBall(std::vector<Ball> &balls, size_t &ballCount, sf::Vector2i cursorPos)
{
    float distance;
    for (size_t i = 0; i < ballCount; i++) {
        distance = balls[i].getDistanceFromCoordPairs(balls[i].x, cursorPos.x, balls[i].y, cursorPos.y);
        if (distance < balls[i].radius) {
            return i;
        }
    }
    return NO_BALL_SELECTED;
}
