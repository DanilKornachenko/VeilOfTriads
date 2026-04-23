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

#include "MainScene.h"

#include "GameScene.h"
#include "SoundManager.h"
#include "audio/AudioEngine.h"

using namespace ax;

static int s_sceneID = 1000;

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename) {
  printf("Error while loading: %s\n", filename);
  printf(
      "Depending on how you compiled you might have to add 'Content/' in front "
      "of filenames in "
      "MainScene.cpp\n");
}

// on "init" you need to initialize your instance
bool MainScene::init() {
  //////////////////////////////
  // 1. super init first
  if (!Scene::init()) {
    return false;
  }

  auto visibleSize = _director->getVisibleSize();
  auto origin = _director->getVisibleOrigin();
  auto safeArea = _director->getSafeAreaRect();
  auto safeOrigin = safeArea.origin;

  SoundManager::preload();
  SoundManager::playMusic();

  /////////////////////////////
  // 2. add a menu item with "X" image, which is clicked to quit the program
  //    you may modify it.

  /////////////////////////////
  // add a menu items

  // add a "next" icon
  auto nextItem =
      MenuItemImage::create("res/UI/button.png", "res/UI/button.png",
                            AX_CALLBACK_1(MainScene::goToGameScene, this));

  if (nextItem == nullptr || nextItem->getContentSize().width <= 0 ||
      nextItem->getContentSize().height <= 0) {
    problemLoading("'res/UI/button.png' and 'res/UI/button.png'");
  } else {
    float x = visibleSize.width / 2;
    float y = visibleSize.height / 2 - 150;
    nextItem->setPosition(Vec2(x, y));

    // Создаём текст
    auto label = Label::createWithTTF("Start", "fonts/PixelGameFont.ttf", 18);

    if (label) {
      label->setPosition(Vec2(nextItem->getContentSize().width / 2,
                              nextItem->getContentSize().height / 2));

      label->setColor(Color3B::WHITE);

      nextItem->addChild(label, 1);
    }
  }

  nextItem->setScale(2.0f);

  // Create button
  auto menuNext = Menu::create(nextItem, NULL);
  menuNext->setPosition(Vec2::ZERO);
  this->addChild(menuNext, 1);

  _soundIcon = Sprite::create(
    SoundManager::isEnabled()
        ? "res/UI/sound_icon1.png"
        : "res/UI/sound_icon2.png"
);

  _soundIcon->setContentSize(Vec2(50, 50));
  _soundIcon->setScale(50.0f / _soundIcon->getContentSize().width);

  _soundIcon->setPosition(Vec2(
        50,
        visibleSize.height - 50
        ));

  addChild(_soundIcon, 100);

// Listener
auto listener = EventListenerTouchOneByOne::create();

listener->onTouchBegan = [=](Touch* touch, Event* event) {
    if (_soundIcon->getBoundingBox().containsPoint(touch->getLocation())) {

        bool enabled = !SoundManager::isEnabled();
        SoundManager::setEnabled(enabled);

        _soundIcon->setTexture(enabled ?
            "res/UI/sound_icon1.png" :
            "res/UI/sound_icon2.png");

        _soundIcon->setContentSize(Vec2(50, 50));
        _soundIcon->setScale(50.0f / _soundIcon->getContentSize().width);

        return true;
    }
    return false;
};

_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, _soundIcon);

  /////////////////////////////
  // title

  auto title =
      Label::createWithTTF("Veil of Triads", "fonts/PixelGameFont.ttf", 128);

  if (title) {
    title->setPosition(Vec2(visibleSize.x / 2, visibleSize.y / 2));

    title->setColor(Color3B::WHITE);

    addChild(title, 1);
  }

  /////////////////////////////
  // 3. add your codes below...

  // Some templates (uncomment what you  need)
  _touchListener = EventListenerTouchAllAtOnce::create();
  _touchListener->onTouchesBegan =
      AX_CALLBACK_2(MainScene::onTouchesBegan, this);
  _touchListener->onTouchesMoved =
      AX_CALLBACK_2(MainScene::onTouchesMoved, this);
  _touchListener->onTouchesEnded =
      AX_CALLBACK_2(MainScene::onTouchesEnded, this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener,
                                                           this);

  _mouseListener = EventListenerMouse::create();
  _mouseListener->onMouseMove = AX_CALLBACK_1(MainScene::onMouseMove, this);
  _mouseListener->onMouseUp = AX_CALLBACK_1(MainScene::onMouseUp, this);
  _mouseListener->onMouseDown = AX_CALLBACK_1(MainScene::onMouseDown, this);
  //_mouseListener->onMouseScroll = AX_CALLBACK_1(MainScene::onMouseScroll,
  // this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(_mouseListener,
                                                           this);

  _keyboardListener = EventListenerKeyboard::create();
  _keyboardListener->onKeyPressed =
      AX_CALLBACK_2(MainScene::onKeyPressed, this);
  _keyboardListener->onKeyReleased =
      AX_CALLBACK_2(MainScene::onKeyReleased, this);
  _eventDispatcher->addEventListenerWithFixedPriority(_keyboardListener, 11);

  // add a label shows "Hello World"
  // create and initialize a label

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
  }

  // scheduleUpdate() is required to ensure update(float) is called on every
  // loop
  scheduleUpdate();

  return true;
}

