#ifdef IMAGEMAGICK_AVAILABLE
  #include "img_gen.hpp"
  #include "game_components.hpp"
  #include "Magick++/Image.h"
  #include "MagickCore/magick-type.h"
  #include <iostream>

  using namespace Magick;

  // Draws a rectangle of color on img at width w and height h with top left at (x, y).
  void drawRect(Image& img, Color color, int x, int y, int w, int h) {
    for(int i = x; i < x+w; i++) {
      for(int j = y; j < y+h; j++) {
        img.pixelColor(i, j, color);
      }
    }
  }

  void generateUnitImage(std::string fileName, std::vector<float> color) {
    // Start with a fully transparent image
    Image ally_image(Geometry(50, 50), Color(0, 0, 0, 0));
    Image enemy_image(Geometry(50, 50), Color(0, 0, 0, 0));
    // Opaque unit color
    Color ally_unit_color(color[0], color[1], color[2], QuantumRange);
    Color enemy_unit_color(0, 0, 0, QuantumRange);
    // Build our image
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 5; j++) {
        // If rand is even we draw this pixel (a 10x10 rect on our 50x50 image)
        if (rand() % 2) {
          // Draw center column
          if (i == 0) {
            drawRect(ally_image, ally_unit_color, 20, j*10, 10, 10);
            drawRect(enemy_image, enemy_unit_color, 20, j*10, 10, 10);
          }
          // Draw border columns
          else if (i == 1) {
            drawRect(ally_image, ally_unit_color, 0, j*10, 10, 10);
            drawRect(enemy_image, enemy_unit_color, 0, j*10, 10, 10);
            drawRect(ally_image, ally_unit_color, 40, j*10, 10, 10);
            drawRect(enemy_image, enemy_unit_color, 40, j*10, 10, 10);
          }
          // Draw inner columns
          else {
            drawRect(ally_image, ally_unit_color, 10, j*10, 10, 10);
            drawRect(enemy_image, enemy_unit_color, 10, j*10, 10, 10);
            drawRect(ally_image, ally_unit_color, 30, j*10, 10, 10);
            drawRect(enemy_image, enemy_unit_color, 30, j*10, 10, 10);
          }
        }
      }
    }

    std::cout << "writing img" << std::endl;
    ally_image.magick("png");
    enemy_image.magick("png");
    std::string path(__FILE__);
    // Janky path by taking substring without the filename since we don't have boost but should work
    ally_image.write(path.substr(0, path.size()-11) + "../data/textures/ally_" + fileName + ".png");
    enemy_image.write(path.substr(0, path.size()-11) + "../data/textures/enemy_" + fileName + ".png");
  }

  void generateUnitImages(int num, std::vector<std::vector<float>> colors, uint seed) {
    srand(seed);
    std::vector<std::string> units = getUnits();
    for (int i = 0; i < num; i++) {
      std::cout << QuantumRange << std::endl;
      generateUnitImage(units[i], colors[i]);
    }
  }
#endif
