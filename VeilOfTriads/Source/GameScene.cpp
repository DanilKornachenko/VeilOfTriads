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
#include "audio/AudioEngine.h"

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

  AudioEngine::preload("res/Music/pop.ogg");

  auto visibleSize = _director->getVisibleSize();
  auto origin = _director->getVisibleOrigin();
  auto safeArea = _director->getSafeAreaRect();
  auto safeOrigin = safeArea.origin;

  /////////////////////////////
  // 2. add a menu item with "X" image, which is clicked to quit the program
  //    you may modify it.

  auto sprite = Sprite::create("res/UI/background.png");
  if (sprite == nullptr) {
    problemLoading("'res/UI/background.png'");
  } else {
    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(origin.x + visibleSize.width / 2,
                             origin.y + visibleSize.height / 2));

    // Растягиваем на весь экран
    float scaleX = visibleSize.width / sprite->getContentSize().width;
    float scaleY = visibleSize.height / sprite->getContentSize().height;

    sprite->setScale(scaleX, scaleY);

    // add the sprite as a child to this layer
    this->addChild(sprite, 0);
    auto drawNode = DrawNode::create();
    drawNode->setPosition(Vec2(0, 0));
    addChild(drawNode);

    drawNode->drawRect(safeArea.origin + Vec2(1, 1),
                       safeArea.origin + safeArea.size, Color4F::BLUE);
  }

  /////////////////////////////
  // Draw Field

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

    _scoreLabel = label;
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
    /*
    Vec2 location = t->getLocation();
    Vec2 index = touchToGridIndex(location);
    if (index.x >= 0 && index.y >= 0) {
      int col = static_cast<int>(index.x);
      int row = static_cast<int>(index.y);
      handleCellClick(row, col);
      AXLOGD("Touch at grid: col=%d, row=%d", col, row);
    }
    */
    _touchStart = t->getLocation();
    _swipeHandled = false;
  }
}

void GameScene::onTouchesMoved(const std::vector<ax::Touch*>& touches,
                               ax::Event* event) {
  if (_isAnimating || _swipeHandled) return;

  for (auto&& t : touches) {
    Vec2 current = t->getLocation();
    Vec2 delta = current - _touchStart;
    float threshold = 30.0f;  // Чувствительность свайпа

    if (fabs(delta.x) < threshold && fabs(delta.y) < threshold) return;

    Vec2 startIndex = touchToGridIndex(_touchStart);

    if (startIndex.x < 0 || startIndex.y < 0) return;

    int col = (int)startIndex.x;
    int row = (int)startIndex.y;

    int targetRow = row;
    int targetCol = col;

    // Определяем направление
    if (fabs(delta.x) > fabs(delta.y)) {
      // горизонтальный свайп
      if (delta.x > 0)
        targetCol += 1;  // вправо
      else
        targetCol -= 1;  // влево
    } else {
      // вертикальный свайп
      if (delta.y > 0)
        targetRow += 1;  // вверх
      else
        targetRow -= 1;  // вниз
    }

    // Проверка границ
    if (targetRow < 0 || targetRow >= _gridRows || targetCol < 0 ||
        targetCol >= _gridCols)
      return;

    // Запускаем анимацию свапа
    animateSwap(row, col, targetRow, targetCol);

    _swipeHandled = true;
  }
}

