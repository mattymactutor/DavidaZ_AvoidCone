#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include "stdio.h"
#include <math.h>
#include "Serial_Comm_SteeringWheel.h"
using namespace std;
using namespace sf;

#include "GameEngine.h"

// TODO
// Basic movement, coin drop, and cone drop works. the cones move but the global bounds is off and it's getting game over
// when the car doesnt hit a cone

// maybe a cone hit dents the car and you get 3 hits

// Documentation
// https://www.sfml-dev.org/tutorials/2.5/window-events.php

USB_Comm usb("/dev/ttyUSB0");

int main(int argc, char ** argv)
{
    srand(time(NULL));

    usb.setParseFunc(parseUSBCommand);

    if (argc > 1){
        TIME_PER_GAME = atoi(argv[0]);
    }

    initGame();

    while (window->isOpen())
    {
        elapsed = gameClock.getElapsedTime();
        gameClock.restart();
        sf::Event event;
        while (window->pollEvent(event))
        {
            if (mode == START_MENU)
            {
                startMenuEvents(event);
            }
            else if (mode == IN_GAME)
            {
                gamePlayEvents(event);
            }
            else if (mode == GAME_OVER)
            {
                gameOverEvents(event);
            }
            else if (mode == WIN)
            {
                winEvents(event);
            }

            // capture clicks
            if (event.type == Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    printf(" ( %d, %d)\n", event.mouseButton.x, event.mouseButton.y);
                }
            }
        }

        // redraw every so often
        //elapsed = clockDraw.getElapsedTime();
        //if (elapsed.asMilliseconds() > 200)
       // {
            clockDraw.restart();
            if (mode == IN_GAME)
            {
                // gameplay returns false when the game is over
                if (gamePlay())
                {
                    drawGameObjects();
                }
            }
           
           

             // change the screen on the main thread not in the thread for rec usb commands
            if (changeMode == START_MENU)
            {
                mode = START_MENU;
                // cant change the screen in this thread
                startMenu();
                changeMode = IDLE;
            }
            else if (changeMode == IN_GAME)
            {
                mode = IN_GAME;
                newGame();
                changeMode = IDLE;
            }
        //}
    }

    return 0;
}
