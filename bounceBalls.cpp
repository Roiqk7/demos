// this program lets user spawn random balls on click and bounce them around

/*
    TODO beautification -> separate large functions from helper functions, better function parameters (coord1...)
        -templates for functions
    TODO optimization and bug fixing
*/


#include <array>
#include <chrono>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <random>
#include <vector>

// window
#define WIDTH 1400
#define HEIGHT 700

// balls
#define MAX_RADIUS 50
#define GRAVITY 0.981   
#define VELOCITY ((2*radius)*GRAVITY)

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
        float x, y;                     // coordinates of the top left of the ball
        int radius;                     
        std::array<float, 2> center;    // array containing coordinates of the center of the ball. [0] for x, [1] for y
        bool clicked;                   // used to initiate aiming process.

        // gives ball random color, size and puts it in the middle of where user clicked
        Ball() {
            srand(clock());  
            // color
            std::uint8_t r = rand()%255, g = rand()%255, b = rand()%255;
            circle.setFillColor(sf::Color{r, g, b});
            // size
            radius = rand()%MAX_RADIUS+1;
            circle.setRadius(radius);
            // position
            mousePos = sf::Mouse::getPosition(window); 
            x = mousePos.x-radius, y = mousePos.y-radius;
            circle.setPosition(sf::Vector2f(x, y));
            // movement
            xVel = rand()%(int) VELOCITY - VELOCITY/2;      
            yVel = rand()%(int) VELOCITY - VELOCITY/2;
            stationary = false;
            // shoot
            clicked = false;
        }

        // tells the ball what to do
        void call(std::vector<Ball> &balls, size_t &ballCount)
        {
            if (checkIfOutOfWindow()) return;
            else {
                update(balls, ballCount);
                if (!stationary) {
                    if (ballCount > 1) checkCollision(balls);
                    checkBorder();
                    move();
                }
                if (clicked) visualizeShoot();
            }
        }

        // used in getClickedBall() so it needs to be public
        template <typename T, typename U> float getDistanceFromCoordPairs(T &x1, U &x2, T &y1, U &y2)
        {
            float xDis, yDis;
            xDis = getCoordDistance(x1, x2);
            yDis = getCoordDistance(y1, y2);
            float distance;
            distance = getDistance(xDis, yDis);
            return distance;
        }

        // ! wrong signs
        void shoot()
       {
            xVel = -getCoordDistance(center[0], cursorPos.x)/2;
            yVel = -getCoordDistance(center[1], cursorPos.y)/2;
            clicked = false;
       }

    private:  
        // Ball()
        sf::CircleShape circle;
        sf::Vector2i mousePos;
        // update
        std::vector<std::array<float, 2>> closeBalls;   // holds indexes at [0] and distances at [1] of all nearby balls 
        int closeBallsCount;                            // number of nearby balls
        // movement
        bool stationary;        // used to decide what to do based on the state
        float xVel, yVel;       // holds the velocity
        float vel;              // holds over all velocity needed in calculation of collisions
        // shoot
        sf::Vector2i cursorPos;    

        // removes the ball if even single pixel is out of the window
        bool checkIfOutOfWindow()
        {
            if (x < 0 || y < 0 || x + (2*radius) > WIDTH || y + (2*radius) > HEIGHT) {
                x = -9999;
                y = -9999;
                calculateCenter();
                return true;
            }
            else return false;
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
            calculateCenter();
            vel = getDistance(xVel, yVel);
            if (ballCount > 1) {
                getCloseBalls(balls, ballCount);
                checkPush(balls, ballCount);
            }
            if (clicked) cursorPos = sf::Mouse::getPosition(window);
        }

        // draws the ball
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

        // calculates the middle of the ball
        void calculateCenter()
        {
            center[0] = x + radius;
            center[1] = y + radius;
        }

        // used in collision and push calculations. Basically finds all nearby balls 
        void getCloseBalls(std::vector<Ball> &balls, size_t &ballCount)
        {
            const int isClose = vel + 10;
            closeBalls.resize(1);
            closeBallsCount = 0;

            // gets all distances that are considered close
            for (size_t i = 0; i < ballCount; i++) {
                if (x == balls[i].x && y == balls[i].y) continue;
                float distance;
                distance = getDistanceFromCoordPairs(center[0], balls[i].center[0], center[1], balls[i].center[1]);
                if (distance-vel <= radius+balls[i].radius+isClose) {
                    closeBalls[closeBallsCount] = {(float) i, distance};
                    closeBallsCount++;
                    closeBalls.resize(closeBallsCount+1);
                }
            }
        }

        // checks if any ball is inside
        void checkPush(std::vector<Ball> &balls, size_t &ballCount)
        {
            for (int i = 0; i < closeBallsCount; i++) {
                if (closeBalls[i][1] <= radius + balls[closeBalls[i][0]].radius) push(balls, i);
            }
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
            if (y + (2*radius) + yVel >= HEIGHT || y + yVel < 0) changeDirection(yVel);
            if (x + (2*radius) + xVel >= WIDTH || x + xVel < 0) changeDirection(xVel);
        }

        /*
                                                                
             ####   ####  #      #      #  ####  #  ####  #    # 
            #    # #    # #      #      # #      # #    # ##   # 
            #      #    # #      #      #  ####  # #    # # #  # 
            #      #    # #      #      #      # # #    # #  # # 
            #    # #    # #      #      # #    # # #    # #   ## 
             ####   ####  ###### ###### #  ####  #  ####  #    # 
            First ball finds all balls which are considered close. Then calculates if they are colliding
            based on simple formula: distance-velocity1 <= radius1 + radius2    
            If they collide their velocities are added up and divided by 2 so their xVel and yVel 
            after collision are equal. They also change directions.                               
        */

        // checks if ball is about to collide
        void checkCollision(std::vector<Ball> &balls)
        {
            for (int i = 0; i < closeBallsCount; i++) {
                if (closeBalls[i][1]-vel <= radius + balls[closeBalls[i][0]].radius) {
                    balls[closeBalls[i][0]].stationary = false;
                    velAfterCollision(balls[closeBalls[i][0]].xVel, balls[closeBalls[i][0]].yVel);
                    changeDirection(xVel, yVel, balls[closeBalls[i][0]].xVel, balls[closeBalls[i][0]].yVel);
                }

                // this visualizes distances of the ball to close balls
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f(center[0], center[1])),
                    sf::Vertex(sf::Vector2f(balls[closeBalls[i][0]].center[0], balls[closeBalls[i][0]].center[1]))
                };
                window.draw(line, 2, sf::Lines);
            }
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

        // pushes the ball out of each other
        void push(std::vector<Ball> &balls, int &i)
        {
            float xDis, yDis;
            xDis = getCoordDistance(center[0], balls[closeBalls[i][0]].center[0]);
            yDis = getCoordDistance(center[1], balls[closeBalls[i][0]].center[1]);
            if (xDis > yDis) {
                if (center[0] < balls[closeBalls[i][0]].center[0]) subtractAndAdd(x, balls[closeBalls[i][0]].x);
                else subtractAndAdd(balls[closeBalls[i][0]].x, x);
            }
            else {
               if (center[1] < balls[closeBalls[i][0]].center[1]) subtractAndAdd(y, balls[closeBalls[i][0]].y);
               else subtractAndAdd(balls[closeBalls[i][0]].y, y);
            }
        }

        /*                                
             ####  #    #  ####   ####  ##### 
            #      #    # #    # #    #   #   
             ####  ###### #    # #    #   #   
                 # #    # #    # #    #   #   
            #    # #    # #    # #    #   #   
             ####  #    #  ####   ####    #   
            If user selects the ball, speedometer appears to show user strength and direction of 
            the shot. We then calculate the velocity as distance of center and mouse position
            and add opposite values to current xVel and yVel when shot.              
        */

       void visualizeShoot()
       {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(center[0], center[1])),
                sf::Vertex(sf::Vector2f(cursorPos.x, cursorPos.y))
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

        template<typename T, typename U> float getCoordDistance(T &coord1, U &coord2)
        {
            return abs((coord1)-(coord2));
        }

        float getDistance(float &coord1, float &coord2)
        {
            return sqrt((coord1*coord1)+(coord2*coord2));
        }

        void changeDirection(float &vel)
        {
            vel *= -1;
        }

        void velAfterCollision(float &xVel2, float &yVel2)
        {
            float newXVel, newYVel;
            newXVel = getDistance(xVel, xVel2)/2; 
            newYVel = getDistance(yVel, yVel2)/2; 
            xVel = newXVel;
            xVel2 = -newXVel;
            yVel = newYVel;
            yVel2 = -newYVel;
        }

        void changeDirection(float &aXVel, float &aYVel, float &bXVel, float &bYVel)
        {
            aXVel *= -1;
            aYVel *= -1;
            bXVel *= -1;
            bYVel *= -1;
        }

        void subtractAndAdd(float &coord1, float &coord2)
        {
            coord1--;
            coord2++;
        }
};


