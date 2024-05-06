#include <bits/stdc++.h>
#include <cmath>
#include <iostream>
#include <raylib.h>
#define INF INT_MAX
using namespace std;

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
class pacman {
  int x;
  int y;
  int speed;
  int dX;
  int dY;
  int pdX;
  int pdY;
  Image pac_left;
  Texture2D pacLeft;

public:
  pacman() {
    x = 9 * 48;
    y = 16 * 48;
    prop[16][9] = 'P';

    speed = 5;
    dX = -1;
    pdX = -1;
    dY = 0;
    pdY = 0;
    pac_left = LoadImage("../assets/pac_left.png");
    pacLeft = LoadTextureFromImage(pac_left);
  }

  Vector2 getPos() {
    // cout << x/48 << " " << y/48 << endl;
    Vector2 pos = {(float)x, (float)y};
    return pos;
  }

  void check() {
    if (dX == pdX && dY == pdY)
      return;
    int xx = x + pdX * speed;
    int yy = y + pdY * speed;
    int centered_x = (xx + 24);
    int centered_y = (yy + 24);
    if (prop[(centered_y + 24 * pdY) / 48][(centered_x + 24 * pdX) / 48] == '#')
      return;
    dX = pdX;
    dY = pdY;
  }

  void update() {
    check();
    int i = (y + 24) / 48;
    int j = (x + 24) / 48;
    prop[i][j] = '.';
    x += dX * speed;
    y += dY * speed;
    int centered_x = (x + 24);
    int centered_y = (y + 24);
    int dx[] = {-1, +1, 0, 0};
    int dy[] = {0, 0, -1, +1};
    for (int i = 0; i < 4; i++) {
      if (prop[(centered_y + 19 * dy[i]) / 48]
              [(centered_x + 19 * dx[i]) / 48] == '#') {
        x -= dx[i] * speed;
        y -= dy[i] * speed;
      }
    }
    i = (y + 24) / 48;
    j = (x + 24) / 48;
    // Update on grid
    prop[i][j] = 'P';
    // cout << y / 48 << " " << x / 48 << endl;
  }

  void draw() {
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
    cout << (x + 24) / 48 << " " << (y + 24) / 48 << endl;
    DrawMap();
  }

  void control() {
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

const int dx[] = { 0, -1, 1, 0 };
const int dy[] = { -1, 0, 0, 1 };;

class ghost {
  int x;
  int y;
  int speed;
  int dX;
  int dY;
  int pdX;
  int pdY;
  Image ghostI;
  Texture2D ghostT;

public:
  ghost() {
    x = 10 * 48;
    y = 8 * 48;
    speed = 4;
    ghostI = LoadImage("../assets/red_right.png");
    ghostT = LoadTextureFromImage(ghostI);
    speed = 4;
    dX = 1;
    dY = 0;
  }
  void check() {
    if ((dX == pdX && dY == pdY))
      return;
    int xx = x + pdX * speed;
    int yy = y + pdY * speed;
    int centered_x = (xx + 24);
    int centered_y = (yy + 24);
    if (prop[(centered_y + 24 * pdY) / 48][(centered_x + 24 * pdX) / 48] == '#')
      return;
    dX = pdX;
    dY = pdY;
  }

  void draw() { DrawTexture(ghostT, x, y, WHITE); }

  void next(Vector2 pac) {
    // Of all the directions possible, Return the direction that is NOT opposite to current AND allows the ghost to be closest  
    int centered_x = (x + 24);
    int centered_y = (y + 24);
    int centered_x_grid = (x + 24)/48;
    int centered_y_grid = (y + 24)/48;
    int target_x = (pac.x + 24)/48;
    int target_y = (pac.y + 24)/48;
    vector<pair<int, float>> distances; 
    for (int i = 0; i < 4; i++){
      // Make sure it is in bounds 
      if (dx[i] == -dX && dy[i] == -dY) continue;
      if (isNotValid((centered_x + dx[i]*24)/48, (centered_y + dy[i]*24)/48, 22, 19)) continue;
      // Calculate distance 
      distances.push_back({i, distance(centered_x_grid + dx[i] , centered_y_grid + dy[i], target_x , target_y)});
    }
    sort(distances.begin(), distances.end(), [](pair<int, float> a, pair<int, float> b){ return a.second < b.second;});
    dX = dx[distances[0].first];
    dY = dy[distances[0].first];
  }
  float distance(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
  }
  void move() {}
  void Update(Vector2 pac) {
    next(pac);
    x += dX * speed;
    y += dY * speed;
        int centered_x = (x + 24);
    int centered_y = (y + 24);
    int dx[] = {-1, +1, 0, 0};
    int dy[] = {0, 0, -1, +1};
    for (int i = 0; i < 4; i++) {
      if (prop[(centered_y + 19 * dy[i]) / 48]
              [(centered_x + 19 * dx[i]) / 48] == '#') {
        x -= dx[i] * speed;
        y -= dy[i] * speed;
      }
    }

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
  Texture2D Maze = LoadTexture("../assets/map.png");

  cout << Maze.height << " " << Maze.width << endl;
  SetTargetFPS(60);

  Map(Maze_img);

  pacman pac;
  ghost gst;

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);
    DrawTexture(Maze, 0, 0, WHITE);

    pac.draw();
    pac.control();
    pac.update();
    gst.draw();
    gst.Update(pac.getPos());
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
