#pragma once
#include <vector>

class FieldOfGems
{
  private:
    std::vector<std::vector<char>> field;
    void generateStartField();
    int genRandomGemNum();

  public:
    enum gem
    {
      yellow = 'y',
      red = 'r',
      purple = 'p',
      orange = 'o',
      green = 'g',
      blue = 'b',
      nothing = '.'
    };

    FieldOfGems();
    const std::vector<std::vector<char>>& readField();
    void click(int x, int y);
};
