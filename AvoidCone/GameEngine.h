#include <SFML/Graphics.hpp>
#include <vector>
#include <stdlib.h> /* srand, rand */
#include <time.h>   /* time */
#include "Cone.h"
using namespace sf;
using namespace std;

enum MODE
{
    START_MENU,
    IN_GAME,
    GAME_OVER,
    WIN,
    IDLE
};

MODE mode = START_MENU;
MODE changeMode = IDLE;

Font arial;
int sizeTitle = 60;
int sizeText = 40;
// GAME PARAMETERS
int SCREEN_WIDTH = 1200;
int SCREEN_HEIGHT = 700;
int DESKTOP_WIDTH = 1919;
int DESKTOP_HEIGHT = 1079;
int TIME_CONE = 1000; // num ms to spawn a new cone
int TIME_COIN = 250;
int NUM_COINS_TO_WIN = 5;
int TIME_PER_GAME = 60;
int PTS_PER_COIN = 1;

// CAR MOVEMENT
double speed = 0.035;
double angle = 90;

Time elapsed, lastCone, lastCoin, lastSecond;
int REDRAW_TIME = 200;
Clock gameClock, clockCoin, clockCone, timeClock, clockDraw;
bool moveLeft = false, moveRight = false, moveUp = false, moveDown = false;

// Game Objects
sf::RenderWindow *window;
// CAR
Sprite car;
Texture textureCar, tCoin, tCone;
RectangleShape walls;
Vector2f velocity;
vector<Drawable *> drawObjects;
vector<Drawable *> coins;
// vector<Drawable *> cones;
vector<Cone> cones;

int score = 0;
int secs = TIME_PER_GAME;
// Gameplay text
sf::Text txtScore, txtTime;
// Win Text

void gameOver();
void setupText(Text &t, String msg, int size, int x, int y, Color c);
void drawGameObjects();
void parseUSBCommand(string msg);

void parseUSBCommand(string msg)
{
    printf("USB CMD: %s\n", msg.c_str());

    // check up
    if (msg == "U")
    {
        moveRight = false;
        moveLeft = false;
    }
    // check left
    else if (msg == "L")
    {
        moveRight = false;
        moveLeft = true;
    }
    // check right
    else if (msg == "R")
    {
        moveRight = true;
        moveLeft = false;
    }
    // check idle
    else if (msg == "I")
    {
        moveUp = false;
        moveDown = false;
    }
    // check gas
    else if (msg == "F")
    {                  // negative is forward
        moveUp = true; // probably dont need this
        moveDown = false;
    }
    // check backward
    else if (msg == "B")
    {                   // backward
        moveUp = false; // probably dont need this
        moveDown = true;
    }
    // check for the button press
    else if (msg == "b")
    {
        //you can't change the screen on this thread so you have to change a variable and let the main thread do it
        if (mode == GAME_OVER || mode == WIN)
        {
            changeMode = START_MENU;
            printf("Change mode to START_MENU\n");
        }
        else if (mode == START_MENU)
        {
            changeMode = IN_GAME;
            printf("Change mode to IN_GAME\n");
        }
    }
}

Sprite *makeCoin()
{
    Sprite *c = new Sprite();

    tCoin.loadFromFile("star.png");
    // apply the png image to the sprite
    c->setTexture(tCoin);
    // it's too big make it half size
    c->setScale(0.9, 0.9);
    // by default this image will rotate around the top-left corner
    // fix it to be the center
    c->setOrigin(c->getTextureRect().width / 2, c->getTextureRect().height / 2);

    FloatRect wallB = walls.getGlobalBounds();

    // a coin should be placed at least 5 car widths away
    int randY = (rand() % (int)(wallB.height - 150)) + (wallB.top + 75);
    int distanceAway = car.getGlobalBounds().width * 5;
    int carX, carY, randX;
    do
    {
        randX = (rand() % (int)(wallB.width - 150)) + (wallB.left + 75);
        //printf("Generating coin spot\n");
        carX = car.getGlobalBounds().left + (car.getGlobalBounds().width / 2);
    } while (abs(randX - carX) < distanceAway);
    // make sure the coin is at least half the screen away

    c->setPosition(Vector2f(randX, randY));

    return c;
}

