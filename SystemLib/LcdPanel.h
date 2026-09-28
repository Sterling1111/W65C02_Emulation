#ifndef W65C02_LCD_PANEL_H
#define W65C02_LCD_PANEL_H

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <vector>

// Presentation of the board's 16x2 module. Controller pixels remain authoritative.
class LcdPanel {
public:
    static constexpr unsigned Width = 800;
    static constexpr unsigned Height = 360;

    LcdPanel();
    void draw(sf::RenderTarget& target, const std::vector<int8_t>& pixels,
              int columns, int rows, sf::RenderStates states = sf::RenderStates::Default);

private:
    sf::RenderTexture faceplate;
    sf::VertexArray dots{sf::Quads};
};

#endif
