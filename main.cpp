#include <bits/stdc++.h>
#include <cmath>
#include <iostream>
#include <raylib.h>
#define INF INT_MAX
using namespace std;

// Make it Private and not globally Available
vector<vector<char>> prop(22, vector<char>(19, '.'));

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

const int dx[] = {0, -1, 1, 0};
const int dy[] = {-1, 0, 0, 1};

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
        prop[i][j] = '.';
      }
      // cout << prop[i][j] << " ";
    }
    // cout << endl;
  }
}

void DrawMap() {
  for (int i = 0; i < 22; i++) {
    for (int j = 0; j < 19; j++) {
      cout << prop[i][j] << " ";
    }
    cout << endl;
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

public:
  void Check() {
    if (dX == pdX && dY == pdY)
      return;

    // Check all 4 corners are not in a wall
    // 47 47, 0 47, 47 0
    int dx[] = {0, 1, 0, 1};
    int dy[] = {0, 1, 1, 0};

    for (int i = 0; i < 4; i++) {
      int xx = x + 3 + pdX * speed;
      int yy = y + 3 + pdY * speed;

      int grid_x = (xx + 42 * dx[i]) / 48;
      int grid_y = (yy + 42 * dy[i]) / 48;
      DrawCircle(xx + 42 * dx[i], yy + 42 * dy[i], 5, RED);
      cout << grid_x << " " << grid_y << endl;
      if (prop[grid_y][grid_x] == '#')
        return;
    }

    dX = pdX;
    dY = pdY;
  }
  Vector2 getPos() {
    // cout << x/48 << " " << y/48 << endl;
    Vector2 pos = {(float)x, (float)y};
    return pos;
  }
  void virtual Update() = 0;
};

// Capitalize this class
class Pacman : public Entity {
  Image pac_left;
  Texture2D pacLeft;

public:
  Pacman(): Entity() {
    x = 9 * 48;
    y = 16 * 48;
    prop[16][9] = 'P';

    speed = 5;
    dX = 0;
    pdX = -1;
    dY = 0;
    pdY = 0;
    pac_left = LoadImage("../assets/pac_left.png");
    pacLeft = LoadTextureFromImage(pac_left);
  }

  void Update() {
    Check();
    int centered_x = x + 24;
    int centered_y = y + 24;
    int i = (y + 24) / 48.0;
    int j = (x + 24) / 48.0;
    // If Right or bottom then floor
    prop[i][j] = '.';

    DrawCircle(x + 24, y + 24, 5, RED);
    cout << i + dY << " " << j + dX << endl;
    if (prop[(centered_y + 26 * dY) / 48][(centered_x + 26 * dX) / 48] != '#') {
      DrawCircle(centered_x + 25 * dX, centered_y + 25 * dY, 5, RED);
      x += dX * speed;
      y += dY * speed;
    }
    i = (y + 24) / 48.0;
    j = (x + 24) / 48.0;
    // Update on grid
    prop[i][j] = 'P';
    // cout << y / 48 << " " << x / 48 << endl;
  }

  void Draw() {
    if (dX == 0 && dY == -1) {
      DrawTexturePro(pacLeft, Rectangle{0, 0, 48, 48},
                     Rectangle{(float)x, (float)y, 48, 48}, Vector2{0, 48},
                     90.0f, WHITE);
    }
    if (dX == 0 && dY == 1) {
      DrawTexturePro(pacLeft, Rectangle{0, 0, 48, 48},
                     Rectangle{(float)x, (float)y, 48, 48}, Vector2{48, 0},
                     -90.0f, WHITE);
    }
    if (dX == -1 && dY == 0) {
      DrawTexturePro(pacLeft, Rectangle{0, 0, 48, 48},
                     Rectangle{(float)x, (float)y, 48, 48}, Vector2{0, 0}, 0,
                     WHITE);
    }
    if (dX == 1 && dY == 0) {
      DrawTexturePro(pacLeft, Rectangle{0, 0, 48, 48},
                     Rectangle{(float)x, (float)y, 48, 48}, Vector2{48, 48},
                     180.0f, WHITE);
    }
  }

  void Control() {
    if (IsKeyPressed(KEY_W)) {
      pdX = 0;
      pdY = -1;
    } else if (IsKeyPressed(KEY_A)) {
      pdX = -1;
      pdY = 0;
    } else if (IsKeyPressed(KEY_S)) {
      pdX = 0;
      pdY = 1;
    } else if (IsKeyPressed(KEY_D)) {
      pdX = 1;
      pdY = 0;
    }
  }
};

class Ghost : public Entity {
  Image ghostI;
  Texture2D ghostT;

public:
  Ghost() {
    x = 10 * 48;
    y = 8 * 48;
    speed = 4;
    ghostI = LoadImage("../assets/red_right.png");
    ghostT = LoadTextureFromImage(ghostI);
    speed = 4;
    dX = 1;
    dY = 0;
  }

  void Draw() { DrawTexture(ghostT, x, y, WHITE); }

  void next(Vector2 pac) {
    // Of all the directions possible, Return the direction that is NOT opposite
    // to current AND allows the Ghost to be closest
    int centered_x = (x + 24);
    int centered_y = (y + 24);
    int centered_x_grid = (x + 24) / 48;
    int centered_y_grid = (y + 24) / 48;
    int target_x = (pac.x + 24) / 48;
    int target_y = (pac.y + 24) / 48;
    vector<pair<int, float>> distances;
    for (int i = 0; i < 4; i++) {
      // Make sure it is in bounds
      if (dx[i] == -dX && dy[i] == -dY)
        continue;
      if (isNotValid((centered_x + dx[i] * 24) / 48,
                     (centered_y + dy[i] * 24) / 48, 22, 19))
        continue;
      if (prop[(centered_y + 26 * dy[i]) / 48]
              [(centered_x + 26 * dx[i]) / 48] == '#')
        continue;
      // Calculate distance
      distances.push_back(
          {i, distance(centered_x_grid + dx[i], centered_y_grid + dy[i],
                       target_x, target_y)});
    }
    sort(distances.begin(), distances.end(),
         [](pair<int, float> a, pair<int, float> b) {
           return a.second < b.second;
         });
    pdX = dx[distances[0].first];
    pdY = dy[distances[0].first];
  }
  float distance(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
  }
  void Update()override{}
  void Update(Vector2 pac) {
    next(pac);
    Check();
    int centered_x = x + 24;
    int centered_y = y + 24;
    int i = (y + 24) / 48.0;
    int j = (x + 24) / 48.0;
    // If Right or bottom then floor

    DrawCircle(x + 24, y + 24, 5, RED);
    cout << i + dY << " " << j + dX << endl;
    if (prop[(centered_y + 26 * dY) / 48][(centered_x + 26 * dX) / 48] != '#') {
      DrawCircle(centered_x + 25 * dX, centered_y + 25 * dY, 5, RED);
      x += dX * speed;
      y += dY * speed;
    }
    i = (y + 24) / 48.0;
    j = (x + 24) / 48.0;
    // Update on grid
    // cout << y / 48 << " " << x / 48 << endl;
  }
};

class Game {

public:
};

int main() {

  const int screenWidth = 912;
  const int screenHeight = 1056;

  InitWindow(screenWidth, screenHeight, "My first RAYLIB program!");
  Image Maze_img = LoadImage("../assets/map.png");
  Color *img = LoadImageColors(Maze_img);
  Texture2D Maze = LoadTexture("../assets/map.png");
  cout << Maze.height << " " << Maze.width << endl;
  SetTargetFPS(60);

  Map(Maze_img);

  Pacman pac;
  Ghost gst;

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);
    DrawTexture(Maze, 0, 0, WHITE);

    pac.Draw();
    pac.Control();
    pac.Update();
    gst.Draw();
    gst.Update(pac.getPos());
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
