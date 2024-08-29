#include <sfml/Graphics.hpp>
#include <sfml/Window.hpp>
#include <sfml/System.hpp>
#include <windows.h>
#include <iostream>
#include "main.h"



HDC initializeMemoryDC(HDC hdcScreen, HBITMAP& hBitmap, int width, int height) {
    HDC hdcMemory = CreateCompatibleDC(hdcScreen);
    hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMemory, hBitmap);
    return hdcMemory;
}


sf::Texture captureScreen(HDC hdcScreen, HDC hdcMemory, HBITMAP hBitmap, const SelectionCoordinates & selectionCoordinates) {

    int width = selectionCoordinates.width;
    int height = selectionCoordinates.height;
    //Copy to bitmap
    BitBlt(hdcMemory, 0, 0, width, height, hdcScreen, selectionCoordinates.anchorPoints[0].x, selectionCoordinates.anchorPoints[0].y, SRCCOPY);

    //Buffer to hold data
    sf::Uint8* buffer = new sf::Uint8[width * height * 4];
    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB };
    GetDIBits(hdcMemory, hBitmap, 0, height, buffer, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    sf::Image screenshot;
    screenshot.create(width, height, buffer);     
    //Memory offloading
    delete[] buffer;

    //Convert to texture
    sf::Texture texture;
    texture.loadFromImage(screenshot);
    return texture;
}

// Function to release GDI resources
void releaseGDIResources(HDC hdcScreen, HDC hdcMemory, HBITMAP hBitmap) {
    DeleteObject(hBitmap);
    DeleteDC(hdcMemory);
    ReleaseDC(GetDesktopWindow(), hdcScreen);
}


struct SelectionCoordinates {
    sf::Vector2i anchorPoints[4];
    int width;
    int height;
};

sf::Vector2i* selectCaptureArea(){
    sf::RenderWindow selectionWindow(sf::VideoMode(800, 600), "Select capture region", sf::Style::None);
    selectionWindow.setMouseCursorVisible(true);

    SelectionCoordinates coordinates;
    bool selecting = false;
    bool firstPointSet = false;
    sf::Vector2i startPoint;

    while (selectionWindow.isOpen()) {
        sf::Event event;
        //Using mouse events to capture screen section
        while (selectionWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                selectionWindow.close();
                return coordinates;
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                //Selection box setup
                selecting = true;
                startPoint = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
                coordinates.anchorPoints[0] = startPoint; //"Top left"
                firstPointSet = true;
            }else{
                coordinates.anchorPoints[1] = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);//"Bottom right"
                selecting = false;
                coordinates.anchorPoints[2] = sf::Vector2i(coordinates.anchorPoints[0].x, coordinates.anchorPoints[1].y);//"Bottom left"
                coordinates.anchorPoints[3] = sf::Vector2i(coordinates.anchorPoints[1].x, coordinates.anchorPoints[0].y);//"Top Right"
                //Dimensions
                coordinates.width = coordinates.anchorPoints[1].x - coordinates.anchorPoints[0].x;
                coordinates.height = coordinates.anchorPoints[1].y - coordinates.anchorPoints[0].y;

                selectionWindow.close();


            }
        }
        //Draws square around the selection zone
        if (selecting) {
            sf::Vector2i currentMousePos = sf::Mouse::getPosition(selectionWindow);
            sf::RectangleShape rect(sf::Vector2f(currentMousePos.x - startPoint.x, currentMousePos.y - startPoint.y));
            rect.setPosition(startPoint.x, startPoint.y);
            rect.setFillColor(sf::Color(0, 0, 255, 50));
            rect.setOutlineColor(sf::Color::Blue);
            rect.setOutlineThickness(1);

            selectionWindow.clear();
            selectionWindow.draw(rect);
            selectionWindow.display();
        }
    }

    return coordinates;
}
//Main loop
void runScreenCaptureLoop() {

    SelectionCoordinates selectionCoordinates = selectCaptureArea();
    if (selectionCoordinates.width <= 0 || selectionCoordinates.height <= 0) {
        std::cerr << "Selection Invalid!" << std::endl;
        return;
    }


    //Create window
    sf::RenderWindow window(sf::VideoMode(selectionCoordinates.width, selectionCoordinates.height), "Translator");

    //Clock for frame rate
    sf::Clock clock;
    const float targetFrameRate = 1.0f / 10.0f;

    HWND hwndDesktop = GetDesktopWindow();
    HDC hdcScreen = GetDC(hwndDesktop);
    HBITMAP hBitmap;
    HDC hdcMemory = initializeMemoryDC(hdcScreen, hBitmap, selectionCoordinates.width, selectionCoordinates.height);

    //Sprite for display
    sf::Sprite screenSprite;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Limit the frame rate to 10 FPS
        if (clock.getElapsedTime().asSeconds() >= targetFrameRate) {
            sf::Texture screenTexture = captureScreen(hdcScreen, hdcMemory, hBitmap, selectionCoordinates);
            screenSprite.setTexture(screenTexture);
            window.clear();
            window.draw(screenSprite);
            window.display();
            clock.restart();
        }
    }
    releaseGDIResources(hdcScreen, hdcMemory, hBitmap);
}


int main() {
    runScreenCaptureLoop();
    return 0;
}