void GameScene::onTouchesEnded(const std::vector<ax::Touch*>& touches,
                               ax::Event* event) {
  _swipeHandled = false;
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
      ax::Sprite* sprite = _gemSprites[r][c];

      if (sprite == nullptr) {
        if (!imagePath.empty()) {
          sprite = ax::Sprite::create(imagePath);
          if (sprite) {
            float x = _gridStartPos.x + c * _cellSize + _cellSize / 2;
            float y = _gridStartPos.y + r * _cellSize + _cellSize / 2;
            sprite->setPosition(ax::Vec2(x, y));
            float scaleX = _cellSize / sprite->getContentSize().width;
            float scaleY = _cellSize / sprite->getContentSize().height;
            sprite->setScale(scaleX, scaleY);
            _gridContainer->addChild(sprite);
            _gemSprites[r][c] = sprite;
          }
        }
      } else {
        // Обновляем позицию и текстуру существующего спрайта
        float x = _gridStartPos.x + c * _cellSize + _cellSize / 2;
        float y = _gridStartPos.y + r * _cellSize + _cellSize / 2;
        sprite->setPosition(ax::Vec2(x, y));

        if (imagePath.empty()) {
          sprite->setVisible(false);
        } else {
          sprite->setVisible(true);
          auto currentTex = sprite->getTexture();
          if (!imagePath.empty()) {
            auto newTex =
                ax::Director::getInstance()->getTextureCache()->addImage(
                    imagePath);
            if (currentTex != newTex) {
              sprite->setTexture(newTex);
            }
          }
          float scaleX = _cellSize / sprite->getContentSize().width;
          float scaleY = _cellSize / sprite->getContentSize().height;
          sprite->setScale(scaleX, scaleY);
        }
      }
    }
  }
  updateSelectionHighlight();
}

void GameScene::animateMatches(const std::vector<std::vector<bool>>& matches) {
  _isAnimating = true;

  float duration = 0.2f;
  int animationsCount = 0;

  int gainedScore = 0;

  for (int r = 0; r < _gridRows; ++r) {
    for (int c = 0; c < _gridCols; ++c) {
      if (matches[r][c] && _gemSprites[r][c]) {
        auto sprite = _gemSprites[r][c];

        if (!soundPlayed) {
          AudioEngine::play2d("res/Music/pop.ogg");
          soundPlayed = true;
        }

        auto fade = FadeOut::create(duration);
        auto scale = ScaleTo::create(duration, 0.0f);
        auto spawn = Spawn::create(fade, scale, nullptr);

        sprite->runAction(spawn);

        animationsCount++;

        gainedScore = calculateScoreFromMatches(matches);
      }
    }
  }

  soundPlayed = false;

  auto callback = CallFunc::create([=]() {
    _score += gainedScore;
    updateScoreLabel();

    _field.removeMatches(matches);

    for (int r = 0; r < _gridRows; ++r) {
      for (int c = 0; c < _gridCols; ++c) {
        if (matches[r][c] && _gemSprites[r][c]) {
          _gemSprites[r][c]->removeFromParent();
          _gemSprites[r][c] = nullptr;
        }
      }
    }

    applyGravityWithAnimation();
  });

  if (animationsCount == 0) {
    callback->execute();
  } else {
    this->runAction(
        Sequence::create(DelayTime::create(duration), callback, nullptr));
  }
}

void GameScene::applyGravityWithAnimation() {
  _field.applyGravity();

  float duration = 0.2f;
  int animationsCount = 0;

  for (int c = 0; c < _gridCols; ++c) {
    int writeRow = 0;

    for (int r = 0; r < _gridRows; ++r) {
      if (_gemSprites[r][c]) {
        auto sprite = _gemSprites[r][c];

        if (r != writeRow) {
          float targetY =
              _gridStartPos.y + writeRow * _cellSize + _cellSize / 2;

          auto move =
              MoveTo::create(duration, Vec2(sprite->getPositionX(), targetY));

          sprite->runAction(move);

          _gemSprites[writeRow][c] = sprite;
          _gemSprites[r][c] = nullptr;

          animationsCount++;
        }

        writeRow++;
      }
    }
  }

  auto callback = CallFunc::create([=]() { spawnNewGems(); });

  if (animationsCount == 0) {
    callback->execute();
  } else {
    this->runAction(
        Sequence::create(DelayTime::create(duration), callback, nullptr));
  }
}

