#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <windows.h>


HDC initializeMemoryDC(HDC hdcScreen, HBITMAP& hBitmap, int width, int height) {
    HDC hdcMemory = CreateCompatibleDC(hdcScreen);
    hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMemory, hBitmap);
    return hdcMemory;
}


sf::Texture captureScreen(HDC hdcScreen, HDC hdcMemory, HBITMAP hBitmap, int screenWidth, int screenHeight) {
    // Copy screen to bitmap
    BitBlt(hdcMemory, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

    sf::Uint8* buffer = new sf::Uint8[screenWidth * screenHeight * 4];  // 4 bytes per pixel (RGBA)

    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), screenWidth, -screenHeight, 1, 32, BI_RGB };
    GetDIBits(hdcMemory, hBitmap, 0, screenHeight, buffer, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    sf::Image screenshot;
    screenshot.create(screenWidth, screenHeight, buffer);
    delete[] buffer;

    sf::Texture texture;

    texture.loadFromImage(screenshot);
    return texture;
}

// Memory management
void releaseGDIResources(HDC hdcScreen, HDC hdcMemory, HBITMAP hBitmap) {
    DeleteObject(hBitmap);
    DeleteDC(hdcMemory);
    ReleaseDC(GetDesktopWindow(), hdcScreen);
}

//Main loop
void runScreenCaptureLoop() {
    sf::RenderWindow window(sf::VideoMode(600, 600), "Screen Capture");

    sf::Clock clock;
    const float targetFrameRate = 1.0f / 10.0f;

    HWND hwndDesktop = GetDesktopWindow();
    HDC hdcScreen = GetDC(hwndDesktop);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);


    HBITMAP hBitmap;
    HDC hdcMemory = initializeMemoryDC(hdcScreen, hBitmap, screenWidth, screenHeight);

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
            sf::Texture screenTexture = captureScreen(hdcScreen, hdcMemory, hBitmap, screenWidth, screenHeight);
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
