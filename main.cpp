#include "headers/animate.h"
#include "headers/console.h"
#include "headers/effects.h"
#include "headers/event.h"
#include "headers/gui.h"
#include "headers/raylib_compat.h"
#include <bits/stdc++.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#define INF INT_MAX
using namespace std;
const int screenWidth = 912 + 400;
const int screenHeight = 1056;

enum STATE { SCATTER, FRIGHTENED, CHASE, EATEN };

// Make it Private and not globally Available
vector<vector<char>> prop(22, vector<char>(19, ' '));
int score = 0;
int Lives = 2;
int Power = 3;

void saveVector(const vector<vector<char>> &vector,
                const std::string &filename) {
  std::ofstream file(filename, std::ios::out | std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return;
  }

  size_t rows = vector.size();
  size_t cols = vector[0].size();

  file.write(reinterpret_cast<const char *>(&rows), sizeof(size_t));
  file.write(reinterpret_cast<const char *>(&cols), sizeof(size_t));

  for (const auto &row : vector) {
    file.write(row.data(), cols);
  }

  file.close();
}

// Function to load the 2D vector from a binary file
vector<vector<char>> loadVector(const std::string &filename) {
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return {};
  }

  size_t rows, cols;

  // Read the dimensions of the vector
  file.read(reinterpret_cast<char *>(&rows), sizeof(size_t));
  file.read(reinterpret_cast<char *>(&cols), sizeof(size_t));

  vector<vector<char>> vector(rows, std::vector<char>(cols));

  // Read the vector data
  for (auto &row : vector) {
    file.read(row.data(), cols);
  }

  file.close();

  return vector;
}

class MapHandler {
  TextBox TBox;
  SmallButton CheckMark;
  SmallButton LoadMark;
  Texture2D *gui;

public:
  MapHandler(Texture2D *gui)
      : TBox({8, screenHeight / 2 + 12 * 16, 190, 50}),
        CheckMark({8, screenHeight / 2 + 12 * 16 + 50}, gui, ICON_TYPES::CHECK),
        LoadMark({8 + 6 * 16 + 10, screenHeight / 2 + 12 * 16 + 50}, gui,
                 ICON_TYPES::MAIL) {
    this->gui = gui;
  }
  void MapEdit() {
    // TODO: Make The Hover animation on the grid
    Vector2 mouse = GetMousePosition();
    // Check if the mouse is in the grid section
    TBox.Draw();
    TBox.Update();
    CheckMark.Draw();
    CheckMark.Update();
    LoadMark.Draw();
    LoadMark.Update();
    string location = "./assets/" + TBox.getInput() + ".bin";
    if (CheckMark.getPressed() && TBox.getInput() != "" && !FileExists(location.c_str())) {
      cout << TBox.getInput() << endl;
      saveVector(prop, location);
      CheckMark.setPressed(false);
    } else if (LoadMark.getPressed() && FileExists(location.c_str())) {
      prop = loadVector("./assets/" + TBox.getInput() + ".bin");
      LoadMark.setPressed(false);
    }
    if (!CheckCollisionPointRec(mouse, Rectangle{200, 0, 48 * 19, 48 * 22})) {
      return;
    }
    DrawHoverRectangle(mouse);
    // Mouse is in the Grid

    // TODO: Handle Right and Left click
    float x = floor((mouse.x - 200) / 48);
    float y = floor((mouse.y) / 48);
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
      // cout << "Right Clicked" << endl;
      prop[(int)y][(int)x] = ' ';
    } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      prop[(int)y][(int)x] = '#';
    }
    // Display the save button and the Input Box Below the Edit button
  }
  void DrawHoverRectangle(Vector2 mouse) {
    float x = floor((mouse.x - 200) / 48);
    float y = floor((mouse.y) / 48);
    DrawRectangleRoundedLinesCompat({x * 48 + 200, y * 48, 48, 48}, 0.1, 1, 5,
                                    ORANGE);
  }
};

struct Point {
  int x, y;
  Point(int _x, int _y) : x(_x), y(_y) {}
};

bool operator>(const Point &a, const Point &b) {
  return make_pair(a.x, a.y) > make_pair(b.x, b.y);
}

bool operator<(const Point &a, const Point &b) {
  return make_pair(a.x, a.y) < make_pair(b.x, b.y);
}

bool isNotValid(int x, int y, int rows, int cols) {
  return x < 0 || x >= cols || y < 0 || y >= rows || prop[y][x] == '#';
}

const int dx[] = {0, +1, 0, -1};
const int dy[] = {-1, 0, 1, 0};

// Place this in a map
void Map(Image map) {
  for (int i = 0; i < 22; i++) {
    for (int j = 0; j < 19; j++) {
      int c = (j * 48) + 24;
      int r = (i * 48) + 24;
      Color color = GetImageColor(map, c, r);
      int hex = ColorToInt(color) % 7;

      if (hex == 6) {
        prop[i][j] = '#';
      } else {
        prop[i][j] = ' ';
      }
      // cout << prop[i][j] << " ";
    }
    // cout << endl;
  }
}

