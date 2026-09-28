#ifndef W65C02_BREADBOARD_VIEW_H
#define W65C02_BREADBOARD_VIEW_H

#include "LcdPanel.h"
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <vector>

// A view of the existing emulated board, not another CPU or circuit simulator.
class BreadboardView {
public:
    static constexpr unsigned Width = 1200;
    static constexpr unsigned Height = 900;
    enum class Button { Released, Reset, Irq, Nmi };
    struct Snapshot {
        std::vector<int8_t> pixels;
        int lcdWidth{}, lcdHeight{};
        uint16_t pc{};
        uint8_t a{}, x{}, y{}, pa{}, pb{};
        double frequencyHz{};
        bool started{}, stopped{}, waiting{}, irq{}, paused{};
    };

    BreadboardView();
    void draw(sf::RenderTarget& target, const Snapshot& state, Button pressed);
    static Button hitTest(sf::Vector2f point);
    static sf::Vector2f boardPoint(sf::Vector2i pixel, sf::Vector2u windowSize);

private:
    sf::Font font;
    sf::RenderTexture background;
    sf::RenderTexture lcdWires;
    LcdPanel lcd;
};

#endif
