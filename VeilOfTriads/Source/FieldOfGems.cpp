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
