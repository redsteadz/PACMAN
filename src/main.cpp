#include <raylib.h>
#include <iostream>
#include <cmath>
using namespace std;

char prop[22][19];

void map(Image map)
{
    for (int i = 0; i < 22; i++)
    {
        for (int j = 0; j < 19; j++)
        {
            int c = (j * 48) + 24;
            int r = (i * 48) + 24;
            Color color = GetImageColor(map, c, r);
            int hex = ColorToInt(color) % 7;

            if (hex == 6)
            {
                prop[i][j] = '#';
            }
            else
            {
                prop[i][j] = '.';
            }
            cout << prop[i][j] << " ";
        }
        cout << endl;
    }
}

class pacman
{
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
    pacman()
    {
        x = 9 * 48;
        y = 16 * 48;
        speed = 5;
        dX = -1;
        pdX = -1;
        dY = 0;
        pdY = 0;
        pac_left = LoadImage("./assets/pac_left.png");
        pacLeft = LoadTextureFromImage(pac_left);
    }

    void check()
    {
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

    void update()
    {
        check();
        x += dX * speed;
        y += dY * speed;
        int centered_x = (x + 24);
        int centered_y = (y + 24);
        int dx[] = {-1, +1, 0, 0};
        int dy[] = {0, 0, -1, +1};
        for (int i = 0; i < 4; i++)
        {
            if (prop[(centered_y + 19 * dy[i]) / 48][(centered_x + 19 * dx[i]) / 48] == '#')
            {
                x -= dx[i] * speed;
                y -= dy[i] * speed;
            }
        }
    }

    void draw()
    {
        if (dX == 0 && dY == -1)
        {
            DrawTexturePro(pacLeft, Rectangle{0, 0, 48, 48}, Rectangle{(float)x, (float)y, 48, 48}, Vector2{0, 48}, 90.0f, WHITE);
        }
        if (dX == 0 && dY == 1)
        {
            DrawTexturePro(pacLeft, Rectangle{0, 0, 48, 48}, Rectangle{(float)x, (float)y, 48, 48}, Vector2{48, 0}, -90.0f, WHITE);
        }
        if (dX == -1 && dY == 0)
        {
            DrawTexturePro(pacLeft, Rectangle{0, 0, 48, 48}, Rectangle{(float)x, (float)y, 48, 48}, Vector2{0, 0}, 0, WHITE);
        }
        if (dX == 1 && dY == 0)
        {
            DrawTexturePro(pacLeft, Rectangle{0, 0, 48, 48}, Rectangle{(float)x, (float)y, 48, 48}, Vector2{48, 48}, 180.0f, WHITE);
        }
    }

    void control()
    {
        if (IsKeyPressed(KEY_W))
        {
            pdX = 0;
            pdY = -1;
        }
        else if (IsKeyPressed(KEY_A))
        {
            pdX = -1;
            pdY = 0;
        }
        else if (IsKeyPressed(KEY_S))
        {
            pdX = 0;
            pdY = 1;
        }
        else if (IsKeyPressed(KEY_D))
        {
            pdX = 1;
            pdY = 0;
        }
    }
};

class ghost
{
    int x;
    int y;
    int speed;
    int dX;
    int dY;
    // int pdX;
    // int pdY;
    Image ghostI;
    Texture2D ghostT;

public:
    ghost()
    {
        x = 9 * 48;
        y = 10 * 48;
        speed = 4;
        dX = 1;
        dY = -1;
        ghostI = LoadImage("./assets/red_right.png");
        ghostT = LoadTextureFromImage(ghostI);
    }

    void draw()
    {
        DrawTexture(ghostT, x, y, WHITE);
    }

    void move()
    {
        x += dX;
        y += dY;
    }
};

int main()
{

    const int screenWidth = 912;
    const int screenHeight = 1056;

    InitWindow(screenWidth, screenHeight, "My first RAYLIB program!");
    Image Maze_img = LoadImage("./assets/map.png");
    Texture2D Maze = LoadTexture("./assets/map.png");

    cout << Maze.height << " " << Maze.width << endl;
    SetTargetFPS(60);

    map(Maze_img);

    pacman pac;
    ghost gst;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);
        DrawTexture(Maze, 0, 0, WHITE);
        gst.draw();
        gst.move();
        pac.draw();
        pac.control();
        pac.update();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}