void MainScene::onTouchesBegan(const std::vector<ax::Touch*>& touches,
                               ax::Event* event) {
  for (auto&& t : touches) {
    AXLOGD("onTouchesBegan detected, X:{}  Y:{}", t->getLocation().x,
           t->getLocation().y);
  }
}

void MainScene::onTouchesMoved(const std::vector<ax::Touch*>& touches,
                               ax::Event* event) {
  for (auto&& t : touches) {
    // AXLOGD("onTouchesMoved detected, X:{}  Y:{}", t->getLocation().x,
    // t->getLocation().y);
  }
}

void MainScene::onTouchesEnded(const std::vector<ax::Touch*>& touches,
                               ax::Event* event) {
  for (auto&& t : touches) {
    // AXLOGD("onTouchesEnded detected, X:{}  Y:{}", t->getLocation().x,
    // t->getLocation().y);
  }
}

bool MainScene::onMouseDown(Event* event) {
  EventMouse* e = static_cast<EventMouse*>(event);
  AXLOGD("onMouseDown detected, button: {}",
         static_cast<int>(e->getMouseButton()));
  return true;
}

bool MainScene::onMouseUp(Event* event) {
  EventMouse* e = static_cast<EventMouse*>(event);
  AXLOGD("onMouseUp detected, button: {}",
         static_cast<int>(e->getMouseButton()));
  return true;
}

bool MainScene::onMouseMove(Event* event) {
  EventMouse* e = static_cast<EventMouse*>(event);
  AXLOGD("onMouseMove detected, X:{}  Y:{}", e->getLocation().x,
         e->getLocation().y);
  return true;
}

bool MainScene::onMouseScroll(Event* event) {
  EventMouse* e = static_cast<EventMouse*>(event);
  // AXLOGD("onMouseScroll detected, X:{}  Y:{}", e->getScrollX(),
  // e->getScrollY());
  return true;
}

void MainScene::onKeyPressed(EventKeyboard::KeyCode code, Event* event) {
  AXLOGD("Scene: #{} onKeyPressed, keycode: {}", _sceneID,
         static_cast<int>(code));
}

void MainScene::onKeyReleased(EventKeyboard::KeyCode code, Event* event) {
  AXLOGD("onKeyReleased, keycode: {}", static_cast<int>(code));
}

void MainScene::update(float delta) {
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

void MainScene::menuCloseCallback(ax::Object* sender) {
  // Close the axmol game scene and quit the application
  _director->end();

  /*To navigate back to native iOS screen(if present) without quitting the
   * application  ,do not use _director->end() as given above,instead trigger a
   * custom event created in RootViewController.mm as below*/

  // EventCustom customEndEvent("game_scene_close_event");
  //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void MainScene::goToGameScene(ax::Object* sender) {
  auto gameScene = utils::createInstance<GameScene>();
  _director->replaceScene(gameScene);
}

MainScene::MainScene() {
  _sceneID = ++s_sceneID;
  AXLOGD("Scene: ctor: #{}", _sceneID);
}

MainScene::~MainScene() {
  AXLOGD("~Scene: dtor: #{}", _sceneID);

  if (_touchListener) _eventDispatcher->removeEventListener(_touchListener);
  if (_keyboardListener)
    _eventDispatcher->removeEventListener(_keyboardListener);
  if (_mouseListener) _eventDispatcher->removeEventListener(_mouseListener);
  _sceneID = -1;
}
