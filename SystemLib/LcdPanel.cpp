#include "LcdPanel.h"
#include <cmath>
#include <stdexcept>

namespace {
using namespace sf;

void rounded(RenderTarget& target, float x, float y, float w, float h,
             float radius, Color color) {
    constexpr int steps = 8;
    constexpr float pi = 3.14159265358979323846f;
    ConvexShape shape(4 * (steps + 1));
    const Vector2f centers[] = {{x+w-radius, y+radius}, {x+w-radius, y+h-radius},
                                {x+radius, y+h-radius}, {x+radius, y+radius}};
    unsigned point = 0;
    for (int corner = 0; corner < 4; ++corner) {
        for (int step = 0; step <= steps; ++step) {
            const float angle = (corner - 1 + float(step) / steps) * pi / 2;
            shape.setPoint(point++, centers[corner] +
                Vector2f(std::cos(angle)*radius, std::sin(angle)*radius));
        }
    }
    shape.setFillColor(color);
    target.draw(shape);
}

void rect(RenderTarget& target, float x, float y, float w, float h, Color color) {
    RectangleShape shape({w, h});
    shape.setPosition(x, y);
    shape.setFillColor(color);
    target.draw(shape);
}

void circle(RenderTarget& target, float x, float y, float radius, Color color) {
    CircleShape shape(radius, 48);
    shape.setPosition(x-radius, y-radius);
    shape.setFillColor(color);
    target.draw(shape);
}

void quad(VertexArray& vertices, float x, float y, float w, float h, Color color) {
    vertices.append(Vertex({x, y}, color));
    vertices.append(Vertex({x+w, y}, color));
    vertices.append(Vertex({x+w, y+h}, color));
    vertices.append(Vertex({x, y+h}, color));
}

void makeFaceplate(RenderTarget& target) {
    target.clear(Color::Transparent);
    // Soft contact shadow, laminate edge, and green solder mask.
    for (int spread = 12; spread >= 0; --spread)
        rounded(target, 40-spread, 36-spread/2.f, 720+2*spread,
                300+spread, 5+spread, Color(0, 0, 0, 7));
    rounded(target, 40, 31, 720, 300, 5, Color(20, 45, 35));
    rounded(target, 40, 28, 720, 300, 5, Color(24, 100, 73));
    rect(target, 48, 29, 704, 1, Color(99, 150, 106));
    rect(target, 48, 326, 704, 1, Color(12, 64, 48));

    // Subtle copper routing visible beneath the solder mask.
    for (int i = 0; i < 5; ++i) {
        const Color trace(34, 112, 81);
        rect(target, 557+i*9, 45+i*6, 157-i*9, 2, trace);
        rect(target, 557+i*9, 45+i*6, 2, 44-i*6, trace);
        rect(target, 103+i*12, 305+i*3, 360-i*12, 1, trace);
    }
    // Plated mounting holes, including the dark bore and metal inner edge.
    for (float x : {60.f, 740.f}) {
        for (float y : {48.f, 308.f}) {
            circle(target, x, y+1, 13, Color(14, 67, 48));
            circle(target, x, y, 11, Color(202, 187, 115));
            circle(target, x, y, 8, Color(231, 214, 152));
            circle(target, x, y, 6.5f, Color(71, 65, 45));
            circle(target, x, y+1, 5.5f, Color(29, 32, 35));
        }
    }
    // Sixteen solder pads along the controller header.
    for (int pin = 0; pin < 16; ++pin) {
        const float x = 112.f + pin*27.f;
        rounded(target, x-6, 50, 12, 27, 3, Color(16, 69, 50));
        rounded(target, x-4, 52, 8, 23, 2, Color(184, 188, 166));
        rect(target, x-2, 54, 2, 19, Color(227, 223, 192));
        circle(target, x, 63, 2.5f, Color(57, 65, 57));
        rect(target, x-1, 78, 2, 4, Color(179, 200, 159));
    }
    // Stamped black metal surround and its folded lips.
    rounded(target, 73, 92, 654, 204, 11, Color(10, 46, 34));
    rounded(target, 75, 91, 650, 202, 10, Color(12, 15, 16));
    rounded(target, 77, 93, 646, 198, 9, Color(63, 66, 61));
    rounded(target, 79, 96, 642, 193, 8, Color(28, 31, 29));
    rounded(target, 91, 106, 618, 5, 2, Color(13, 16, 15));
    rect(target, 95, 107, 610, 1, Color(69, 72, 64));
    rounded(target, 91, 276, 618, 5, 2, Color(12, 15, 14));
    rect(target, 95, 280, 610, 1, Color(65, 68, 59));
    rounded(target, 96, 117, 608, 152, 7, Color(10, 12, 16));
    rounded(target, 98, 120, 604, 147, 5, Color(50, 60, 72));

    // The glass is dark navy with a restrained vertical illumination gradient.
    // This is a static material treatment, not frame-dependent LCD persistence.
    rounded(target, 100, 122, 600, 143, 4, Color(20, 32, 62));
    for (int row = 0; row < 139; ++row) {
        const float light = std::sin(float(row) / 139.f * 3.14159265f);
        rect(target, 102, 124+row, 596, 1,
             Color(20+int(light*5), 32+int(light*7), 61+int(light*12)));
    }
    rect(target, 105, 123, 590, 1, Color(76, 87, 103, 100));
    rect(target, 103, 125, 1, 136, Color(64, 77, 94, 70));
}
} // namespace

LcdPanel::LcdPanel() {
    if (!faceplate.create(Width, Height))
        throw std::runtime_error("Could not create the LCD panel render texture");
    makeFaceplate(faceplate);
    faceplate.display();
}

void LcdPanel::draw(sf::RenderTarget& target, const std::vector<int8_t>& pixels,
                    int columns, int rows, sf::RenderStates states) {
    target.draw(sf::Sprite(faceplate.getTexture()), states);
    dots.clear();
    constexpr float pitch = 6.f, dotSize = 5.f;
    const float left = std::floor((Width - (columns*pitch-1))/2.f);
    const float top = std::floor(193.5f - (rows*pitch-1)/2.f);
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            const auto state = pixels[y*columns+x];
            if (state < 0) continue; // Gaps between cells contain no electrodes.
            const float px = left + x*pitch, py = top + y*pitch;
            if (state) {
                quad(dots, px-1, py-1, dotSize+2, dotSize+2, Color(164, 194, 233, 16));
                quad(dots, px, py, dotSize, dotSize, Color(198, 218, 236));
                quad(dots, px, py, dotSize, 1, Color(225, 234, 241, 110));
            } else {
                quad(dots, px, py, dotSize, dotSize, Color(10, 20, 45, 42));
            }
        }
    }
    target.draw(dots, states);
}