void DrawMap(Texture2D map) {
  for (int i = 0; i < 22; i++) {
    for (int j = 0; j < 19; j++) {
      // cout << prop[i][j] << " ";
      if (prop[i][j] == ' ') {
        Rectangle src = {0, 8, 8, 8};
        DrawTexturePro(map, src, {(float)j * 48 + 200, (float)i * 48, 48, 48},
                       {0, 0}, 0, WHITE);
      } else if (prop[i][j] == '0') {
        Rectangle src = {8, 8, 8, 8};
        DrawTexturePro(map, src, {(float)j * 48 + 200, (float)i * 48, 48, 48},
                       {0, 0}, 0, WHITE);
      } else if (prop[i][j] == 'P') {
        Rectangle src = {8 + 8 * 2, 8, 8, 8};
        DrawTexturePro(map, src, {(float)j * 48 + 200, (float)i * 48, 48, 48},
                       {0, 0}, 0, WHITE);
      }

      if (prop[i][j] != '#')
        continue;
      // Now check how many sides are attached
      int loc = 0;
      for (int k = 0; k < 4; k++) {
        int xx = j + dx[k];
        int yy = i + dy[k];

        if (xx < 0 || xx >= 19 || yy < 0 || yy >= 22)
          continue;

        if (prop[yy][xx] == '#')
          loc |= (int)pow(2, k);
      }

      // Draw the appropriate sprite
      Rectangle src = {(float)loc * 8, 0, 8, 8};
      DrawTexturePro(map, src, {(float)j * 48 + 200, (float)i * 48, 48, 48},
                     {0, 0}, 0, WHITE);
    }
    // cout << endl;
  }
}

class Entity {
protected:
  int x;
  int y;
  int speed;
  int dX;
  int dY;
  int pdX;
  int pdY;
  int frameC = 0;
  int frameS = 8;
  int currentFrame = 0;

public:
  Vector2 getPos() { return {(float)x, (float)y}; }
  void Check() {
    if (dX == pdX && dY == pdY)
      return;

    int dx[] = {0, 1, 0, 1};
    int dy[] = {0, 1, 1, 0};

    for (int i = 0; i < 4; i++) {
      int xx = x + 3 + pdX * speed;
      int yy = y + 3 + pdY * speed;

      int grid_x = (xx + 42 * dx[i]) / 48;
      int grid_y = (yy + 42 * dy[i]) / 48;
      // DrawCircle(xx + 42 * dx[i] + 200, yy + 42 * dy[i], 5, RED);
      // cout << grid_x << " " << grid_y << endl;
      if (prop[grid_y][grid_x] == '#')
        return;
    }

    dX = pdX;
    dY = pdY;
  }
  void virtual Update() = 0;
};

// Capitalize this class
class Pacman : public Entity {
  Image pac_left;
  Texture2D pacLeft;
  Rectangle pacRect = {0, 0, 16, 16};
  Sound chomp;
  bool eaten = false;
  bool kill = false;
  bool set = false;
  int coordX;
  int coordY;

public:
  bool getEaten() { return eaten; }
  void setEaten(bool eaten) { this->eaten = eaten; }
  bool killMode() { return kill; }
  void killMode(bool kill) { this->kill = kill; }
  Pacman() : Entity() {
    x = 9 * 48;
    y = 16 * 48;
    chomp = LoadSound("./assets/sound/chomp.wav");
    SetSoundVolume(chomp, 0.5);
    speed = 5;
    dX = 0;
    pdX = -1;
    dY = 0;
    pdY = 0;
    pac_left = LoadImage("./assets/Sprite-0001.png");
    pacLeft = LoadTextureFromImage(pac_left);
  }
  Rectangle getRect() { return Rectangle{(float)x, (float)y, 48, 48}; }
  Vector2 getDir() { return Vector2{(float)dX, (float)dY}; };
  // Exposed so the attract screen can animate the real sprite.
  Texture2D Sprite() { return pacLeft; }
  // Reused as the countdown beep on the last ten seconds.
  Sound Chomp() { return chomp; }
  void Reset() {
    x = 9 * 48;
    y = 16 * 48;
    dX = -1;
    pdX = -1;
    dY = 0;
    pdY = 0;
  }
  void Update() {
    Check();
    int centered_x = x + 24;
    int centered_y = y + 24;
    int i = (y + 24) / 48.0;
    int j = (x + 24) / 48.0;
    // If Right or bottom then floor

    if (prop[i][j] == ' ') {
      score += 20;
      prop[i][j] = 'X';
      PlaySound(chomp);
    } else if (prop[i][j] == '0') {
      score += 40;
      prop[i][j] = 'X';
      PlaySound(chomp);
      eaten = true;
    } else if (prop[i][j] == 'P') {
      score += 10;
      prop[i][j] = 'X';
      PlaySound(chomp);
      killMode(true);
    }

    // DrawCircle(x + 24 + 200, y + 24, 5, RED);
    // cout << i + dY << " " << j + dX << endl;
    if (prop[(centered_y + 26 * dY) / 48][(centered_x + 26 * dX) / 48] != '#') {
      // DrawCircle(centered_x + 25 * dX + 200, centered_y + 25 * dY, 5, RED);

      // Increase frame
      Animate();
      x += dX * speed;
      y += dY * speed;
    }
    i = (y + 24) / 48.0;
    j = (x + 24) / 48.0;
    // Update on grid
    // cout << y / 48 << " " << x / 48 << endl;
  }

