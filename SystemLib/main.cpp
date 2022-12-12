#include <iostream>
#include <SFML/Graphics.hpp>
#include "System.h"
#include <cstdint>
#include <chrono>

using namespace sf;

Mutex mutex;
bool next{};

void RenderThread(RenderWindow& window, System& system, bool& running) {
    uint64_t renderDuration = ((Cycles::getTSCFrequency() * 1000000) / 60);     //30 Hz
    RectangleShape lcdScreen;
    lcdScreen.setSize(Vector2f(380.f, 68.f));
    lcdScreen.setPosition(10.f, 10.f);
    lcdScreen.setFillColor(Color(31, 31, 255, 255));
    lcdScreen.setFillColor(Color::Transparent);

    RectangleShape pixels[system.lcd.numPixelsX()][system.lcd.numPixelsY()];

    for (int y = 0; y < system.lcd.numPixelsY(); ++y) {
        for (int x = 0; x < system.lcd.numPixelsX(); ++x) {
            pixels[x][y] = RectangleShape();
            pixels[x][y].setSize(Vector2f(3.f, 3.f));
            pixels[x][y].setPosition(Vector2f(lcdScreen.getPosition().x + x * 4, lcdScreen.getPosition().y + y * 4));
            pixels[x][y].setFillColor(Color::White);
        }
    }

    if(window.isOpen()) {
        window.clear(Color(31, 31, 255, 255));
        window.draw(lcdScreen);
        window.display();
    }

    uint64_t renderStartTimePoint = __builtin_ia32_rdtsc();
    while (running)
    {
        if(system.firstReset) {
            mutex.lock();
            system.cpu.execute(1);
            mutex.unlock();
        }

        if((__builtin_ia32_rdtsc() - renderStartTimePoint) > renderDuration) {
            window.clear(Color(31, 31, 255, 255));
            window.draw(lcdScreen);

            system.lcd.updatePixels();   // generates a snapshot of the pixels state
            for (int y = 0; y < system.lcd.numPixelsY(); ++y) {
                for (int x = 0; x < system.lcd.numPixelsX(); ++x) {
                    RectangleShape& pixel = pixels[x][y];
                    uint8_t red = pixel.getFillColor().r;
                    uint8_t alpha = pixel.getFillColor().a;
                    if(y == 8 || x % 6 == 5) {
                        pixel.setFillColor(Color(31, 31, 255, 255));
                        continue;
                    }
                    if(!system.firstReset) {
                        if(y < 8) {
                            pixel.setFillColor(Color::White);
                        } else {
                            pixel.setFillColor(Color(0, 0, 224, 100));
                        }
                    } else {
                        char pixelState{system.lcd.pixelState(x, y)};
                        if(pixelState == -1 || pixelState == 0) {
                            if(red != 0) {
                                auto rg = red * .85;
                                pixel.setFillColor(Color(rg, rg, 224, 225));
                            } else {
                                pixel.setFillColor(Color(0, 0, 224, 225));
                            }
                        } else {
                            if(red != 255) {
                                uint16_t rg = std::min((uint16_t )255, (uint16_t )((red + 10) * 1.5));
                                pixel.setFillColor(Color(rg, rg, 255, 255));
                            } else {
                                pixels[x][y].setFillColor(Color::White);
                            }
                        }
                    }
                    window.draw(pixels[x][y]);
                }
            }
            window.display();
            renderStartTimePoint = __builtin_ia32_rdtsc();
        }
    }
    window.setActive(false);
}

int main() {
    int dog = 3;
    System system{0x00, 0x3fff, 0x6000, 0x7fff,
                  0x8000, 0xffff, .199};
    system.loadProgram("a.out");
    RenderWindow window(VideoMode(400.f, 88.f),
                        "W65C02 Emulation", Style::Close);
    window.setKeyRepeatEnabled(false);
    bool running{true};
    Thread renderThread([&window, &system, &running] {
        return RenderThread(window, system, running);
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
                    system.firstReset = true;
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

