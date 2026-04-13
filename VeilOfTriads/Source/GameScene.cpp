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
#include "FieldOfGems.h"
#include <unordered_map>

using namespace ax;

static int s_sceneID = 1000;

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf(
        "Depending on how you compiled you might have to add 'Content/' in front of filenames in "
        "GameScene.cpp\n");
}

// on "init" you need to initialize your instance
bool GameScene::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = _director->getVisibleSize();
    auto origin      = _director->getVisibleOrigin();
    auto safeArea    = _director->getSafeAreaRect();
    auto safeOrigin  = safeArea.origin;

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
                                           AX_CALLBACK_1(GameScene::menuCloseCallback, this));

    if (closeItem == nullptr || closeItem->getContentSize().width <= 0 || closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = safeOrigin.x + safeArea.size.width - closeItem->getContentSize().width / 2;
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
    drawNode->setPosition(Vec2::ZERO); // Позиция узла в его родительской системе координат

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
    _touchListener                 = EventListenerTouchAllAtOnce::create();
    _touchListener->onTouchesBegan = AX_CALLBACK_2(GameScene::onTouchesBegan, this);
    _touchListener->onTouchesMoved = AX_CALLBACK_2(GameScene::onTouchesMoved, this);
    _touchListener->onTouchesEnded = AX_CALLBACK_2(GameScene::onTouchesEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);

    _mouseListener                = EventListenerMouse::create();
    _mouseListener->onMouseMove   = AX_CALLBACK_1(GameScene::onMouseMove, this);
    _mouseListener->onMouseUp     = AX_CALLBACK_1(GameScene::onMouseUp, this);
    _mouseListener->onMouseDown   = AX_CALLBACK_1(GameScene::onMouseDown, this);
    _mouseListener->onMouseScroll = AX_CALLBACK_1(GameScene::onMouseScroll, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_mouseListener, this);

    _keyboardListener                = EventListenerKeyboard::create();
    _keyboardListener->onKeyPressed  = AX_CALLBACK_2(GameScene::onKeyPressed, this);
    _keyboardListener->onKeyReleased = AX_CALLBACK_2(GameScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithFixedPriority(_keyboardListener, 11);

    // add a label shows "Hello World"
    // create and initialize a label

    auto label = Label::createWithTTF("Hello World", "fonts/PixelGameFont.ttf", 24);
    if (label == nullptr)
    {
        problemLoading("'fonts/PixelGameFont.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        label->setPosition(
            Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - label->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(label, 1);
    }

    // scheduleUpdate() is required to ensure update(float) is called on every loop
    scheduleUpdate();

    return true;
}

Vec2 GameScene::touchToGridIndex(const Vec2& touchLocation)
{
  if (touchLocation.x < _gridStartPos.x ||
      touchLocation.x > _gridStartPos.x + _gridCols * _cellSize ||
      touchLocation.y < _gridStartPos.y ||
      touchLocation.y > _gridStartPos.y + _gridRows * _cellSize)
  {
    return Vec2(-1, -1); // За пределами
  }

  int col = static_cast<int>((touchLocation.x - _gridStartPos.x) / _cellSize);
  int row = static_cast<int>((touchLocation.y - _gridStartPos.y) / _cellSize);
  return Vec2(col, row);
}

void GameScene::onTouchesBegan(const std::vector<ax::Touch*>& touches, ax::Event* event)
{
    for (auto&& t : touches)
    {
        AXLOGD("onTouchesBegan detected, X:{}  Y:{}", t->getLocation().x, t->getLocation().y);
        Vec2 location = t->getLocation();
        Vec2 index = touchToGridIndex(location);
        if (index.x >= 0 && index.y >= 0)
        {
          int col = static_cast<int>(index.x);
          int row = static_cast<int>(index.y);

          _field.click(row, col);
          redrawGrid();
        }
    }
}

void GameScene::onTouchesMoved(const std::vector<ax::Touch*>& touches, ax::Event* event)
{
    for (auto&& t : touches)
    {
        // AXLOGD("onTouchesMoved detected, X:{}  Y:{}", t->getLocation().x, t->getLocation().y);
    }
}

void GameScene::onTouchesEnded(const std::vector<ax::Touch*>& touches, ax::Event* event)
{
    for (auto&& t : touches)
    {
        // AXLOGD("onTouchesEnded detected, X:{}  Y:{}", t->getLocation().x, t->getLocation().y);
    }
}

bool GameScene::onMouseDown(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 location = e->getLocationInView();
    location = _director->convertToGL(location);

    Vec2 index = touchToGridIndex(location);
    if (index.x >= 0 && index.y >= 0)
    {
      int col = static_cast<int>(index.x);
      int row = static_cast<int>(index.y);
      _field.click(row, col);
      redrawGrid();
    }
    AXLOGD("onMouseDown detected, button: {}", static_cast<int>(e->getMouseButton()));
    return true;
}

bool GameScene::onMouseUp(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    AXLOGD("onMouseUp detected, button: {}", static_cast<int>(e->getMouseButton()));
    return true;
}

bool GameScene::onMouseMove(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    AXLOGD("onMouseMove detected, X:{}  Y:{}", e->getLocation().x, e->getLocation().y);
    return true;
}

bool GameScene::onMouseScroll(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    // AXLOGD("onMouseScroll detected, X:{}  Y:{}", e->getScrollX(), e->getScrollY());
    return true;
}

void GameScene::onKeyPressed(EventKeyboard::KeyCode code, Event* event)
{
    AXLOGD("Scene: #{} onKeyPressed, keycode: {}", _sceneID, static_cast<int>(code));
}

void GameScene::onKeyReleased(EventKeyboard::KeyCode code, Event* event)
{
    AXLOGD("onKeyReleased, keycode: {}", static_cast<int>(code));
}

void GameScene::update(float delta)
{
    switch (_gameState)
    {
    case GameState::init:
    {
        _gameState = GameState::update;
        break;
    }

    case GameState::update:
    {
        /////////////////////////////
        // Add your codes below...like....
        //
        // UpdateJoyStick();
        // UpdatePlayer();
        // UpdatePhysics();
        // ...
        break;
    }

    case GameState::pause:
    {
        /////////////////////////////
        // Add your codes below...like....
        //
        // anyPauseStuff()

        break;
    }

    case GameState::menu1:
    {  /////////////////////////////
        // Add your codes below...like....
        //
        // UpdateMenu1();
        break;
    }

    case GameState::menu2:
    {  /////////////////////////////
        // Add your codes below...like....
        //
        // UpdateMenu2();
        break;
    }

    case GameState::end:
    {  /////////////////////////////
        // Add your codes below...like....
        //
        // CleanUpMyCrap();
        menuCloseCallback(this);
        break;
    }

    }  // switch
}

void GameScene::menuCloseCallback(ax::Object* sender)
{
    // Close the axmol game scene and quit the application
    _director->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use
     * _director->end() as given above,instead trigger a custom event created in RootViewController.mm
     * as below*/

    // EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void GameScene::setupGrid()
{
  auto visibleSize = _director->getVisibleSize();
  auto origin = _director->getVisibleOrigin();

  _cellSize = (visibleSize.height * 0.9f) / _gridRows;
  float gridWidth = _gridCols * _cellSize;
  float gridHeight = _gridRows * _cellSize;
  _gridStartPos.x = origin.x + (visibleSize.width - gridWidth) * 0.5f;
  _gridStartPos.y = origin.y + (visibleSize.height - gridHeight) * 0.5f;

  _gridDrawNode = DrawNode::create();
  _gridDrawNode->setPosition(Vec2::ZERO);
  this->addChild(_gridDrawNode);

  redrawGrid();
}

void GameScene::redrawGrid()
{
  _gridDrawNode->clear();
  auto field = _field.readField();

  static std::unordered_map<char, Color4F> colorMap = {
    {FieldOfGems::yellow,   Color4F(1.0f, 1.0f, 0.0f, 1.0f)},
    {FieldOfGems::red,      Color4F(1.0f, 0.0f, 0.0f, 1.0f)},
    {FieldOfGems::purple,   Color4F(0.8f, 0.0f, 0.8f, 1.0f)},
    {FieldOfGems::orange,   Color4F(1.0f, 0.5f, 0.0f, 1.0f)},
    {FieldOfGems::green,    Color4F(0.0f, 1.0f, 0.0f, 1.0f)},
    {FieldOfGems::blue,     Color4F(0.0f, 0.0f, 1.0f, 1.0f)},
    {FieldOfGems::nothing, Color4F(0.0f, 0.0f, 0.0f, 0.0f)}
  };

  for (int r = 0; r < _gridRows; ++r)
  {
    for (int c = 0; c < _gridCols; ++c)
    {
      float x = _gridStartPos.x + c * _cellSize;
      float y = _gridStartPos.y + r * _cellSize;
      Vec2 origin(x, y);
      Vec2 dest(x + _cellSize, y + _cellSize);

      char gemChar = field[r][c];
      Color4F color = colorMap[gemChar];

      _gridDrawNode->drawRect(origin, dest, Color4F::WHITE);

      if (gemChar != FieldOfGems::nothing)
      {
        _gridDrawNode->drawSolidRect(origin, dest, color);
      }
    }
  }
}

GameScene::GameScene()
{
    _sceneID = ++s_sceneID;
    AXLOGD("Scene: ctor: #{}", _sceneID);
}

GameScene::~GameScene()
{
    AXLOGD("~Scene: dtor: #{}", _sceneID);

    if (_touchListener)
        _eventDispatcher->removeEventListener(_touchListener);
    if (_keyboardListener)
        _eventDispatcher->removeEventListener(_keyboardListener);
    if (_mouseListener)
        _eventDispatcher->removeEventListener(_mouseListener);
    _sceneID = -1;
}