  void Animate() {
    frameC++;
    if (frameC >= (60 / frameS)) {
      frameC = 0;
      currentFrame = (currentFrame + 1) % 3;
      pacRect.x = currentFrame * 16;
    }
  }

  void Draw() {
    if (dX == 0 && dY == -1) {
      pacRect.y = 2 * 16;
    } else if (dX == 0 && dY == 1) {
      pacRect.y = 3 * 16;
    } else if (dX == -1 && dY == 0) {
      pacRect.y = 1 * 16;
    } else if (dX == 1 && dY == 0) {
      pacRect.y = 0 * 16;
    }

    if (currentFrame == 2) {
      pacRect.y = 0 * 16;
    }

    DrawTexturePro(pacLeft, pacRect, {(float)x + 200, (float)y, 48, 48}, {0, 0},
                   0, WHITE);
  }

  void PowerUp() {
    if (Power <= 0)
      return;
    int centerX = (x + 24) / 48.0;
    int centerY = (y + 24) / 48.0;
    if (IsKeyDown(KEY_SPACE)) {
      while (prop[centerY + dY][centerX + dX] != '#') {
        centerY += dY;
        centerX += dX;
        // cout << centerY << " " << centerX << endl;
      }
      coordX = centerX * 48 + 200;
      coordY = centerY * 48;
      DrawCircle(centerX * 48 + 24 + 200, centerY * 48 + 24, 20, YELLOW);
      set = true;
    }
    if (set && IsKeyUp(KEY_SPACE)) {
      Vector2 origin = {(float)x + 200, (float)y};
      Vector2 destination = {(float)coordX, (float)coordY};
      // cout << origin.x << " " << origin.y << endl;
      // cout << destination.x << " " << destination.y << endl;
      MovePacman *anim = new MovePacman(&pacLeft, origin, destination, pacRect);
      AnimationManager::addAnimation(anim);
      EffectManager::addEffect(Smoke, Vector2{(float)x + 200, (float)y});
      x = coordX - 200;
      y = coordY;
      EffectManager::addEffect(Dash, Vector2{(float)x + 200, (float)y});
      set = false;
    }
  }

  // Direction input. WASD is kept because the original game used it, and the
  // arrow keys are added because that is what a first-time player reaches for.
  void Control() {
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
      pdX = 0;
      pdY = -1;
    } else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
      pdX = -1;
      pdY = 0;
    } else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
      pdX = 0;
      pdY = 1;
    } else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
      pdX = 1;
      pdY = 0;
    }
    PowerUp();
  }
};

class Ghost : public Entity {
  Image ghostI;
  Texture2D ghostT;
  Rectangle GhostRect;
  // STATE: behave according to STATE
  STATE state = CHASE;
  int timer = 0;
  Vector2 scatterLocation = {18 * 48, -48};
  Vector2 homeLocation = {9 * 48, 10 * 48};
  Vector2 start;

public:
  Vector2 scatter() { return scatterLocation; }
  void Reset() {
    x = 11 * 48;
    y = 8 * 48;
    start = {(float)x, (float)y};
    dX = 1;
    dY = 0;
  }
  Ghost(float Color, Vector2 scatterLocation) {
    x = 11 * 48 - Color * 48;
    y = 8 * 48;
    start = {(float)x, (float)y};
    this->scatterLocation = scatterLocation;
    speed = 4;
    ghostI = LoadImage("./assets/Sprite-0001.png");
    ghostT = LoadTextureFromImage(ghostI);
    GhostRect = {0, 4 * 16 + Color * 16, 16, 16};
    speed = 4;
    dX = 1;
    dY = 0;
  }

  void Draw() {
    // Up
    if (state == CHASE || state == SCATTER) {
      DrawTexturePro(ghostT, GhostRect, {(float)x + 200, (float)y, 48, 48},
                     {0, 0}, 0, WHITE);
    } else if (state == FRIGHTENED) {
      DrawTexturePro(
          ghostT, Rectangle{(float)16 * currentFrame + 8 * 16, 4 * 16, 16, 16},
          {(float)x + 200, (float)y, 48, 48}, {0, 0}, 0, WHITE);
    } else if (state == EATEN) {
      // Right
      float source_x = 0 + 16 * 8;
      if (dX == -1)
        source_x += 16;
      else if (dY == -1)
        source_x += 16 * 2;
      else if (dY == 1)
        source_x += 16 * 3;
      DrawTexturePro(ghostT, Rectangle{source_x, 5 * 16, 16, 16},
                     {(float)x + 200, (float)y, 48, 48}, {0, 0}, 0, WHITE);
    }
  }

  void setState(STATE s) {
    if (s == FRIGHTENED)
      timer = 5 * 60;
    else
      timer = 0;
    state = s;
  }

