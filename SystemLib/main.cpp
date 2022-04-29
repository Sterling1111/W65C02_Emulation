#include <iostream>
#include <SFML/Graphics.hpp>
#include "System.h"
#include <cstdint>
#include <intrin.h>
#include "PortBus.h"
#include "LCD.h"

using namespace sf;

int main() {
    LCD lcd;
    System system{0x00, 0x3fff, 0x6000, 0x7fff, 0x8000, 0xffff, .0001};
    system.loadProgram("a.out");

    RenderWindow window(VideoMode(400.f, 88.f), "SFML Application" /*Style::Close*/);
    window.setFramerateLimit(30);
    RectangleShape lcdScreen;
    lcdScreen.setSize(Vector2f(380.f, 68.f));
    lcdScreen.setPosition(10.f, 10.f);
    lcdScreen.setFillColor(Color(31, 31, 255, 255));

    RectangleShape* pixels[system.lcd.numPixelsX()][system.lcd.numPixelsY()];

    for (int y = 0; y < system.lcd.numPixelsY(); ++y) {
        for (int x = 0; x < system.lcd.numPixelsX(); ++x) {
            pixels[x][y] = new RectangleShape();
            pixels[x][y]->setSize(Vector2f(3.f, 3.f));
            pixels[x][y]->setPosition(Vector2f(lcdScreen.getPosition().x + x * 4, lcdScreen.getPosition().y + y * 4));
            pixels[x][y]->setFillColor(Color::White);
            pixels[x][y]->setOutlineThickness(3);
            pixels[x][y]->setOutlineColor(Color::Transparent);
        }
    }

    while (window.isOpen())
    {
        system.cpu.execute(1);
        Event event;

        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }



        window.clear();
        window.draw(lcdScreen);

        system.lcd.updatePixels();   // generates a snapshot of the pixels state
        for (int y = 0; y < system.lcd.numPixelsY(); ++y) {
            for (int x = 0; x < system.lcd.numPixelsX(); ++x) {
                char pixel{system.lcd.pixelState(x, y)};
                if(pixel == -1 || pixel == 0) {
                    pixels[x][y]->setFillColor(Color(0, 0, 224, 255));
                } else {
                    pixels[x][y]->setFillColor(Color(240, 240, 255, 255));
                }
                window.draw(*pixels[x][y]);
            }
        }
        window.display();
    }

    for (int y = 0; y < system.lcd.numPixelsY(); ++y) {
        for (int x = 0; x < system.lcd.numPixelsX(); ++x) {
            delete pixels[x][y];
        }
    }
}

