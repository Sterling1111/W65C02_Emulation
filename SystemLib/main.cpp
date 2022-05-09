#include <iostream>
#include <SFML/Graphics.hpp>
#include "System.h"
#include <cstdint>

using namespace sf;

Mutex mutex;


void RenderThread(RenderWindow* window, System* system, bool& running) {
    uint64_t renderDuration = ((Cycles::getTSCFrequency() * 1000000) / 60);     //30 Hz
    //TODO - the lcd screen is displaying nonsense data when the lcdFunctionDuration is too high
    uint64_t lcdFunctionDuration = (Cycles::getTSCFrequency() * 37);           //37 us

    RectangleShape lcdScreen;
    lcdScreen.setSize(Vector2f(380.f, 68.f));
    lcdScreen.setPosition(10.f, 10.f);
    lcdScreen.setFillColor(Color(31, 31, 255, 255));
    lcdScreen.setFillColor(Color::Transparent);

    RectangleShape* pixels[system->lcd.numPixelsX()][system->lcd.numPixelsY()];

    for (int y = 0; y < system->lcd.numPixelsY(); ++y) {
        for (int x = 0; x < system->lcd.numPixelsX(); ++x) {
            pixels[x][y] = new RectangleShape();
            pixels[x][y]->setSize(Vector2f(3.f, 3.f));
            pixels[x][y]->setPosition(Vector2f(lcdScreen.getPosition().x + x * 4, lcdScreen.getPosition().y + y * 4));
            pixels[x][y]->setFillColor(Color::White);
        }
    }

    if(window->isOpen()) {
        window->clear(Color(31, 31, 255, 255));
        window->draw(lcdScreen);
        window->display();
    }

    uint64_t renderStartTimePoint = __builtin_ia32_rdtsc();
    uint64_t lcdInstructionStartTimePoint;
    bool lcdInstructionCounterStarted{false};
    while (running)
    {
        mutex.lock();
        system->cpu.execute(1);
        mutex.unlock();
        if(system->lcd.busy) {
            if(lcdInstructionCounterStarted) {
                if((__builtin_ia32_rdtsc() - lcdInstructionStartTimePoint) > lcdFunctionDuration) {
                    system->lcd.busy = false;
                    lcdInstructionCounterStarted = false;
                }
            } else {
                lcdInstructionCounterStarted = true;
                lcdInstructionStartTimePoint = __builtin_ia32_rdtsc();
            }
        }


        if((__builtin_ia32_rdtsc() - renderStartTimePoint) > renderDuration) {
            window->clear(Color(31, 31, 255, 255));
            window->draw(lcdScreen);

            system->lcd.updatePixels();   // generates a snapshot of the pixels state
            for (int y = 0; y < system->lcd.numPixelsY(); ++y) {
                for (int x = 0; x < system->lcd.numPixelsX(); ++x) {
                    char pixel{system->lcd.pixelState(x, y)};
                    if(pixel == -1 || pixel == 0) {
                        if(y == 8 || x % 6 == 5) {
                            pixels[x][y]->setFillColor(Color(31, 31, 255, 255));
                        } else {
                            pixels[x][y]->setFillColor(Color(0, 0, 224, 255));
                        }
                    } else {
                        pixels[x][y]->setFillColor(Color(240, 240, 255, 255));
                    }
                    window->draw(*pixels[x][y]);
                }
            }
            window->display();
            renderStartTimePoint = __builtin_ia32_rdtsc();
        }
    }
    for (int y = 0; y < system->lcd.numPixelsY(); ++y) {
        for (int x = 0; x < system->lcd.numPixelsX(); ++x) {
            delete pixels[x][y];
        }
    }
    window->setActive(false);
}

int main() {
    System system{0x00, 0x3fff, 0x6000, 0x7fff,
                  0x8000, 0xffff, .75};
    system.loadProgram("a.out");
    RenderWindow window(VideoMode(400.f, 88.f),
                        "W65C02 Emulation", Style::Close);
    window.setKeyRepeatEnabled(false);
    bool running{true};
    Thread renderThread([capture0 = &window, capture1 = &system, &running] {
        return RenderThread(capture0, capture1, running);
    });
    window.setActive(false);
    renderThread.launch();
    Time delayTime = milliseconds(1);

    while(running) {
        sleep(delayTime);
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                running = false;
            if(event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::I)
                    system.cpu.IRQB = true;
                else if(event.key.code == Keyboard::N) {
                    system.cpu.NMIB = true;
                }
                else if(event.key.code == Keyboard::R) {
                    mutex.lock();
                    system.cpu.reset(system.eeprom[0xFFFC - 0x8000] | system.eeprom[0xFFFD - 0x8000] << 8);
                    mutex.unlock();
                }
            }
            if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::I)
                    system.cpu.IRQB = false;
                else if (event.key.code == Keyboard::N)
                    system.cpu.NMIB = false;
            }
        }
    }
    renderThread.wait();
    window.setActive(true);
    window.close();
    return 0;
}