  virtual Vector2 chaseTarget(Pacman &pac) = 0;

  bool isHome(int centered_x_grid, int centered_y_grid) {
    return (centered_y_grid == 10 &&
            (centered_x_grid >= 8 && centered_x_grid <= 10));
  }
  Vector2 target(Pacman &pac) {
    if (CheckCollisionPointRec(Vector2{(float)x + 200, (float)y},
                               Rectangle{8 * 48 + 200, 9 * 48, 48 * 3, 48 * 2}))
      return Vector2{9 * 48, 9 * 48};
    switch (state) {
    case SCATTER:
      // cout << "SCATTERING" << endl;
      return scatterLocation;
      break;
    case CHASE:
      return chaseTarget(pac);
      break;
    case FRIGHTENED:
      return {(float)x, (float)y};
      break;
    case EATEN:
      return homeLocation;
      break;
    default:
      return {(float)x, (float)y};
      break;
    }
  }
  // The state controls the target and the texture that appears
  void next(Vector2 target) {
    // Of all the directions possible, Return the direction that is NOT opposite
    // to current AND allows the Ghost to be closest

    int centered_x = (x + 24);
    int centered_y = (y + 24);

    int centered_x_grid = (x + 24) / 48;
    int centered_y_grid = (y + 24) / 48;

    int target_x = (target.x + 24) / 48;
    int target_y = (target.y + 24) / 48;

    vector<pair<int, float>> distances;
    for (int i = 0; i < 4; i++) {
      // Not at home && not going in reverse
      if (!isHome(centered_x_grid, centered_y_grid) && dx[i] == -dX &&
          dy[i] == -dY)
        continue;

      if (isNotValid((centered_x + dx[i] * 24) / 48,
                     (centered_y + dy[i] * 24) / 48, 22, 19))
        continue;

      if (prop[(centered_y + 26 * dy[i]) / 48]
              [(centered_x + 26 * dx[i]) / 48] == '#')
        continue;

      // Calculate distance
      // i direction (dx, dy)
      distances.push_back(
          {i, distance(centered_x_grid + dx[i], centered_y_grid + dy[i],
                       target_x, target_y)});
    }
    // Ascending order sort according to the distance
    sort(distances.begin(), distances.end(),
         [](pair<int, float> a, pair<int, float> b) {
           return a.second < b.second;
         });
    // cout << distances.size() << " " << endl;
    if (distances.size() == 0) {
      setState(SCATTER);
      return;
    }
    if (state == FRIGHTENED) {
      int randomValid = GetRandomValue(0, distances.size() - 1);
      pdX = dx[distances[randomValid].first];
      pdY = dy[distances[randomValid].first];
    } else {
      pdX = dx[distances[0].first];
      pdY = dy[distances[0].first];
    }
  }
  float distance(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
  }

  void Animate() {
    frameC++;
    if (frameC >= (60 / frameS)) {
      frameC = 0;
      currentFrame = (currentFrame + 1) % 2;
      GhostRect.x = currentFrame * 16;
      if (dX == 0 && dY == -1) {
        GhostRect.x += 2 * 32;
      }
      // Down
      else if (dX == 0 && dY == 1) {
        GhostRect.x += 3 * 32;
      }
      // Left
      else if (dX == -1 && dY == 0) {
        GhostRect.x += 1 * 32;
      }
      // Right
      else if (dX == 1 && dY == 0) {
        GhostRect.x += 0 * 32;
      }
    }
  }
  void Update() override {}
  void Update(Pacman &pac) {
    // What is the next direction
    // dX dY
    next(target(pac));
    Check();
    int centered_x = x + 24;
    int centered_y = y + 24;
    int i = (y + 24) / 48.0;
    int j = (x + 24) / 48.0;
    // If Right or bottom then floor
    // DrawCircle(x + 24 + 200, y + 24, 5, RED);
    // cout << i + dY << " " << j + dX << endl;
    if (prop[(centered_y + 26 * dY) / 48][(centered_x + 26 * dX) / 48] != '#') {
      // DrawCircle(centered_x + 25 * dX + 200, centered_y + 25 * dY, 5, RED);
      Animate();
      x += dX * speed;
      y += dY * speed;
    }
    i = (y + 24) / 48.0;
    j = (x + 24) / 48.0;
    // Update on grid
    // cout << y / 48 << " " << x / 48 << endl;
    // DrawRectangle(8*48 + 200, 9*48, 48*3, 48*2, RED);
    int rand = GetRandomValue(0, 1000);

    if (state == FRIGHTENED && timer != 0)
      timer--;
    else if (state == FRIGHTENED && timer == 0)
      setState(CHASE);

    if (state == CHASE && rand < 2 && !isHome(i, j)) {
      setState(SCATTER);
    } else if (state == SCATTER && rand < 10) {
      setState(CHASE);
    }

    // Home
    if (CheckCollisionPointRec(Vector2{(float)x, (float)y},
                               Rectangle{8 * 48, 9 * 48, 48 * 3, 48 * 2})) {
      setState(CHASE);
    }
    if (state != EATEN &&
        CheckCollisionRecs(pac.getRect(),
                           Rectangle{(float)x, (float)y, 48, 48})) {
      // cout << "Collision" << endl;
      if (state == FRIGHTENED) {
        setState(EATEN);
      } else {
        pac.Reset();
        Lives--;
      }
    }
  }
};

