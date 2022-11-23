// this program lets user spawn random balls on click and bounce them around

/*
    TODO collision works but needs to be optimized in these ways:
        -change directory is 
        -account for vel in collisions 
    TODO push out
    ? user can select ball, hold click on it, add strength by dragging and shoot
        -if user clicks on ball and doesn't release
        -speedometer pops up showing direction and strength
        -calculate coords of cursor, add opposite values to velocities
*/


#include <array>
#include <chrono>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <random>

#define WIDTH 1400
#define HEIGHT 700

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
        sf::CircleShape circle;
        sf::Vector2i mousePos;
        int radius;
        std::array<float, 2> center;
        // movement
        float x, y;
        float xVel, yVel;
        float vel;

        // gives ball random color, size and puts it in the middle of where user clicked
        Ball() {
            srand(clock());  
            // visual
            // color
            std::uint8_t r = rand()%255, g = rand()%255, b = rand()%255;
            circle.setFillColor(sf::Color{r, g, b});
            // size
            radius = rand()%50+1;
            circle.setRadius(radius);
            // position
            mousePos = sf::Mouse::getPosition(window); 
            x = mousePos.x-radius, y = mousePos.y-radius;
            circle.setPosition(sf::Vector2f(x, y));
            // movement
            xVel = rand()%(int) VELOCITY - VELOCITY/2;      
            yVel = rand()%(int) VELOCITY - VELOCITY/2;
            stationary = false;
            // collision
            closestDistance = closestIndex = 9999;
        }

        void render() 
        {
            circle.setPosition(sf::Vector2f(x, y));
            window.draw(circle);
        }

        void move(std::vector<Ball> &balls, size_t &ballCount)
        {
            update();
            if (!stationary) {
                if (ballCount > 1) checkCollision(balls, ballCount);
                checkBorder();
                x += xVel;
                y += yVel;
                xVel *= GRAVITY;
                yVel *= GRAVITY;
            }
        }

    private:  
        bool stationary;    
        float closestDistance, closestIndex;

        void update()
        {
            checkStationary();
            if (!stationary) {
                center[0] = x + radius, center[1] = y + radius;
                vel = sqrt((xVel*xVel)+(yVel*yVel));
            }
        }

        void checkBorder()
        {
            if (y + (2*radius) + yVel >= HEIGHT || y + yVel < 0) changeDirection(yVel);
            if (x + (2*radius) + xVel >= WIDTH || x + xVel < 0) changeDirection(xVel);
        }

        void changeDirection(float &vel)
        {
            vel *= -1;
        }

        void checkStationary()
        {
            if (abs(xVel) < 0.1 && abs(yVel) < 0.1) stationary = true;
        }

    /*
                                                              
         ####   ####  #      #      #  ####  #  ####  #    # 
        #    # #    # #      #      # #      # #    # ##   # 
        #      #    # #      #      #  ####  # #    # # #  # 
        #      #    # #      #      #      # # #    # #  # # 
        #    # #    # #      #      # #    # # #    # #   ## 
         ####   ####  ###### ###### #  ####  #  ####  #    # 
                                                      
    */
        void checkCollision(std::vector<Ball> &balls, size_t &ballCount)
        {
            std::vector<int> closeBallIndexes;
            for (size_t i = 0; i < ballCount; i++) {
                if (x == balls[i].x && y == balls[i].y) continue;

                float xDis, yDis;
                xDis = abs((center[0])-(balls[i].center[0]));
                yDis = abs((center[1])-(balls[i].center[1]));
                float distance;
                distance = getDistance(xDis, yDis);

                if (distance < closestDistance) {
                    closestDistance = distance;
                    closestIndex = i;
                }
                else continue;

                if (closestDistance-vel <= radius + balls[i].radius) {
                    balls[i].stationary = false;
                    velAfterCollision(xVel, yVel, balls[i].xVel, balls[i].yVel);
                    changeDirection(xVel, yVel, balls[i].xVel, balls[i].yVel);
                }
            }

            visualizeDistance(center[0], center[1], 
                balls[closestIndex].center[0], balls[closestIndex].center[1]);
        }

        float getDistance(float &x, float &y)
        {
            return sqrt((x*x)+(y*y));
        }

        void velAfterCollision(float &aVel, float &bVel, float &cVel, float &dVel)
        {
            float newXVel, newYVel;
            newXVel = (abs(aVel)+abs(cVel))/2;
            newYVel = (abs(bVel)+abs(dVel))/2;
            aVel = newXVel;
            cVel = -newXVel;
            bVel = newYVel;
            dVel = -newYVel;
        }

        void changeDirection(float &aVel, float &bVel, float &cVel, float &dVel)
        {
            aVel *= -1;
            bVel *= -1;
            cVel *= -1;
            dVel *= -1;
        }

        // ! delete
        void visualizeDistance(float &aX, float &aY, float &bX, float &bY)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(aX, aY)),
                sf::Vertex(sf::Vector2f(bX, bY))
            };
            window.draw(line, 2, sf::Lines);
        }                 
};


void sfml();
void doEveryFrame(std::vector<Ball> &balls, size_t &ballCount);
void spawnBall(std::vector<Ball> &balls, size_t &ballCount);


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

    size_t ballCount = 0;
    std::vector<Ball> balls{1};
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) spawnBall(balls, ballCount);
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
        balls[i].move(balls, ballCount);
        balls[i].render();
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