void newGame()
{
    drawObjects.clear();
    coins.clear();
    cones.clear();
    window->clear();
    // Init Car
    textureCar.loadFromFile("car.png");
    // apply the png image to the sprite
    car.setTexture(textureCar);
    // it's too big make it half size
    car.setScale(0.2, 0.2);
    // by default this image will rotate around the top-left corner
    // fix it to be the center
    car.setOrigin(car.getTextureRect().width / 2, car.getTextureRect().height / 2);
    car.setPosition(Vector2f(DESKTOP_WIDTH / 2, DESKTOP_HEIGHT / 2));
    car.setRotation(270);

    drawObjects.push_back(&car);

    // put the outside walls on
    walls.setSize(Vector2f(SCREEN_WIDTH - 40, SCREEN_HEIGHT - 40));
    walls.setPosition((DESKTOP_WIDTH / 2) - (SCREEN_WIDTH / 2), (DESKTOP_HEIGHT / 2) - (SCREEN_HEIGHT / 2));
    walls.setFillColor(Color(0, 0, 0));
    walls.setOutlineThickness(20);
    walls.setOutlineColor(Color(255, 0, 0));
    // prepend so it is the first thing drawn
    drawObjects.insert(drawObjects.begin(), &walls);

    // put the score on
    score = 0;
    FloatRect wallB = walls.getGlobalBounds();
    int scoreX = (wallB.left + wallB.width);
    setupText(txtScore, "Score: 0", sizeText, scoreX, -1, Color::White);
    txtScore.setPosition(scoreX - txtScore.getGlobalBounds().width - 50, wallB.top + 25);

    txtScore.setStyle(sf::Text::Bold);
    drawObjects.push_back(&txtScore);

    secs = TIME_PER_GAME;
    setupText(txtTime, "Time: " + to_string(secs), sizeText, wallB.left + 40, wallB.top + 25, Color::White);
    txtTime.setStyle(sf::Text::Bold);
    drawObjects.push_back((&txtTime));

    gameClock.restart();
    timeClock.restart();

    // if a button was held down when the game ends then it's flag is still true
    // reset
    moveLeft = false;
    moveRight = false;
    moveUp = false;
    moveDown = false;

    printf("...New Game...\n");
    drawGameObjects();
    mode = IN_GAME;
}

void win()
{
    mode = WIN;
    window->clear();

    Texture textReward;
    textReward.loadFromFile("steering_wheel_trophy.png");
    Sprite trophy;
    trophy.setTexture(textReward);
    trophy.setScale(0.5, 0.5);
    // by default this image will rotate around the top-left corner
    // fix it to be the center
    trophy.setOrigin(trophy.getTextureRect().width / 2, trophy.getTextureRect().height / 2);
    trophy.setPosition(Vector2f(DESKTOP_WIDTH / 2, DESKTOP_HEIGHT / 3));

    Text congrats;
    setupText(congrats, "You won!", sizeTitle, -1, (int)(DESKTOP_HEIGHT * 0.5), Color::White);
    window->draw(congrats);

    Text descr;
    setupText(descr, "Press [RED BUTTON] to replay", sizeText, -1, (int)(DESKTOP_HEIGHT * 0.6), Color::White);

    window->draw(trophy);
    window->draw(descr);
    window->display();
}

void winEvents(Event event)
{
    switch (event.type)
    {
    // window closed
    case sf::Event::Closed:
        window->close();
        break;
    case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::A)
        {
            newGame();
        }
        else if (event.key.code == Keyboard::Escape)
        {
            window->close();
        }
        break;
    }
}

void startMenu()
{
    window->clear();

    Text title;
    setupText(title, "Collect the Coins", sizeTitle, -1, (int)(DESKTOP_HEIGHT * 0.25), Color::White);
    window->draw(title);
    window->setMouseCursorVisible(false);

    Text description;
    int y = DESKTOP_HEIGHT * 0.5;
    setupText(description, "Collect the coins and avoid the cones.", sizeText, -1, y, Color::White);
    window->draw(description);
    setupText(description, "Don't hit the red walls!", sizeText, -1, y + 15 + sizeText, Color::White);
    window->draw(description);

    Text descr;
    setupText(descr, "Press [RED BUTTON] to play", sizeText, -1, (int)(DESKTOP_HEIGHT * 0.75), Color::White);

    window->draw(descr);
    window->display();
    mode = START_MENU;
}