class Blinky : public Ghost {
public:
  Blinky() : Ghost(0.0f, Vector2{18 * 48, -48}) {}
  Vector2 chaseTarget(Pacman &pac) override { return pac.getPos(); }
};

class Pinky : public Ghost {
public:
  Pinky() : Ghost(1, Vector2{0, -48}) {}
  Vector2 chaseTarget(Pacman &pac) override {
    return Vector2Add(pac.getPos(),
                      Vector2Multiply(pac.getDir(), {48 * 4, 48 * 4}));
  }
};

class Inky : public Ghost {
public:
  Inky() : Ghost(2, Vector2{18 * 48, 22 * 48}) {}
  Vector2 chaseTarget(Pacman &pac) override {
    return Vector2Subtract(pac.getPos(),
                           Vector2Multiply(pac.getDir(), {48 * 4, 48 * 4}));
  }
};
class Clyde : public Ghost {
public:
  Clyde() : Ghost(3, Vector2{0, 22 * 48}) {}
  Vector2 chaseTarget(Pacman &pac) override {
    if (distance(getPos().x, getPos().y, pac.getPos().x, pac.getPos().y) <
        4 * 48)
      return scatter();
    return pac.getPos();
  }
};

Texture2D gui;
// Initialize the map
list<Effect> EffectManager::effectList;
map<EffectType, Texture2D> EffectMap::effectMap;
unordered_multiset<Animation *> AnimationManager::animations;

// ===========================================================================
//  Event mode: attract screen, HUD and results screen
//
//  These screens sit on top of the original game. Nothing about the maze,
//  movement or ghost AI changed; this only replaces the "press start and
//  figure it out" shell with one a walk-up stranger can use unassisted.
// ===========================================================================

namespace event {

// How many dots, power tiles and ghost tiles the current map started with.
int tilesAtRoundStart = 1;

int CountUneatenTiles() {
  int n = 0;
  for (int i = 0; i < 22; i++)
    for (int j = 0; j < 19; j++) {
      char c = prop[i][j];
      if (c == ' ' || c == '0' || c == 'P')
        n++;
    }
  return n;
}

// 0-100, measured against the board as it was when the round started so a
// custom map still reports sensibly.
int PercentCleared() {
  if (tilesAtRoundStart <= 0)
    return 0;
  int pct = 100 * (tilesAtRoundStart - CountUneatenTiles()) / tilesAtRoundStart;
  return pct < 0 ? 0 : (pct > 100 ? 100 : pct);
}

// A soft pulse used for anything that should feel alive but not distracting.
float Pulse(float speed = 3.0f) {
  return 0.5f + 0.5f * sinf((float)GetTime() * speed);
}

int CenterX() { return screenWidth / 2; }

// --- Attract ---------------------------------------------------------------

void DrawAttract(Pacman &pac) {
  // Dim the real maze so the screen never looks like a crashed game.
  DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 0, 0, 205});

  const int px = 300, py = 46, pw = 712, ph = 964;
  const int cx = px + pw / 2;
  Panel(px, py, pw, ph);

  // Title, with the real Pac-Man sprite chewing beside it.
  event::DrawCenteredTextShadow("PAC-MAN", cx, py + 26, 62, GOLD);
  int chomp = (int)(Pulse(6.0f) * 2.99f);
  DrawTexturePro(pac.Sprite(), Rectangle{(float)chomp * 16, 0, 16, 16},
                 Rectangle{(float)(cx - MeasureText("PAC-MAN", 62) / 2 - 96),
                           (float)(py + 52), 56, 56},
                 {0, 0}, 0, WHITE);
  DrawTexturePro(pac.Sprite(), Rectangle{(float)((chomp + 1) % 3) * 16, 0, 16, 16},
                 Rectangle{(float)(cx + MeasureText("PAC-MAN", 62) / 2 + 34),
                           (float)(py + 52), 56, 56},
                 {0, 0}, 0, WHITE);

  event::DrawCenteredText("Eat every dot.  Dodge the ghosts.", cx, py + 122, 22,
                          event::RGB(200, 200, 220));
  event::DrawCenteredText("CLEAR THE BOARD TO WIN", cx, py + 158, 30, GOLD);

  // How to play.
  DrawRectangle(px + 46, py + 208, pw - 92, 3, Color{255, 255, 255, 40});
  event::DrawCenteredText("HOW TO PLAY", cx, py + 224, 24, YELLOW);
  event::DrawCenteredText("W A S D   or   ARROW KEYS    move", cx, py + 262, 21,
                          WHITE);
  event::DrawCenteredText("SPACE    speed boost (limited uses)", cx, py + 294, 21,
                          WHITE);
  event::DrawCenteredText("Big dot    lets you eat a ghost", cx, py + 326, 21,
                          WHITE);

  DrawRectangle(px + 46, py + 366, pw - 92, 3, Color{255, 255, 255, 40});

  // Best scores, so there is a reason to come back for another go.
  event::DrawCenteredText("BEST SCORES", cx, py + 382, 24, YELLOW);
  const std::vector<int> &entries = session.board.Entries();
  if (entries.empty()) {
    event::DrawCenteredText("no scores yet - be the first", cx, py + 424, 20,
                            event::RGB(150, 150, 170));
  } else {
    int row = py + 418;
    int place = 1;
    for (int s : entries) {
      char line[64];
      snprintf(line, sizeof(line), "%d.      %d", place++, s);
      DrawText(line, cx - 110, row, 21, WHITE);
      row += 27;
      if (place > (int)kMaxScores)
        break;
    }
  }

  // Rewards - the reason to get close even without winning.
  event::DrawCenteredText("REVOLVES EARNED", cx, py + 570, 24, YELLOW);
  struct Row {
    const char *left;
    int amount;
  };
  static const Row rows[] = {
      {"Clear the whole board", 5},
      {"Eat 90% of the board", 3},
      {"Eat 70% of the board", 2},
      {"Eat 40% of the board", 1},
  };
  int ry = py + 606;
  for (const Row &r : rows) {
    char line[80];
    snprintf(line, sizeof(line), "%s  ........  %d", r.left, r.amount);
    DrawText(line, cx - 200, ry, 21, event::RGB(220, 220, 235));
    ry += 30;
  }

  // Call to action.
  float p = Pulse(4.0f);
  Color prompt = Color{(unsigned char)(255), (unsigned char)(255),
                       (unsigned char)(140 + 115 * p), 255};
  event::DrawCenteredTextShadow("PRESS ANY KEY TO PLAY", cx, py + 742, 40,
                                 prompt);
  event::DrawCenteredText("(or click anywhere)", cx, py + 790, 20,
                          event::RGB(140, 140, 165));

  // A row of the four ghosts, drifting gently, so the screen has motion.
  static const Color ghostColors[] = {RED, PINK, SKYBLUE, ORANGE};
  float t = (float)GetTime();
  for (int i = 0; i < 4; i++) {
    float wob = sinf(t * 2.0f + i) * 10.0f;
    DrawCircleV(Vector2{(float)(px + 110 + i * 140), (float)(py + ph - 60 + wob)},
                26.0f, ghostColors[i]);
    // Eyes.
    DrawCircleV(Vector2{(float)(px + 104 + i * 140), (float)(py + ph - 66 + wob)},
                6.0f, WHITE);
    DrawCircleV(Vector2{(float)(px + 120 + i * 140), (float)(py + ph - 66 + wob)},
                6.0f, WHITE);
    DrawCircleV(Vector2{(float)(px + 106 + i * 140), (float)(py + ph - 66 + wob)},
                3.0f, event::RGB(20, 20, 40));
    DrawCircleV(Vector2{(float)(px + 122 + i * 140), (float)(py + ph - 66 + wob)},
                3.0f, event::RGB(20, 20, 40));
  }
}

