> [!NOTE]
> **Libraries**
> SFML is located in `E:\SFML-2.6.1`
> 
> 
> **Additional Dependencies**
> sfml-graphics.lib;sfml-window.lib;sfml-system.lib;gdi32.lib;user32.lib;
> 
> **Debugging**
> sfml-graphics-d.lib;sfml-window-d.lib;sfml-system-d.lib;gdi32.lib;user32.lib;

## **1. Screen Recording**

- ~~Lock screen recording at 10fps to reduce CPU strain while program is running~~
	- Tweak the fps once word recognition has been implemented
- [Release](https://stackoverflow.com/questions/33497466/how-to-free-gdi-resources-correctly) unused GDI resources as they [don't release automatically](https://www.deleaker.com/blog/2021/12/16/gdi-leaks-how-to-identify-and-fix-them/)
- Create a duplicate of the original 60fps image which is intended for processing
	- Lock duplicate image at 10fps to begin with and update as necessary
- Image processing!
- Accessibility features for colour blindness
- Text to speech?

**[SFML](https://www.sfml-dev.org/)**
 "SFML is a simple, fast, cross-platform and object-oriented multimedia API. It provides access to windowing, graphics, audio and network."

*Capture*
![[Program_Capture.png]]
*Original*
![[Genuine_Capture.png]]

==Colours are being read as RGBA but the original format might be different?==
~~The actual colour values shouldn't matter themselves as image processing will be performed later to improve accuracy of word recognition~~. Creating a high quality second window to view translated text from is the original purpose of this program. Capture one display at full 60fps while creating a copy of a second window which is intended for processing use. The first display is meant to be for viewing therefore it must retain correct colouring and good quality.

==How will a duplicate image affect CPU strain?==

Implement selector feature to narrow down screen capture to a section of the screen, this will reduce the strain on Tesseract later down the line as it will have a lower area to search through.

Eventually turn selector graphic into something like this:
![[Screen_Recording_Mockup_Ange.png]]

**Area selection logic**
![[Screen_Capture_Logic.png]]
Use 4 anchor points to draw out a square on the screen
[ABS function](https://support.microsoft.com/en-gb/office/abs-function-3420200f-5628-4e8c-99da-c99d7c87713c) to transform any number into a positive value
```cpp
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
```
Anchors are labelled with quotation marks due to the actual direction of the mouse movement being unknown. 

## **2. Word Recognition**

**[Tesseract 4](https://github.com/tesseract-ocr/tesseract)**

"Tesseract 4 adds a new neural net (LSTM) based [OCR engine](https://en.wikipedia.org/wiki/Optical_character_recognition) which is focused on line recognition, but also still supports the legacy Tesseract OCR engine of Tesseract 3 which works by recognizing character patterns."

Tesseract OCR is also capable of recognising a wide variety of languages and allows the user to specify a language. According to some posts online it requires a lot of image pre-processing.
1. Upsize/Downsize input image to 300dpi
2. ~~Remove colour from the image. Grayscale is good~~
3. Cut out unnecessary areas of the screen, this should be done using the area selection
4. Tesseract 4 produces significantly better results in word recognition, so no grayscale will be required

Some colours can [look the same](https://en.wikipedia.org/wiki/CIELAB_color_space) in grayscale. Just applying grayscale to an image may reduce accuracy or in worst case scenario may bury text altogether.

*Original Colours*
![[Red_Original.jpg]]
*Grayscale*
![[Red_Grayscale.jpg]]

==What can be done to these colours to make them more distinct in grayscale?==

*Grayscale with contrast*
![[Red_Grayscale_+_Contrast.png]]

Raising the contrast by 70 and then applying grayscale has made a significant improvement in how visible the different colours are, however raising contrast in one area can create the same issue with different colours in another area. The best move would be to leave the images as is once converting to grayscale and hope nobody made poor graphical decisions by putting text on a little to no contrast background.

Tesseract 4 [does not require a grayscale image](https://limitlessdatascience.wordpress.com/2019/07/31/tesseract-3-0-and-4-0-implementation-and-output-comparison/) to help with faster processing, therefore I am just going to leave the original image as it is.

==Does Tesseract have the ability to process 10 images a second?==

Create a gate that only allows images to be processed by Tesseract once they've changed. This should reduce the amount of work Tesseract has to do while still actively recognising new words.