void initGame()
{

    VideoMode desktop = VideoMode::getDesktopMode();
    DESKTOP_HEIGHT = desktop.height; // / 2;
    DESKTOP_WIDTH = desktop.width;   // / 2;
    printf("Desktop Width: %d   Height: %d\n", DESKTOP_WIDTH, DESKTOP_HEIGHT);
    window = new RenderWindow(sf::VideoMode(DESKTOP_WIDTH - 500, DESKTOP_HEIGHT - 500), "Car Game", Style::None);
    window->setPosition(Vector2i(0, 0));
    // load font
    if (!arial.loadFromFile("arial.ttf"))
    {
        printf("COULD NOT LOAD FONT\n");
        exit(1);
    }

    mode = START_MENU;
    startMenu();
}

void gamePlayEvents(Event event)
{
    switch (event.type)
    {
    // window closed
    case sf::Event::Closed:
        window->close();
        break;

    // key pressed
    case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::Left)
        {
            moveLeft = true;
        }
        else if (event.key.code == sf::Keyboard::Right)
        {
            moveRight = true;
        }
        else if (event.key.code == sf::Keyboard::Up)
        {
            moveUp = true;
        }
        else if (event.key.code == sf::Keyboard::Down)
        {
            moveDown = true;
        }
        else if (event.key.code == Keyboard::Escape)
        {
            window->close();
        }
        break;
    case sf::Event::KeyReleased:
        if (event.key.code == sf::Keyboard::Up)
        {
            moveUp = false;
        }
        else if (event.key.code == sf::Keyboard::Right)
        {
            moveRight = false;
        }
        else if (event.key.code == sf::Keyboard::Left)
        {
            moveLeft = false;
        }
        else if (event.key.code == sf::Keyboard::Down)
        {
            moveDown = false;
        }
        break;
    // we don't process other types of events
    default:
        break;
    }
}

bool checkCrash()
{
    // Check for off screen crash
    FloatRect carBounds = car.getGlobalBounds();
    FloatRect wallB = walls.getGlobalBounds();
    // account for it being centered on the screen
    int padding = 0;
    if (carBounds.top < wallB.top + padding)
    {
        printf("TOP SCREEN CRASH\n");
        gameOver();
        return true;
    }
    if (carBounds.left < wallB.left + padding)
    {
        printf("LEFT SCREEN CRASH\n");
        gameOver();
        return true;
    }
    if ((carBounds.top + carBounds.height) > (wallB.top + wallB.height) - padding)
    {
        printf("BOTTOM SCREEN CRASH\n");
        gameOver();
        return true;
    }
    if ((carBounds.left + carBounds.width) > (wallB.left + wallB.width))
    {
        printf("RIGHT SCREEN CRASH\n");
        gameOver();
        return true;
    }
    return false;
}

