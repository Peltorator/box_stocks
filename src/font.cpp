#pragma once
#include <SFML/Graphics.hpp>

namespace NFont {
    sf::Font font;

    void GetFont() {
        font.loadFromFile("arial.ttf");
    }
};
