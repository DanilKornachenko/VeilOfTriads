#include "FieldOfGems.h"

#include <algorithm>
#include <random>
#include <vector>

FieldOfGems::FieldOfGems() : field(10, std::vector<char>(10, '.')) {
  generateStartField();
  processMatches();
}

const std::vector<std::vector<char>>& FieldOfGems::readField() { return field; }

void FieldOfGems::click(int x, int y) { field[x][y] = FieldOfGems::red; }

void FieldOfGems::swaped(int row1, int col1, int row2, int col2) {
  if (row1 < 0 || row1 >= (int)field.size() || col1 < 0 ||
      col1 >= (int)field[0].size() || row2 < 0 || row2 >= (int)field.size() ||
      col2 < 0 || col2 >= (int)field[0].size())
    return;
  std::swap(field[row1][col1], field[row2][col2]);
}

int FieldOfGems::genRandomGemNum() {
  static std::random_device rd;

  static std::mt19937 gen(rd());

  static std::uniform_int_distribution<int> dist(0, 5);

  return dist(gen);
}

void FieldOfGems::generateStartField() {
  std::vector<gem> randomed_gems = {yellow, red, purple, orange, green, blue};
  /*
  field = {
      {yellow, red, red, orange, green, blue, yellow, red, purple, orange},
      {red, red, orange, red, red, yellow, red, purple, orange, green},
      {purple, orange, green, blue, yellow, red, purple, orange, green, blue},
      {orange, green, blue, yellow, red, purple, orange, green, blue, red},
      {green, blue, yellow, red, purple, orange, green, blue, red, purple},
      {orange, yellow, red, purple, orange, green, blue, red, purple, orange},
      {orange, red, purple, orange, green, blue, red, purple, orange, green},
      {red, orange, orange, green, blue, red, purple, orange, green, blue},
      {orange, orange, green, blue, red, purple, orange, green, blue, yellow},
      {orange, green, blue, red, purple, orange, green, blue, yellow, red}};
  */
  for (auto& sub_field : field) {
    for (auto& cell : sub_field) {
      int idx = genRandomGemNum();
      cell = randomed_gems[idx];
    }
  }
}

bool FieldOfGems::findMatches(std::vector<std::vector<bool>>& outMatches,
                              int triggerRow, int triggerCol) {
  int rows = (int)field.size();
  int cols = (int)field[0].size();
  outMatches.assign(rows, std::vector<bool>(cols, false));
  bool found = false;

  // Горизонтальные совпадения
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols - 2; ++c) {
      char val = field[r][c];
      if (val == nothing) continue;
      int len = 1;
      while (c + len < cols && getBaseGem(field[r][c + len]) == getBaseGem(val))
        ++len;
      if (len >= 3) {
        for (int i = 0; i < len; ++i) outMatches[r][c + i] = true;

        // бонус
        if (len >= 5) {
          int bonusCol = c + len - 1;

          if (triggerRow == r && triggerCol >= c && triggerCol < c + len) {
            bonusCol = triggerCol;
          }

          outMatches[r][bonusCol] = false;

          field[r][bonusCol] = makeHorizontalLightning(field[r][bonusCol]);
        }

        found = true;
      }
      c += len - 1;
    }
  }

  // Вертикальные совпадения
  for (int c = 0; c < cols; ++c) {
    for (int r = 0; r < rows - 2; ++r) {
      char val = field[r][c];
      if (val == nothing) continue;
      int len = 1;
      while (r + len < rows && getBaseGem(field[r + len][c]) == getBaseGem(val))
        ++len;
      if (len >= 3) {
        for (int i = 0; i < len; ++i) outMatches[r + i][c] = true;

        if (len >= 5) {
          int bonusRow = r + len - 1;

          if (triggerCol == c && triggerRow >= r && triggerRow < r + len) {
            bonusRow = triggerRow;
          }

          outMatches[bonusRow][c] = false;

          field[bonusRow][c] = makeVerticalLightning(field[bonusRow][c]);
        }

        found = true;
      }
      r += len - 1;
    }
  }
  return found;
}