bool gamePlay()
{

    //---Create the coins---
    // If it has been more than X ms after the user just hit a coin and a coin has not been created yet
    // create a new coin.
    lastCoin = clockCoin.getElapsedTime();
    FloatRect wallB = walls.getGlobalBounds();
    if (lastCoin.asMilliseconds() > TIME_COIN && coins.size() == 0)
    {
        // create new coin if less than 3 on the screen
        coins.push_back(makeCoin());
        // also make a cone

        cones.push_back(Cone(wallB.width, wallB.top + wallB.height, wallB.left, wallB.top));
    }

    //---Create the cones---
    // If it has been more than X ms after a cone has been dropped
    // create a new cone.
    lastCone = clockCone.getElapsedTime();
    if (lastCone.asMilliseconds() > TIME_CONE)
    {
        // create new coin if less than 3 on the screen
        cones.push_back(Cone(wallB.width, wallB.top + wallB.height, wallB.left, wallB.top));
        clockCone.restart();
    }

    // use iterator to erase the cones that intersect
    auto it = cones.begin();
    while (it != cones.end())
    {
        // check for an intersection
        // this actually checks for intersection or if the cone went off screen in the last draw
        // the function sends back 0 or 1 to let you know which happened
        if (int res = (*it).checkIntersect(car.getGlobalBounds()))
        {
            it = cones.erase(it);
            if (res == 1)
            {
                printf("Cone Intersect\n");
                gameOver();
                return false;
            }
            else if (res == 2)
            {
                // printf("Cone off screen\n");
            }
        }
        else
        {
            it++;
        }
    }

    // use iterator to erase the coins that intersect
    auto it2 = coins.begin();
    while (it2 != coins.end())
    {
        Sprite *c = (Sprite *)(*it2);
        // if you intersect with coin erase it and add points
        if (car.getGlobalBounds().intersects(c->getGlobalBounds()))
        {
            it2 = coins.erase(it2);
            delete *it2;

            // if it does intersect restart clock so a new coin is made
            clockCoin.restart();
            score += 1;
            printf("Hit coin\n");
            // upate score object
            txtScore.setString("Score: " + to_string(score));
            if (score == NUM_COINS_TO_WIN)
            {
                win();
                return false;
            }
        }
        else
        {
            it2++;
        }
    }

    //--- Update the timer ---
    lastSecond = timeClock.getElapsedTime();
    if (lastSecond.asMilliseconds() > 1000)
    {
        timeClock.restart();

        secs--;
        if (TIME_PER_GAME == -1)
        {
            txtTime.setString("Time: Unlimited");
        }
        else
        {
            txtTime.setString("Time: " + to_string(secs));
        }
        if (secs == 0)
        {
            gameOver();
            return false;
        }
    }

    if (moveUp)
    {

        Vector2f pos = car.getPosition();
        pos.y -= speed;
        car.setPosition(pos);
    }
    else if (moveDown)
    {
        Vector2f pos = car.getPosition();
        pos.y += speed;
        car.setPosition(pos);
    }
    if (moveRight)
    {

        Vector2f pos = car.getPosition();
        pos.x += speed;
        car.setPosition(pos);
    }
    else if (moveLeft)
    {
        Vector2f pos = car.getPosition();
        pos.x -= speed;
        car.setPosition(pos);
    }

    // Check for off screen crash
    if (checkCrash())
    {
        return false;
    }

    return true;
}

void startMenuEvents(Event event)
{
    switch (event.type)
    {
    // window closed
    case sf::Event::Closed:
        window->close();
        break;
    case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::A)
        {
            newGame();
        }
        else if (event.key.code == Keyboard::Escape)
        {
            window->close();
        }
        break;
    }
}

void gameOverEvents(Event event)
{
    switch (event.type)
    {
    // window closed
    case sf::Event::Closed:
        window->close();
        break;
    case sf::Event::KeyReleased:
        if (event.key.code == sf::Keyboard::A)
        {

            newGame();
        }
        else if (event.key.code == Keyboard::Escape)
        {
            window->close();
        }
        break;
    }
}

void setupText(Text &t, String msg, int size, int x, int y, Color c)
{
    t.setFont(arial); // font is a sf::Font
    // set the string to display
    t.setString(msg);
    // set the character size
    t.setCharacterSize(size); // in pixels, not points!
    // set the color
    t.setFillColor(c);
    // set the text style
    // t.setStyle(sf::Text::Bold | sf::Text::Underlined);
    int xCenter = (DESKTOP_WIDTH / 2) - (t.getGlobalBounds().width / 2);
    int yCenter = (DESKTOP_HEIGHT / 2) - (t.getGlobalBounds().height / 2);
    // printf("%f\n", t.getGlobalBounds().width);
    if (x == -1)
    {
        x = xCenter;
    }
    if (y == -1)
    {
        y = yCenter;
    }
    t.setPosition(x, y);
}

void gameOver()
{
    window->clear();

    sf::Text gameOverText;
    // set the text style
    gameOverText.setStyle(sf::Text::Bold | sf::Text::Underlined);

    setupText(gameOverText, "GAME OVER", sizeTitle, -1, (int)(DESKTOP_HEIGHT * 0.25), Color::White);

    Text descr;
    setupText(descr, "Press [RED BUTTON] to replay", sizeText, -1, (int)(DESKTOP_HEIGHT * 0.6), Color::White);

    window->draw(gameOverText);
    window->draw(descr);
    window->display();

    mode = GAME_OVER;
    printf("GAME OVER MODE\n");
}

void drawGameObjects()
{
    window->clear();
    // redraw every object
    for (int i = 0; i < drawObjects.size(); i++)
    {
        window->draw(*drawObjects[i]);
    }
    for (size_t i = 0; i < coins.size(); i++)
    {
        window->draw(*coins[i]);
    }
    for (size_t i = 0; i < cones.size(); i++)
    {
        cones[i].draw(window);
    }

    window->display();
}