#include <functional>
#include <iostream>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <unordered_set>
#include <vector>
using namespace std;
#ifndef ANIMATE_H
#define ANIMATE_H

// Make a base class for animations and then make a derived class for each type
// of animations

enum class AnimationType { SquareExpand, PieceMove, Text, Persist };

class Animation {
protected:
  float duration;
  Rectangle destination;
  Vector2 origin;
  int currentFrame;
  int TotalFrames;
  Vector2 position;
  float dy, dx;
  AnimationType type;

public:
  Animation(Vector2 origin, Vector2 position, AnimationType type) : type(type) {
    this->origin = {origin.x, origin.y};
    this->position = {position.x, position.y};
    currentFrame = 0;
    TotalFrames = 30;
    destination = {origin.x, origin.y, 48, 48};
  }

  AnimationType getType() { return type; }

  virtual void Draw() = 0;

  virtual bool Update() {
    // Update until destination.x reaches position.x
    // cout << currentFrame << " " << TotalFrames << endl;
    if (currentFrame <= TotalFrames) {
      destination.x =
          Lerp(destination.x, position.x, (float)currentFrame / TotalFrames);
      destination.y =
          Lerp(destination.y, position.y, (float)currentFrame / TotalFrames);
      currentFrame++;
      return true;
    }
    return false;
  }
  virtual ~Animation() {}
};

// Make a class called PersistAnimation that stays in place for a set amount of
// time on the screen

class PersistAnimation : public Animation {
  int maxFrames;
  int currentFrame;
  Texture2D *sprite;
  Rectangle source;
public:
  PersistAnimation(Vector2 origin, float time, Texture2D *sprite, Rectangle source)
      : Animation(origin, origin, AnimationType::Persist), sprite(sprite) {
    maxFrames = time * 60;
    this->source = source;
    currentFrame = 0;
  }
  void Draw() override {
    // cout << "PERSIST ANIMTATION: " << destination.x << " " << destination.y << endl;
    DrawTexturePro(*sprite, source, destination, Vector2{0, 0}, 0, WHITE);
  };
  bool Update() override {
    if (currentFrame < maxFrames) {
      currentFrame++;
      return true;
    }
    return false;
  }
};

class AnimationManager {
  // Maybe seperate Game Animations and GUI animations to control them
  // individually ?
  static unordered_multiset<Animation *> animations;

public:
  static void addAnimation(Animation *animation) {
    animations.insert(animation);
  }

  // update all of the animation and remove from list when done with
  static void Update() {
    for (auto it = animations.begin(); it != animations.end();) {
      if (!(*it)->Update()) {
        delete *it;
        it = animations.erase(it);
      } else {
        ++it;
      }
    }
  }

  static void Draw() {
    for (Animation *animation : animations) {
      animation->Draw();
    }
  }
};

class MovePacman : public Animation {
  Texture2D *sprite;
  Rectangle source;
public:
  MovePacman(Texture2D *sprite, Vector2 origin, Vector2 position, Rectangle source)
      : Animation(origin, position, AnimationType::PieceMove) {
    cout << origin.x << " " << origin.y << " " << position.x << " " << position.y << endl;
    this->sprite = sprite;
    this->source = source;
  }
  void Draw() override {
    // DrawTexturePro(*sprite, source, destination, Vector2{0, 0}, 0, WHITE);
    PersistAnimation *anim = new PersistAnimation({destination.x, destination.y}, 0.1, sprite, source);
    AnimationManager::addAnimation(anim);
  }
};


#endif // !ANIMATE_H