// --- Heads-up display ------------------------------------------------------

void DrawHUD(Pacman &pac, int livesLeft) {
  // Score and lives in the left gutter, where the original game put them.
  char scoreLine[64];
  snprintf(scoreLine, sizeof(scoreLine), "SCORE %d", score);
  DrawText(scoreLine, 8, 8, 26, WHITE);

  for (int i = 0; i < livesLeft && i < 5; i++)
    DrawCircleV(Vector2{26.0f + (float)i * 40.0f, 78.0f}, 15.0f, GOLD);

  // Board progress, so "getting close" is visible while playing.
  int pct = PercentCleared();
  char progLine[64];
  snprintf(progLine, sizeof(progLine), "BOARD %d%%", pct);
  DrawText(progLine, 8, 116, 22, event::RGB(190, 190, 210));
  DrawRectangleRec(Rectangle{8.0f, 148.0f, 168.0f, 16.0f}, event::RGB(40, 40, 60));
  DrawRectangleRec(Rectangle{8.0f, 148.0f, 168.0f * pct / 100.0f, 16.0f}, LIME);

  // Countdown. Pulses red and beeps through the last ten seconds.
  bool urgent = event::session.roundLeft <= 10.0f;
  float p = Pulse(8.0f);
  Color timeColor = urgent
                        ? Color{(unsigned char)(255),
                                (unsigned char)(90 + 80 * p),
                                (unsigned char)(90), 255}
                        : WHITE;
  int secs = (int)ceilf(event::session.roundLeft);
  char timeLine[32];
  snprintf(timeLine, sizeof(timeLine), "%d", secs);

  // The countdown sits on top of the maze, which is far too busy to read it
  // against, so give it its own solid plate.
  const int plateW = 260, plateH = 100;
  DrawRectangleRec(
      Rectangle{(float)((screenWidth - plateW) / 2), 0.0f, (float)plateW,
                (float)plateH},
      event::RGB(10, 10, 24, 225));
  DrawRectangleLinesEx(
      Rectangle{(float)((screenWidth - plateW) / 2), 0.0f, (float)plateW,
                (float)plateH},
      2, urgent ? event::RGB(255, 90, 90) : event::RGB(255, 255, 255, 60));

  int tw = MeasureText(timeLine, 54);
  DrawText(timeLine, (screenWidth - tw) / 2, 12, 54, timeColor);
  DrawText("SECONDS LEFT", (screenWidth - MeasureText("SECONDS LEFT", 18)) / 2,
           70, 18, event::RGB(180, 180, 200));

  if (urgent && secs != event::session.lastTickSecond && secs > 0) {
    event::session.lastTickSecond = secs;
    PlaySound(pac.Chomp());
  }
}

