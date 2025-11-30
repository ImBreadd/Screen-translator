#include <SFML/Graphics.hpp>
#include <windows.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream>

HDC initializeMemoryDC(HDC hdcScreen, HBITMAP& hBitmap, int width, int height) {
    HDC hdcMemory = CreateCompatibleDC(hdcScreen);
    hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMemory, hBitmap);
    return hdcMemory;
}

sf::Image captureScreenImage(HDC hdcScreen, HDC hdcMemory, HBITMAP hBitmap,
    int screenWidth, int screenHeight) {
    BitBlt(hdcMemory, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

    sf::Uint8* buffer = new sf::Uint8[screenWidth * screenHeight * 4];
    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), screenWidth, -screenHeight, 1, 32, BI_RGB };
    GetDIBits(hdcMemory, hBitmap, 0, screenHeight, buffer, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    sf::Image screenshot;
    screenshot.create(screenWidth, screenHeight, buffer);
    delete[] buffer;
    return screenshot;
}

void releaseGDIResources(HDC hdcScreen, HDC hdcMemory, HBITMAP hBitmap) {
    DeleteObject(hBitmap);
    DeleteDC(hdcMemory);
    ReleaseDC(GetDesktopWindow(), hdcScreen);
}

// Convert SFML image to Leptonica Pix
Pix* sfImageToPix(const sf::Image& img) {
    unsigned w = img.getSize().x;
    unsigned h = img.getSize().y;
    Pix* pix = pixCreate(w, h, 32);
    for (unsigned y = 0; y < h; ++y) {
        for (unsigned x = 0; x < w; ++x) {
            sf::Color c = img.getPixel(x, y);
            pixSetRGBPixel(pix, x, y, c.r, c.g, c.b);
        }
    }
    return pix;
}

void runScreenCaptureLoop() {
    HWND hwndDesktop = GetDesktopWindow();
    HDC hdcScreen = GetDC(hwndDesktop);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HBITMAP hBitmap;
    HDC hdcMemory = initializeMemoryDC(hdcScreen, hBitmap, screenWidth, screenHeight);

    sf::Clock clock;
    const float targetFrameRate = 1.0f / 5.0f; // 5 FPS

    while (true) {
        if (clock.getElapsedTime().asSeconds() >= targetFrameRate) {
            sf::Image screenshot = captureScreenImage(hdcScreen, hdcMemory, hBitmap,
                screenWidth, screenHeight);

            // Run OCR
            Pix* pix = sfImageToPix(screenshot);
            tesseract::TessBaseAPI ocr;
            ocr.Init(NULL, "eng"); // load English traineddata
            ocr.SetImage(pix);
            char* outText = ocr.GetUTF8Text();
            std::cout << "OCR Output:\n" << outText << std::endl;

            delete[] outText;
            pixDestroy(&pix);
            ocr.End();

            clock.restart();
        }
    }

    releaseGDIResources(hdcScreen, hdcMemory, hBitmap);
}

int main() {
    runScreenCaptureLoop();
    return 0;
}
