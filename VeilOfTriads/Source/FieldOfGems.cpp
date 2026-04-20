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

  for (auto& sub_field : field) {
    for (auto& cell : sub_field) {
      int idx = genRandomGemNum();
      cell = randomed_gems[idx];
    }
  }
}

bool FieldOfGems::findMatches(
    std::vector<std::vector<bool>>& outMatches) const {
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
      while (c + len < cols && field[r][c + len] == val) ++len;
      if (len >= 3) {
        for (int i = 0; i < len; ++i) outMatches[r][c + i] = true;
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
      while (r + len < rows && field[r + len][c] == val) ++len;
      if (len >= 3) {
        for (int i = 0; i < len; ++i) outMatches[r + i][c] = true;
        found = true;
      }
      r += len - 1;
    }
  }
  return found;
}

void FieldOfGems::removeMatches(const std::vector<std::vector<bool>>& matches) {
  for (size_t r = 0; r < field.size(); ++r)
    for (size_t c = 0; c < field[r].size(); ++c)
      if (matches[r][c]) field[r][c] = nothing;
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