// --- Results ---------------------------------------------------------------

void DrawResults() {
  DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 0, 0, 210});

  const int px = 366, py = 120, pw = 580, ph = 744;
  const int cx = px + pw / 2;
  Panel(px, py, pw, ph);

  const char *outcome;
  if (session.reason == EndReason::ClearedBoard)
    outcome = "BOARD CLEARED!";
  else if (session.reason == EndReason::OutOfLives)
    outcome = "OUT OF LIVES";
  else
    outcome = "TIME UP";

  Color accent = session.reward.accent;
  Color lit = event::RGB(accent.r, accent.g, accent.b);
  event::DrawCenteredTextShadow(outcome, cx, py + 22, 42, lit);
  event::DrawCenteredText(session.reward.detail, cx, py + 76, 20,
                          event::RGB(190, 190, 210));

  // The payout, as a badge. This is the bit the event actually cares about.
  int badgeY = py + 200;
  int badgeR = 90;
  DrawCircleV(Vector2{(float)cx, (float)badgeY}, (float)badgeR,
              Color{accent.r, accent.g, accent.b, 60});
  DrawCircleLinesV(Vector2{(float)cx, (float)badgeY}, (float)badgeR, lit);
  char amount[16];
  snprintf(amount, sizeof(amount), "%d", session.reward.revolves);
  int aw = MeasureText(amount, 76);
  DrawText(amount, cx - aw / 2, badgeY - 44, 76, lit);
  event::DrawCenteredText("REVOLVES", cx, badgeY + badgeR + 2, 22,
                          event::RGB(210, 210, 230));

  // What they scored.
  DrawRectangle(px + 40, py + 338, pw - 80, 3, Color{255, 255, 255, 40});
  char line[96];
  snprintf(line, sizeof(line), "SCORE            %d", session.finalScore);
  DrawText(line, px + 70, py + 360, 24, WHITE);
  snprintf(line, sizeof(line), "BOARD EATEN      %d%%", session.percentCleared);
  DrawText(line, px + 70, py + 398, 24, WHITE);
  snprintf(line, sizeof(line), "BEST             %d", session.board.Best());
  DrawText(line, px + 70, py + 436, 24, event::RGB(190, 190, 210));

  if (session.newBest) {
    float sp = Pulse(6.0f);
    event::DrawCenteredText(
        "NEW HIGH SCORE!", cx, py + 486,
        30, Color{(unsigned char)(255), (unsigned char)(230),
                  (unsigned char)(80 + 100 * sp), 255});
  }

  // Play again, or wait for the screen to reset itself.
  int left = (int)ceilf(session.resultsLeft);
  snprintf(line, sizeof(line), "back to the menu in %d", left);
  event::DrawCenteredText(line, cx, py + 546, 20, event::RGB(150, 150, 170));
  float fill = session.resultsLeft / kResultsSeconds;
  DrawRectangleRec(Rectangle{(float)(px + 60), (float)(py + 580),
                             (float)(pw - 120), 10},
                   event::RGB(50, 50, 70));
  DrawRectangleRec(Rectangle{(float)(px + 60), (float)(py + 580),
                             (float)(pw - 120) * fill, 10},
                   lit);

  event::DrawCenteredTextShadow("PRESS ANY KEY", cx, py + 626, 34, WHITE);
}

}  // namespace event