void FieldOfGems::removeMatches(const std::vector<std::vector<bool>>& matches) {
  int rows = field.size();
  int cols = field[0].size();

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      if (matches[r][c]) {
        field[r][c] = nothing;
      }
    }
  }
}

void FieldOfGems::applyGravity() {
  int rows = (int)field.size();
  int cols = (int)field[0].size();

  for (int c = 0; c < cols; ++c) {
    // собираем все ненулевые элементы к началу столбца (row = 0)
    int writeRow = 0;
    for (int r = 0; r < rows; ++r) {
      if (field[r][c] != nothing) {
        field[writeRow][c] = field[r][c];
        ++writeRow;
      }
    }
    // Остальные заполняем пустотой
    for (int r = writeRow; r < rows; ++r) field[r][c] = nothing;
  }
}

void FieldOfGems::setGem(int row, int col, char gem) {
  if (row >= 0 && row < (int)field.size() && col >= 0 &&
      col < (int)field[0].size())
    field[row][col] = gem;
}

void FieldOfGems::fillEmptyCells() {
  std::vector<gem> gems = {yellow, red, purple, orange, green, blue};
  for (auto& row : field) {
    for (auto& cell : row) {
      if (cell == nothing) {
        int idx = genRandomGemNum();
        cell = gems[idx];
      }
    }
  }
}

bool FieldOfGems::processMatches() {
  std::vector<std::vector<bool>> matches;
  bool anyRemoved = false;
  while (findMatches(matches)) {
    anyRemoved = true;
    removeMatches(matches);
    applyGravity();
    fillEmptyCells();
  }
  return anyRemoved;
}

bool FieldOfGems::isHorizontalLightning(char gem) const {
  return gem >= 'A' && gem <= 'Z';
}

bool FieldOfGems::isVerticalLightning(char gem) const {
  return gem >= '1' && gem <= '6';
}

bool FieldOfGems::isLightning(char gem) const {
  return isHorizontalLightning(gem) || isVerticalLightning(gem);
}

char FieldOfGems::getBaseGem(char gem) const {
  switch (gem) {
    case yellow:
    case yellow_h:
    case yellow_v:
      return yellow;

    case red:
    case red_h:
    case red_v:
      return red;

    case purple:
    case purple_h:
    case purple_v:
      return purple;

    case orange:
    case orange_h:
    case orange_v:
      return orange;

    case green:
    case green_h:
    case green_v:
      return green;

    case blue:
    case blue_h:
    case blue_v:
      return blue;
  }

  return gem;
}

char FieldOfGems::makeHorizontalLightning(char gem) const {
  switch (getBaseGem(gem)) {
    case yellow:
      return yellow_h;
    case red:
      return red_h;
    case purple:
      return purple_h;
    case orange:
      return orange_h;
    case green:
      return green_h;
    case blue:
      return blue_h;
  }

  return gem;
}

char FieldOfGems::makeVerticalLightning(char gem) const {
  switch (getBaseGem(gem)) {
    case yellow:
      return yellow_v;
    case red:
      return red_v;
    case purple:
      return purple_v;
    case orange:
      return orange_v;
    case green:
      return green_v;
    case blue:
      return blue_v;
  }

  return gem;
}

std::vector<std::vector<bool>> FieldOfGems::expandLightningMatches(
    const std::vector<std::vector<bool>>& matches) const {
  int rows = field.size();
  int cols = field[0].size();

  std::vector<std::vector<bool>> expanded = matches;

  bool changed = true;

  while (changed) {
    changed = false;

    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < cols; ++c) {
        if (!expanded[r][c]) continue;

        char gem = field[r][c];

        if (isHorizontalLightning(gem)) {
          for (int x = 0; x < cols; ++x) {
            if (!expanded[r][x]) {
              expanded[r][x] = true;
              changed = true;
            }
          }
        }

        if (isVerticalLightning(gem)) {
          for (int y = 0; y < rows; ++y) {
            if (!expanded[y][c]) {
              expanded[y][c] = true;
              changed = true;
            }
          }
        }
      }
    }
  }

  return expanded;
}
