#include "FieldOfGems.h"
#include <random>
#include <vector>

FieldOfGems::FieldOfGems() :
  field(20, std::vector<char>(10, '.'))
{
  generateStartField();
}

const std::vector<std::vector<char>>& FieldOfGems::readField()
{
  return field;
}

void FieldOfGems::click(int x, int y)
{
  field[x][y] = FieldOfGems::red;
}

void FieldOfGems::swaped(int row1, int col1, int row2, int col2)
{
  if (row1 < 0 || row1 >= (int)field.size() || col1 < 0 || col1 >= (int)field[0].size() ||
      row2 < 0 || row2 >= (int)field.size() || col2 < 0 || col2 >= (int)field[0].size())
    return;
  std::swap(field[row1][col1], field[row2][col2]);
}

int FieldOfGems::genRandomGemNum()
{
  static std::random_device rd;

  static std::mt19937 gen(rd());

  static std::uniform_int_distribution<int> dist(0, 5);

  return dist(gen);
}

void FieldOfGems::generateStartField()
{
  std::vector<gem> randomed_gems = {yellow, red, purple, orange, green, blue};

  for (auto& sub_field : field)
  {
    for (auto& cell : sub_field)
    {
      int idx = genRandomGemNum();
      cell = randomed_gems[idx];
    }
  }
}