int main() {
  AttachParentConsole();

  // Every texture, sound and the default map are loaded from "./assets",
  // relative to the working directory. Report that plainly instead of
  // segfaulting on an empty map when the folder is missing.
  if (!FileExists("./assets/defaultMap.bin")) {
    cerr << "Error: could not find ./assets/defaultMap.bin." << endl;
    cerr << "Run the game from the folder that contains it - the 'assets'"
         << " directory has to sit next to the executable." << endl;
    return EXIT_FAILURE;
  }

  InitWindow(screenWidth, screenHeight, "PAC-MAN");
  InitAudioDevice();
  SetTargetFPS(60);

  Texture2D map = LoadTexture("./assets/tileSheet.png");
  Helper::LoadIcons();
  gui = LoadTexture("./assets/GUI.png");
  SmallButton Pause({8, screenHeight / 2}, &gui);
  SmallButton EditButton({8, screenHeight / 2 + 6 * 16}, &gui,
                         ICON_TYPES::LOCK);
  Music bg_music = LoadMusicStream("./assets/sound/pacman_remix.mp3");
  SetMusicVolume(bg_music, 0.7);
  PlayMusicStream(bg_music);
  prop = loadVector("./assets/defaultMap.bin");
  MapHandler MapEdit(&gui);

  Pacman pac;
  Blinky Blinky;
  Pinky Pinky;
  Inky Inky;
  Clyde Clyde;
  EffectMap::Init();
  Ghost *ghosts[] = {&Blinky, &Pinky, &Inky, &Clyde};

  bool win = false;
  float hitPause = 0.0f;
  int livesBefore = Lives;

  event::session.Begin();

  auto resetBoard = [&]() {
    prop = loadVector("./assets/defaultMap.bin");
    score = 0;
    Lives = event::kStartingLives;
    Power = 3;
    win = false;
    hitPause = 0.0f;
    livesBefore = Lives;
    pac.Reset();
    for (int i = 0; i < 4; i++)
      ghosts[i]->Reset();
    event::tilesAtRoundStart = event::CountUneatenTiles();
  };

  while (!WindowShouldClose()) {
    BeginDrawing();
    UpdateMusicStream(bg_music);
    ClearBackground(BLACK);

    if (!event::kEnabled) {
      // Original single-session behaviour, kept intact.
      DrawMap(map);
      Pause.Draw();
      Pause.Update();
      EditButton.Draw();
      EditButton.Update();
      DrawText(("Score:" + to_string(score)).c_str(), 8, 5, 30, WHITE);
      DrawText(("Lives:" + to_string(Lives)).c_str(), 8, 30, 30, WHITE);
      pac.Draw();
      for (int i = 0; i < 4; i++)
        ghosts[i]->Draw();
      pac.Control();
      if (EditButton.getPressed())
        MapEdit.MapEdit();
      if (!win && !Pause.getPressed()) {
        pac.Update();
        for (int i = 0; i < 4; i++) {
          ghosts[i]->Update(pac);
          if (pac.getEaten())
            ghosts[i]->setState(FRIGHTENED);
          else if (pac.killMode())
            ghosts[i]->setState(EATEN);
        }
        if (pac.getEaten())
          pac.setEaten(false);
        else if (pac.killMode())
          pac.killMode(false);
        if (score == 3730)
          win = true;
      }
      if (win)
        DrawText("YOU WIN", (screenWidth - 120) / 2, screenHeight / 2, 30, WHITE);
      EffectManager::updateEffects();
      AnimationManager::Draw();
      AnimationManager::Update();
      EndDrawing();
      continue;
    }

    event::Change change = event::session.Tick(score, event::PercentCleared());

    if (change == event::Change::StartRound) {
      resetBoard();
    }

    if (change == event::Change::FinishRound) {
      event::session.Finish(event::EndReason::TimeUp, score, event::PercentCleared());
    }

    switch (event::session.screen) {
      case event::Screen::Attract: {
        DrawMap(map);
        event::DrawAttract(pac);
        break;
      }

      case event::Screen::Playing: {
        DrawMap(map);
        // The editor and its buttons are hidden here: a stranger clicking
        // around should never end up editing or saving a map.
        pac.Draw();
        for (int i = 0; i < 4; i++)
          ghosts[i]->Draw();
        event::DrawHUD(pac, Lives);
        EffectManager::updateEffects();
        AnimationManager::Draw();
        AnimationManager::Update();
        break;
      }

      case event::Screen::Results: {
        DrawMap(map);
        pac.Draw();
        for (int i = 0; i < 4; i++)
          ghosts[i]->Draw();
        EffectManager::updateEffects();
        AnimationManager::Draw();
        AnimationManager::Update();
        event::DrawResults();
        break;
      }
    }

    // Simulation only runs while a round is live.
    if (event::session.screen == event::Screen::Playing) {
      if (hitPause > 0.0f) {
        hitPause -= GetFrameTime();
        if (fmod((float)GetTime(), 0.24f) < 0.12f)
          DrawText("OUCH!", (screenWidth - MeasureText("OUCH!", 48)) / 2,
                   screenHeight / 2 - 60, 48, RED);
      } else {
        pac.Control();
        pac.Update();
        for (int i = 0; i < 4; i++) {
          ghosts[i]->Update(pac);
          if (pac.getEaten())
            ghosts[i]->setState(FRIGHTENED);
          else if (pac.killMode())
            ghosts[i]->setState(EATEN);
        }
        if (pac.getEaten())
          pac.setEaten(false);
        else if (pac.killMode())
          pac.killMode(false);

        // A ghost got us.
        if (Lives < livesBefore) {
          livesBefore = Lives;
          hitPause = 1.1f;
          if (Lives <= 0)
            event::session.Finish(event::EndReason::OutOfLives, score,
                           event::PercentCleared());
        }

        // Every dot eaten. The original compared scores with ==, which skipped
        // the target whenever a ghost was eaten first and so made winning
        // nearly impossible.
        if (!win && score >= event::kScoreToClearBoard) {
          win = true;
          event::session.Finish(event::EndReason::ClearedBoard, score, 100);
        }
      }
    }

    EndDrawing();

  }

  UnloadMusicStream(bg_music);
  CloseWindow();
  return 0;
}