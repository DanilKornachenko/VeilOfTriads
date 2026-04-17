/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 Copyright (c) 2019-present Axmol Engine contributors (see AUTHORS.md).

 https://axmol.dev/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "GameScene.h"

#include <cmath>

#include "2d/ActionEase.h"
#include "2d/ActionInterval.h"
#include "Director.h"
#include "FieldOfGems.h"

using namespace ax;

static int s_sceneID = 1000;

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename) {
  printf("Error while loading: %s\n", filename);
  printf(
      "Depending on how you compiled you might have to add 'Content/' in front "
      "of filenames in "
      "GameScene.cpp\n");
}

// on "init" you need to initialize your instance
bool GameScene::init() {
  //////////////////////////////
  // 1. super init first
  if (!Scene::init()) {
    return false;
  }

  auto visibleSize = _director->getVisibleSize();
  auto origin = _director->getVisibleOrigin();
  auto safeArea = _director->getSafeAreaRect();
  auto safeOrigin = safeArea.origin;

  /////////////////////////////
  // 2. add a menu item with "X" image, which is clicked to quit the program
  //    you may modify it.

  // add a "close" icon to exit the progress. it's an autorelease object
  auto closeItem =
      MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
                            AX_CALLBACK_1(GameScene::menuCloseCallback, this));

  if (closeItem == nullptr || closeItem->getContentSize().width <= 0 ||
      closeItem->getContentSize().height <= 0) {
    problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
  } else {
    float x = safeOrigin.x + safeArea.size.width -
              closeItem->getContentSize().width / 2;
    float y = safeOrigin.y + closeItem->getContentSize().height / 2;
    closeItem->setPosition(Vec2(x, y));
  }

  // create menu, it's an autorelease object
  auto menu = Menu::create(closeItem, NULL);
  menu->setPosition(Vec2::ZERO);
  this->addChild(menu, 1);

  /////////////////////////////
  // Draw Field
  /*
  int gridCols = 10;
  int gridRows = 20;

  float cellSize = (visibleSize.height * 0.9f) / gridRows;

  float gridWidth = gridCols * cellSize;
  float gridHeight = gridRows * cellSize;
  float startX = origin.x + (visibleSize.width - gridWidth) * 0.5f;
  float startY = origin.y + (visibleSize.height - gridHeight) * 0.5f;

  auto drawNode = DrawNode::create();
  drawNode->setPosition(Vec2::ZERO); // Позиция узла в его родительской системе
  координат

  Color4F whiteColor(1.0f, 1.0f, 1.0f, 1.0f); // RGBA

  for (int row = 0; row < gridRows; ++row) {
      for (int col = 0; col < gridCols; ++col) {
          float x = startX + col * cellSize;
          float y = startY + row * cellSize;
          Vec2 origin(x, y);
          Vec2 destination(x + cellSize, y + cellSize);
          drawNode->drawRect(origin, destination, whiteColor);
      }
  }

  this->addChild(drawNode);
  */
  setupGrid();
  /////////////////////////////
  // 3. add your codes below...

  // Some templates (uncomment what you  need)
  _touchListener = EventListenerTouchAllAtOnce::create();
  _touchListener->onTouchesBegan =
      AX_CALLBACK_2(GameScene::onTouchesBegan, this);
  _touchListener->onTouchesMoved =
      AX_CALLBACK_2(GameScene::onTouchesMoved, this);
  _touchListener->onTouchesEnded =
      AX_CALLBACK_2(GameScene::onTouchesEnded, this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener,
                                                           this);

  _mouseListener = EventListenerMouse::create();
  _mouseListener->onMouseMove = AX_CALLBACK_1(GameScene::onMouseMove, this);
  _mouseListener->onMouseUp = AX_CALLBACK_1(GameScene::onMouseUp, this);
  _mouseListener->onMouseDown = AX_CALLBACK_1(GameScene::onMouseDown, this);
  _mouseListener->onMouseScroll = AX_CALLBACK_1(GameScene::onMouseScroll, this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(_mouseListener,
                                                           this);

  _keyboardListener = EventListenerKeyboard::create();
  _keyboardListener->onKeyPressed =
      AX_CALLBACK_2(GameScene::onKeyPressed, this);
  _keyboardListener->onKeyReleased =
      AX_CALLBACK_2(GameScene::onKeyReleased, this);
  _eventDispatcher->addEventListenerWithFixedPriority(_keyboardListener, 11);

  // add a label shows "Hello World"
  // create and initialize a label

  auto label = Label::createWithTTF("Score : 0", "fonts/PixelGameFont.ttf", 24);
  if (label == nullptr) {
    problemLoading("'fonts/PixelGameFont.ttf'");
  } else {
    // position the label on the center of the screen
    label->setPosition(
        Vec2(origin.x + visibleSize.width / 2,
             origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);
  }

  // scheduleUpdate() is required to ensure update(float) is called on every
  // loop
  scheduleUpdate();

  return true;
}

Vec2 GameScene::touchToGridIndex(const Vec2& touchLocation) {
  if (touchLocation.x < _gridStartPos.x ||
      touchLocation.x > _gridStartPos.x + _gridCols * _cellSize ||
      touchLocation.y < _gridStartPos.y ||
      touchLocation.y > _gridStartPos.y + _gridRows * _cellSize) {
    return Vec2(-1, -1);  // За пределами
  }

  int col = static_cast<int>((touchLocation.x - _gridStartPos.x) / _cellSize);
  int row = static_cast<int>((touchLocation.y - _gridStartPos.y) / _cellSize);
  return Vec2(col, row);
}

void GameScene::onTouchesBegan(const std::vector<ax::Touch*>& touches,
                               ax::Event* event) {
  for (auto&& t : touches) {
    Vec2 location = t->getLocation();
    Vec2 index = touchToGridIndex(location);
    if (index.x >= 0 && index.y >= 0) {
      int col = static_cast<int>(index.x);
      int row = static_cast<int>(index.y);
      handleCellClick(row, col);
      AXLOGD("Touch at grid: col=%d, row=%d", col, row);
    }
  }
}

void GameScene::onTouchesMoved(const std::vector<ax::Touch*>& touches,
                               ax::Event* event) {
  for (auto&& t : touches) {
    // AXLOGD("onTouchesMoved detected, X:{}  Y:{}", t->getLocation().x,
    // t->getLocation().y);
  }
}

void GameScene::onTouchesEnded(const std::vector<ax::Touch*>& touches,
                               ax::Event* event) {
  for (auto&& t : touches) {
    // AXLOGD("onTouchesEnded detected, X:{}  Y:{}", t->getLocation().x,
    // t->getLocation().y);
  }
}

bool GameScene::onMouseDown(Event* event) {
  /*EventMouse* e = static_cast<EventMouse*>(event);
  Vec2 location = e->getLocationInView();
  location = _director->convertToGL(location);

  Vec2 index = touchToGridIndex(location);
  if (index.x >= 0 && index.y >= 0)
  {
    int col = static_cast<int>(index.x);
    int row = static_cast<int>(index.y);
    handleCellClick(row, col);
    AXLOGD("Touch at grid: col=%d, row=%d", col, row);
    printf("Touch at grid: col=%d, row=%d\n", col, row);
  }
  */
  return true;
}

bool GameScene::onMouseUp(Event* event) {
  EventMouse* e = static_cast<EventMouse*>(event);
  AXLOGD("onMouseUp detected, button: {}",
         static_cast<int>(e->getMouseButton()));
  return true;
}

bool GameScene::onMouseMove(Event* event) {
  EventMouse* e = static_cast<EventMouse*>(event);
  AXLOGD("onMouseMove detected, X:{}  Y:{}", e->getLocation().x,
         e->getLocation().y);
  return true;
}

bool GameScene::onMouseScroll(Event* event) {
  EventMouse* e = static_cast<EventMouse*>(event);
  // AXLOGD("onMouseScroll detected, X:{}  Y:{}", e->getScrollX(),
  // e->getScrollY());
  return true;
}

void GameScene::onKeyPressed(EventKeyboard::KeyCode code, Event* event) {
  AXLOGD("Scene: #{} onKeyPressed, keycode: {}", _sceneID,
         static_cast<int>(code));
}

void GameScene::onKeyReleased(EventKeyboard::KeyCode code, Event* event) {
  AXLOGD("onKeyReleased, keycode: {}", static_cast<int>(code));
}

void GameScene::update(float delta) {
  switch (_gameState) {
    case GameState::init: {
      _gameState = GameState::update;
      break;
    }

    case GameState::update: {
      /////////////////////////////
      // Add your codes below...like....
      //
      // UpdateJoyStick();
      // UpdatePlayer();
      // UpdatePhysics();
      // ...
      break;
    }

    case GameState::pause: {
      /////////////////////////////
      // Add your codes below...like....
      //
      // anyPauseStuff()

      break;
    }

    case GameState::menu1: {  /////////////////////////////
      // Add your codes below...like....
      //
      // UpdateMenu1();
      break;
    }

    case GameState::menu2: {  /////////////////////////////
      // Add your codes below...like....
      //
      // UpdateMenu2();
      break;
    }

    case GameState::end: {  /////////////////////////////
      // Add your codes below...like....
      //
      // CleanUpMyCrap();
      menuCloseCallback(this);
      break;
    }

  }  // switch
}

void GameScene::menuCloseCallback(ax::Object* sender) {
  // Close the axmol game scene and quit the application
  _director->end();

  /*To navigate back to native iOS screen(if present) without quitting the
   * application  ,do not use _director->end() as given above,instead trigger a
   * custom event created in RootViewController.mm as below*/

  // EventCustom customEndEvent("game_scene_close_event");
  //_eventDispatcher->dispatchEvent(&customEndEvent);
}

static std::string getGemImagePath(char gemChar) {
  std::string colorName;
  switch (gemChar) {
    case FieldOfGems::yellow:
      colorName = "yellow";
      break;
    case FieldOfGems::red:
      colorName = "red";
      break;
    case FieldOfGems::purple:
      colorName = "purple";
      break;
    case FieldOfGems::orange:
      colorName = "orange";
      break;
    case FieldOfGems::green:
      colorName = "green";
      break;
    case FieldOfGems::blue:
      colorName = "blue";
      break;
    default:
      return "";  // пустая клетка
  }
  return "res/Colored gems/" + colorName + "/gem_" + colorName + "_big_4x.png";
}

void GameScene::setupGrid() {
  auto visibleSize = _director->getVisibleSize();
  auto origin = _director->getVisibleOrigin();

  _cellSize = (visibleSize.height * 0.9f) / _gridRows;
  float gridWidth = _gridCols * _cellSize;
  float gridHeight = _gridRows * _cellSize;
  _gridStartPos.x = origin.x + (visibleSize.width - gridWidth) * 0.5f;
  _gridStartPos.y = origin.y + (visibleSize.height - gridHeight) * 0.5f;

  // Создаём контейнеры для всех спрайтов
  _gridContainer = Node::create();
  _gridContainer->setPosition(Vec2::ZERO);
  this->addChild(_gridContainer);

  // Инициализируем двумерный вектор спрайтов
  _gemSprites.resize(_gridRows);
  for (int r = 0; r < _gridRows; ++r) {
    _gemSprites[r].resize(_gridCols, nullptr);
  }

  // Первичное создание спрайтов
  redrawGrid();
}

void GameScene::redrawGrid() {
  auto field = _field.readField();

  for (int r = 0; r < _gridRows; ++r) {
    for (int c = 0; c < _gridCols; ++c) {
      char gemChar = field[r][c];
      std::string imagePath = getGemImagePath(gemChar);

      // Если спрайт не создан - создаём
      if (_gemSprites[r][c] == nullptr) {
        if (!imagePath.empty()) {
          auto sprite = Sprite::create(imagePath);
          if (sprite) {
            float x = _gridStartPos.x + c * _cellSize + _cellSize / 2;
            float y = _gridStartPos.y + r * _cellSize + _cellSize / 2;

            sprite->setPosition(Vec2(x, y));

            // Масштабируем, чтобы вписать в ячейку
            float scaleX = _cellSize / sprite->getContentSize().width;
            float scaleY = _cellSize / sprite->getContentSize().height;

            sprite->setScale(scaleX, scaleY);

            _gridContainer->addChild(sprite);
            _gemSprites[r][c] = sprite;
          }
        }
      } else {
        // Спрайт уже существует - обновляем текстуру или скрываем
        if (imagePath.empty()) {
          _gemSprites[r][c]->setVisible(false);
        } else {
          _gemSprites[r][c]->setVisible(true);
          // Если текстура изменилась, загружаем новую
          auto currentTex = _gemSprites[r][c]->getTexture();
          auto newTex =
              Director::getInstance()->getTextureCache()->addImage(imagePath);
          if (currentTex != newTex) {
            _gemSprites[r][c]->setTexture(newTex);
          }
        }
      }
    }
  }
  updateSelectionHighlight();
}

void GameScene::updateSelectionHighlight() {
  // Сначала сбрасываем цвет всех спрайтов
  for (int r = 0; r < _gridRows; ++r) {
    for (int c = 0; c < _gridCols; ++c) {
      if (_gemSprites[r][c]) {
        _gemSprites[r][c]->setColor(ax::Color3B::WHITE);
      }
    }
  }

  // Подсвечиваем выбранный, если есть
  if (_selectedRow >= 0 && _selectedCol >= 0 && _selectedRow < _gridRows &&
      _selectedCol < _gridCols) {
    auto sprite = _gemSprites[_selectedRow][_selectedCol];
    if (sprite) {
      sprite->setColor(ax::Color3B(220, 220, 220));
    }
  }
}

void GameScene::handleCellClick(int row, int col) {
  if (_isAnimating) return;

  AXLOGD("handleCellClick: row=%d, col=%d, selected=(%d,%d)", row, col,
         _selectedRow, _selectedCol);
  printf("handleCellClick: row=%d, col=%d, selected=(%d,%d)\n", row, col,
         _selectedRow, _selectedCol);

  if (row < 0 || row >= _gridRows || col < 0 || col >= _gridCols) return;

  if (_selectedRow == -1) {
    // Ничего не выбрано - выбираем текущую
    _selectedRow = row;
    _selectedCol = col;
    updateSelectionHighlight();
    return;
  }

  // Уже есть выбранная клетка
  if (_selectedRow == row && _selectedCol == col) {
    // Клик по той же - снимаем выбор
    _selectedRow = -1;
    _selectedCol = -1;
    updateSelectionHighlight();
    return;
  }

  // Проверяем, соседняя ли клетка (манхэттенское расстояние == 1)
  int dr = std::abs(row - _selectedRow);
  int dc = std::abs(col - _selectedCol);

  if ((dr == 1 && dc == 0) || (dr == 0 && dc == 1)) {
    int selectedRow = _selectedRow;
    int selectedCol = _selectedCol;

    _selectedRow = -1;
    _selectedCol = -1;
    updateSelectionHighlight();

    _field.swaped(selectedRow, selectedCol, row, col);
    bool hadMatches = _field.processMatches();

    if (!hadMatches) {
      _field.swaped(selectedRow, selectedCol, row, col);
    }
    redrawGrid();
  }
}

GameScene::GameScene() {
  _sceneID = ++s_sceneID;
  AXLOGD("Scene: ctor: #{}", _sceneID);
}

GameScene::~GameScene() {
  AXLOGD("~Scene: dtor: #{}", _sceneID);

  if (_touchListener) _eventDispatcher->removeEventListener(_touchListener);
  if (_keyboardListener)
    _eventDispatcher->removeEventListener(_keyboardListener);
  if (_mouseListener) _eventDispatcher->removeEventListener(_mouseListener);
  _sceneID = -1;
}
