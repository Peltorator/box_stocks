#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>
#include "button.cpp"
#include "font.cpp"

struct ItemTile {
    float X;
    float Y;
    float Dx;
    float Dy;
    uint64_t ItemID;
    std::string ItemName;
    uint32_t CurCnt;
    uint32_t MaxCnt;
    bool ShowCnt;
    Button MinusButton;
    Button PlusButton;
    sf::Texture PictureTexture;
    bool IsPresent;

    ItemTile() = default;

    ItemTile(const float dx, const float dy, const uint64_t itemID, const std::string& itemName, const uint32_t maxCnt, const std::string& img, const bool showCnt) {
        X = 0;
        Y = 0;
        Dx = dx;
        Dy = dy;
        ItemID = itemID;
        ItemName = itemName;
        CurCnt = 0;
        MaxCnt = maxCnt;
        ShowCnt = showCnt;
        MinusButton = Button(0, 0, 0.25 * Dx, 0.2 * Dy, "-");
        PlusButton = Button(0, 0, 0.25 * Dx, 0.2 * Dy, "+");

        PictureTexture.loadFromMemory(img.c_str(), img.size());

        IsPresent = false;
    }

    void UpdateButtonsPositions() {
        MinusButton.X = X + 0.1 * Dx;
        MinusButton.Y = Y + 0.7 * Dy;
        PlusButton.X = X + 0.65 * Dx;
        PlusButton.Y = Y + 0.7 * Dy;
    }

    void SetPosition(float x, float y) {
        X = x;
        Y = y;
        UpdateButtonsPositions();
    }

    void Draw(sf::RenderWindow& window) {
        sf::RectangleShape rectangle(sf::Vector2f(Dx, Dy));
        rectangle.setPosition(X, Y);
        rectangle.setFillColor(sf::Color::White);

        sf::Text nameText;
        nameText.setFont(NFont::font);
        nameText.setString(ItemName);
        nameText.setCharacterSize(18);
        nameText.setCharacterSize(std::min(1.0, Dx / nameText.getLocalBounds().width * 0.9) * 18.0);
        nameText.setFillColor(sf::Color::Black);
        nameText.setPosition(X + 0.5 * Dx - 0.5 * nameText.getLocalBounds().width, Y + 0.54 * Dy);

        sf::Text cntText;
        cntText.setFont(NFont::font);
        cntText.setString((ShowCnt ? std::to_string(CurCnt) + " / " : "") + std::to_string(MaxCnt));
        cntText.setCharacterSize(18);
        cntText.setCharacterSize(std::min(1.0, 0.25 * Dx / cntText.getLocalBounds().width) * 18.0);
        cntText.setFillColor(sf::Color::Black);
        cntText.setPosition(X + 0.5 * Dx - 0.5 * cntText.getLocalBounds().width, Y + 0.8 * Dy - 0.5 * cntText.getLocalBounds().height);

        sf::Sprite pictureSprite;
        pictureSprite.setTexture(PictureTexture);
        float pictureHeight = pictureSprite.getLocalBounds().height;
        float pictureWidth = pictureSprite.getLocalBounds().width;
        float scale = std::min(0.5f * Dy / pictureHeight, Dx / pictureWidth);
        pictureSprite.setPosition(X + 0.5 * Dx - 0.5 * pictureWidth * scale, Y + 5.f);
        pictureSprite.scale(scale, scale);
        
        window.draw(rectangle);
        window.draw(nameText);
        window.draw(cntText);
        window.draw(pictureSprite);
        MinusButton.Draw(window);
        PlusButton.Draw(window);
    }
};