void GameScene::spawnNewGems() {
  _field.fillEmptyCells();

  float duration = 0.2f;
  int animationsCount = 0;

  auto field = _field.readField();

  for (int r = 0; r < _gridRows; ++r) {
    for (int c = 0; c < _gridCols; ++c) {
      if (_gemSprites[r][c] == nullptr) {
        char gem = field[r][c];
        if (gem == '.' || gem == '\0') continue;

        std::string path = getGemImagePath(gem);

        auto sprite = Sprite::create(path);
        if (!sprite) continue;

        float x = _gridStartPos.x + c * _cellSize + _cellSize / 2;
        float startY = _gridStartPos.y + _gridRows * _cellSize;
        float targetY = _gridStartPos.y + r * _cellSize + _cellSize / 2;

        sprite->setPosition(Vec2(x, startY));

        float scaleX = _cellSize / sprite->getContentSize().width;
        float scaleY = _cellSize / sprite->getContentSize().height;
        sprite->setScale(scaleX, scaleY);

        _gridContainer->addChild(sprite);
        _gemSprites[r][c] = sprite;

        auto move = MoveTo::create(duration, Vec2(x, targetY));
        sprite->runAction(move);

        animationsCount++;
      }
    }
  }

  auto callback = CallFunc::create([=]() { checkMatchesAgain(); });

  if (animationsCount == 0) {
    callback->execute();
  } else {
    this->runAction(
        Sequence::create(DelayTime::create(duration), callback, nullptr));
  }
}

void GameScene::checkMatchesAgain() {
  std::vector<std::vector<bool>> matches;

  if (_field.findMatches(matches)) {
    animateMatches(matches);
  } else {
    _isAnimating = false;
  }
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

    animateSwap(selectedRow, selectedCol, row, col);
  }
}

void GameScene::animateSwap(int r1, int c1, int r2, int c2) {
  _isAnimating = true;

  auto sprite1 = _gemSprites[r1][c1];
  auto sprite2 = _gemSprites[r2][c2];

  Vec2 pos1 = sprite1->getPosition();
  Vec2 pos2 = sprite2->getPosition();

  float duration = 0.2f;

  auto move1 = EaseInOut::create(MoveTo::create(duration, pos2), 2.0f);
  auto move2 = EaseInOut::create(MoveTo::create(duration, pos1), 2.0f);

  auto callback = CallFunc::create([=]() {
    std::swap(_gemSprites[r1][c1], _gemSprites[r2][c2]);

    _field.swaped(r1, c1, r2, c2);

    std::vector<std::vector<bool>> matches;

    if (_field.findMatches(matches)) {
      animateMatches(matches);

    } else {
      animateSwapBack(r1, c1, r2, c2);
    }
  });

  sprite1->runAction(move1);
  sprite2->runAction(Sequence::create(move2, callback, nullptr));
}

void GameScene::animateSwapBack(int r1, int c1, int r2, int c2) {
  auto sprite1 = _gemSprites[r1][c1];
  auto sprite2 = _gemSprites[r2][c2];

  Vec2 pos1 = sprite1->getPosition();
  Vec2 pos2 = sprite2->getPosition();

  float duration = 0.2f;

  auto move1 = MoveTo::create(duration, pos2);
  auto move2 = MoveTo::create(duration, pos1);

  auto callback = CallFunc::create([=]() {
    // возвращаем обратно
    std::swap(_gemSprites[r1][c1], _gemSprites[r2][c2]);
    _field.swaped(r1, c1, r2, c2);

    _isAnimating = false;
    redrawGrid();
  });

  sprite1->runAction(move1);
  sprite2->runAction(Sequence::create(move2, callback, nullptr));
}

void GameScene::updateScoreLabel() {
  if (_scoreLabel) {
    _scoreLabel->setString("Score : " + std::to_string(_score));
  }
}

int GameScene::calculateScoreFromMatches(
    const std::vector<std::vector<bool>>& matches) {
  int score = 0;

  for (int r = 0; r < _gridRows; ++r) {
    int length = 0;

    for (int c = 0; c < _gridCols; ++c) {
      if (matches[r][c]) {
        length++;
      } else {
        if (length >= 3) {
          score += getMatchScore(length);
        }
        length = 0;
      }
    }

    if (length >= 3) {
      score += getMatchScore(length);
    }
  }

  for (int c = 0; c < _gridCols; ++c) {
    int length = 0;

    for (int r = 0; r < _gridRows; ++r) {
      if (matches[r][c]) {
        length++;
      } else {
        if (length >= 3) {
          score += getMatchScore(length);
        }
        length = 0;
      }
    }

    if (length >= 3) {
      score += getMatchScore(length);
    }
  }

  return score;
}

int GameScene::getMatchScore(int length) {
  if (length >= 3) return length * 10;

  return 0;
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
