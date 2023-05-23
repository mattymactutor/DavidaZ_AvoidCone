

#ifndef CONE_H
#define CONE_H

#include <SFML/Graphics.hpp>
#include <stdlib.h> /* srand, rand */
#include <time.h>   /* time */
using namespace sf;
using namespace std;


#define SPEED_LOW 0.02
#define SPEED_HIGH 0.03

class Cone
{
private:
    Sprite *cone;
    Texture * texture;
    bool isAlive_ = true;
    int SCREEN_HEIGHT;
    double SPEED = 0.02;

public:
    Cone(int SCREEN_WIDTH, int SCREEN_HEIGHT, int startX, int startY)
    {
        this->SCREEN_HEIGHT = SCREEN_HEIGHT;

        cone = new Sprite();
        texture = new Texture();
        texture->loadFromFile("traffic-cone.png");
        // apply the png image to the sprite
        cone->setTexture(*texture);
        // it's too big make it half size
        cone->setScale(0.02, 0.02);
        // by default this image will rotate around the top-left corner
        // fix it to be the center
        cone->setOrigin(cone->getTextureRect().width / 2, cone->getTextureRect().height / 2);

        int randX = rand() % (SCREEN_WIDTH - 50);
        // int randY = rand() % (SCREEN_HEIGHT - 50);
        int y = startY; // choose top of screen

        //get rand number from 0 to 1;
        double zeroTo1 = (double) rand() / RAND_MAX;
        //shift it and put it in the right range
        //          0.2   + (0.5 *                0.01)
        SPEED = SPEED_LOW + zeroTo1 * (SPEED_HIGH - SPEED_LOW);


        cone->setPosition(Vector2f(randX + startX, y));
        isAlive_ = true;
        //printf("Cone Created with speed: %.4f\n",SPEED);
    }

    ~Cone(){
       // delete cone;
        //printf("Cone memory cleaned!");
    }

    int checkIntersect(FloatRect rect){
        if ( rect.intersects(cone->getGlobalBounds())){
            return 1;
        } else if (!isAlive_){
            return 2;
        }

        return 0;
    }
    bool isAlive(){
        return isAlive_;
    }
    void draw(RenderWindow * window){        
            //move down
            Vector2f pos = cone->getPosition();
            pos.y += SPEED;
            cone->setPosition(pos);

            //if the y value goes outside the screen the cone should be removed
            //the next time checkIntersect is called
            if (pos.y > SCREEN_HEIGHT){
                isAlive_ = false;
            }

            window->draw(*cone);        
    }
};

#endif