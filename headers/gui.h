#include <iostream>
#include <raylib.h>
#include <unordered_map>
#define MAX_INPUT_CHARS 32
using namespace std;

// TODO: Make a generic Button class

enum ICON_TYPES {
  PAUSE,
  LOCK,
  CHECK,
  MAIL,
};

static unordered_map<ICON_TYPES, Rectangle> IconsRectangle;

class Helper {
public:
  static void LoadIcons() {
    IconsRectangle[PAUSE] = {6 * 16, 5 * 16, 16, 16};
    IconsRectangle[LOCK] = {6 * 16, 4 * 16, 16, 16};
    IconsRectangle[CHECK] = {4 * 16, 4 * 16, 16, 16};
    IconsRectangle[MAIL] = {5 * 16, 4 * 16, 16, 16};
  }
};

class TextBox {
  char input[MAX_INPUT_CHARS + 1] =
      "\0"; // NOTE: One extra space required for null terminator char '\0'
  int letterCount = 0;
  Rectangle textBox;
  bool mouseOnText = false;
  int framesCounter = 0;

public:
  string getInput() { return input; }
  TextBox(Rectangle rect) { textBox = rect; }
  void Update() {
    if (CheckCollisionPointRec(GetMousePosition(), textBox))
      mouseOnText = true;
    else
      mouseOnText = false;

    if (mouseOnText) {
      // Set the window's cursor to the I-Beam
      SetMouseCursor(MOUSE_CURSOR_IBEAM);

      // Get char pressed (unicode character) on the queue
      int key = GetCharPressed();

      // Check if more characters have been pressed on the same frame
      while (key > 0) {
        // NOTE: Only allow keys in range [32..125]
        if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS)) {
          input[letterCount] = (char)key;
          input[letterCount + 1] = '\0';
          letterCount++;
        }

        key = GetCharPressed();
      }

      if (IsKeyPressed(KEY_BACKSPACE)) {
        letterCount--;
        if (letterCount < 0)
          letterCount = 0;
        input[letterCount] = '\0';
      }
    } else
      SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    if (mouseOnText)
      framesCounter++;
    else
      framesCounter = 0;
  }

  void Draw() {
    // DrawText("PLACE MOUSE OVER INPUT BOX!", 240, 140, 20, GRAY);

    DrawRectangleRec(textBox, LIGHTGRAY);
    if (mouseOnText)
      DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width,
                         (int)textBox.height, RED);
    else
      DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width,
                         (int)textBox.height, WHITE);

    DrawText(input, (int)textBox.x + 5, (int)textBox.y + 8, 40, MAROON);

    // DrawText(TextFormat("INPUT CHARS: %i/%i", letterCount, MAX_INPUT_CHARS),
    //          315, 250, 20, DARKGRAY);

    if (mouseOnText) {
      if (letterCount < MAX_INPUT_CHARS) {
        // Draw blinking underscore char
        if (((framesCounter / 20) % 2) == 0)
          DrawText("_", (int)textBox.x + 8 + MeasureText(input, 40),
                   (int)textBox.y + 12, 40, MAROON);
      } else
        DrawText("Press BACKSPACE to delete chars...", 230, 300, 20, GRAY);
    }
  };
};

  class Button {
  protected:
    bool pressed;
    Rectangle src_button;
    Texture2D *gui;
    Rectangle dst;

  public:
    Button(Vector2 pos, Rectangle src_button, Texture2D *gui)
        : src_button(src_button), gui(gui) {
      dst = {pos.x, pos.y, src_button.width * 6, src_button.height * 6};
      pressed = false;
    }
    virtual void Draw() = 0;
    virtual void onClick() = 0;
    virtual void Update() {
      if (CheckCollisionPointRec(GetMousePosition(), dst) &&
          IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        pressed = !pressed;
        // cout << "Button pressed" << pressed <<endl;
      }
    }
    void setPressed(bool p) { pressed = p; }
    bool getPressed() { return pressed; }
  };

  class SmallButton : public Button {
    Rectangle src_icon;

  public:
    SmallButton(Vector2 pos, Texture2D *gui, ICON_TYPES type = PAUSE)
        : Button(pos, {0, 5 * 16, 16, 16}, gui) {
      src_icon = IconsRectangle[type];
      cout << src_icon.x << " " << src_icon.y << " " << src_icon.width << " "
           << src_icon.height << endl;
    }
    void Draw() {
      if (pressed)
        src_button.x = 16;
      else
        src_button.x = 0;
      DrawTexturePro(*gui, src_button, dst, {0, 0}, 0, WHITE);
      DrawTexturePro(
          *gui, src_icon,
          Rectangle{dst.x, dst.y - (pressed ? -2 : 0), dst.width, dst.height},
          {0, 0}, 0, WHITE);
    }
    void onClick() {}
  };
