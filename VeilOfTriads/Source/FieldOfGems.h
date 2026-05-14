#pragma once
#include <vector>

class FieldOfGems {
 private:
  std::vector<std::vector<char>> field;
  void generateStartField();

 public:
  enum gem {
    yellow = 'y',
    red = 'r',
    purple = 'p',
    orange = 'o',
    green = 'g',
    blue = 'b',

    yellow_h = 'Y',
    red_h = 'R',
    purple_h = 'P',
    orange_h = 'O',
    green_h = 'G',
    blue_h = 'B',

    yellow_v = '1',
    red_v = '2',
    purple_v = '3',
    orange_v = '4',
    green_v = '5',
    blue_v = '6',

    nothing = '.'
  };

  FieldOfGems();
  const std::vector<std::vector<char>>& readField();
  void click(int x, int y);
  void swaped(int row1, int col1, int row2, int col2);
  bool processMatches();
  void fillEmptyCells();
  int genRandomGemNum();
  void setGem(int row, int col, char gem);
  bool findMatches(std::vector<std::vector<bool>>& outMatches,
                   int bonusRow = -1, int bonusCol = -1);
  void removeMatches(const std::vector<std::vector<bool>>& matches);
  void applyGravity();
  bool isLightning(char gem) const;
  bool isHorizontalLightning(char gem) const;
  bool isVerticalLightning(char gem) const;
  char makeHorizontalLightning(char gem) const;
  char makeVerticalLightning(char gem) const;
  char getBaseGem(char gem) const;
  std::vector<std::vector<bool>> expandLightningMatches(
      const std::vector<std::vector<bool>>& matches) const;
};
