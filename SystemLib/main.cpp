#include <iostream>
#include <SFML/Graphics.hpp>
#include "System.h"
#include <cstdint>
#include <intrin.h>
#include "PortBus.h"

using namespace sf;

int main() {
    System system{0x00, 0x3fff, 0x6000, 0x7fff, 0x8000, 0xffff, .0001};
    system.loadProgram("a.out");

    RenderWindow window(VideoMode(400.f, 88.f), "SFML Application", Style::Close);
    //window.setFramerateLimit(60);

    CircleShape* lights[8];
    for (int i = 0; i < 8; ++i) {
        lights[i] = new CircleShape;
        lights[i]->setRadius(10);
        lights[i]->setPosition(Vector2f(325 - i * 40, 30));
        lights[i]->setOutlineThickness(2);
        lights[i]->setOutlineColor(Color::Red);
        lights[i]->setFillColor(Color::Black);
    }



    while(window.isOpen()) {
        system.cpu.execute(1);
        Event event;

        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }
        window.clear();
        for (int i = 0; i < 8; ++i) {
            if(system.lights.lights.test(i)) {
                lights[i]->setFillColor(Color::Red);
            } else {
                lights[i]->setFillColor(Color::Black);
            }
            window.draw(*lights[i]);
        }
        window.display();
    }
    for (int i = 0; i < 8; ++i) {
        delete lights[i];
    }
    return 0;
}