void sfml();
void doEveryFrame(std::vector<Ball> &balls, size_t &ballCount);
void spawnBall(std::vector<Ball> &balls, size_t &ballCount);
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
    std::cout << "Welcome to bounce balls!\n";
    std::cout << "Press (S) while hovering over ball to select it.\nThen select direction and strength and press (S) again to shoot\n";
    std::cout << "Press (X) to clear window\n";
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
*/


// visualizes the balls
void sfml()
{
    window.setFramerateLimit (60);
    window.setKeyRepeatEnabled(false);

    int ballSelected;
    size_t ballCount = 0;
    std::vector<Ball> balls{1};

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) spawnBall(balls, ballCount);
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::X) {
                    return sfml();
                }
                if (event.key.code == sf::Keyboard::S) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    if (ballSelected == -1) {
                        ballSelected = getClickedBall(balls, ballCount, mousePos);
                        balls[ballSelected].clicked = true;
                        continue;
                    }
                    balls[ballSelected].shoot();
                    balls[ballSelected].clicked = false;
                    ballSelected = -1;
                }
            }
        }
        doEveryFrame(balls, ballCount);
    }
    return;
}


// helper function which is executed every frame
void doEveryFrame(std::vector<Ball> &balls, size_t &ballCount)
{
    window.clear(sf::Color::Black);
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


float getDistance(float &num1, float &num2)
{
    return sqrt((num1*num1)+(num2*num2));
}


// goes linearly though balls to find ball which user clicked on. Returns index if ball is found. Else -1.
int getClickedBall(std::vector<Ball> &balls, size_t &ballCount, sf::Vector2i mousePos)
{
    float distance;
    for (size_t i = 0; i < ballCount; i++) {
        distance = balls[i].getDistanceFromCoordPairs(balls[i].center[0], mousePos.x, balls[i].center[1], mousePos.y);
        if (distance < balls[i].radius) {
            return i;
        }
    }
    return -1;
}
