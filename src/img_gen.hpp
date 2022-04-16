#ifdef IMAGEMAGICK_AVAILABLE
  #include <Magick++.h>
  #include <common.hpp>
  #include <string>

  void generateUnitImage(std::string fileName, std::vector<float> color);
  void generateUnitImages(int num, std::vector<std::vector<float>> colors, uint seed);
#endif
