#pragma once

// ===========================================================================
//  Event / arcade mode
//
//  Turns the game into something a walk-up stranger can understand and enjoy
//  without anyone explaining it:
//
//    ATTRACT  -> animated title, how-to-play and best scores, idle forever
//    PLAYING  -> one short timed round, big countdown, lives shown as pips
//    RESULTS  -> outcome, score, and how many "revolves" the run earned
//
//  Every screen returns to the attract screen by itself, so the machine always
//  resets for the next player. Nothing here replaces the original game: set
//  kEnabled to false and the classic single-session behaviour comes back.
//
//  In-game text is ASCII only - raylib's built-in font has no other glyphs.
// ===========================================================================

#include "raylib.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace event {

// ---------------------------------------------------------------------------
// Tunables - these are the knobs to turn for your event.
// ---------------------------------------------------------------------------

inline constexpr bool kEnabled = true;

// How long one round lasts. Keep it short so a queue keeps moving.
inline constexpr float kRoundSeconds = 90.0f;

// Results screen holds this long, then returns to attract on its own.
inline constexpr float kResultsSeconds = 15.0f;

// Pause on a screen after a transition so one keypress cannot skip two screens.
inline constexpr float kInputLockSeconds = 0.6f;

inline constexpr int kStartingLives = 3;

// Every pellet in assets/defaultMap.bin, ignoring ghosts: 178 * 20 + 4 * 40
// + 10. Clearing this many points means the board is finished.
inline constexpr int kScoreToClearBoard = 3730;

// Where best scores are remembered. The directory only has to be writable; if
// it is not, scores simply stay in memory for this session.
inline const char *const kScoreFile = "pacman_scores.txt";

inline constexpr int kMaxScores = 5;

// ---------------------------------------------------------------------------
// Rewards
//
// The brief was that winning, or getting close to winning, should earn
// something. Percentages are of the board cleared, so a strong but unfinished
// run still pays out.
// ---------------------------------------------------------------------------

struct Reward {
  int revolves = 0;
  const char *headline = "KEEP GOING!";
  const char *detail = "";
  Color accent = LIGHTGRAY;
};

inline Reward EvaluateReward(int percentCleared, bool clearedBoard) {
  Reward r;
  if (clearedBoard) {
    r.revolves = 5;
    r.headline = "BOARD CLEARED!";
    r.detail = "You finished the maze.";
    r.accent = GOLD;
  } else if (percentCleared >= 90) {
    r.revolves = 3;
    r.headline = "SO CLOSE!";
    r.detail = "Nearly the whole board.";
    r.accent = YELLOW;
  } else if (percentCleared >= 70) {
    r.revolves = 2;
    r.headline = "NICE RUN!";
    r.detail = "Strong progress.";
    r.accent = SKYBLUE;
  } else if (percentCleared >= 40) {
    r.revolves = 1;
    r.headline = "GOOD TRY!";
    r.detail = "Plenty left for next time.";
    r.accent = LIME;
  } else {
    r.revolves = 0;
    r.headline = "WARM UP!";
    r.detail = "The maze is waiting for you.";
    r.accent = LIGHTGRAY;
  }
  return r;
}

// ---------------------------------------------------------------------------
// Best scores
// ---------------------------------------------------------------------------

class Leaderboard {
  std::vector<int> scores;

public:
  void Load(const char *path) {
    std::ifstream in(path);
    int s = 0;
    while (in >> s)
      scores.push_back(s);
    Sort();
  }

  void Save(const char *path) const {
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open())
      return;  // read-only install directory: scores stay for this session
    for (int s : scores)
      out << s << "\n";
  }

  // Returns true when the score made the board.
  bool Submit(int score) {
    scores.push_back(score);
    Sort();
    if ((int)scores.size() > kMaxScores)
      scores.resize(kMaxScores);
    return Best() == score;
  }

  int Best() const { return scores.empty() ? 0 : scores.front(); }

  const std::vector<int> &Entries() const { return scores; }

  int Rank(int score) const {
    for (size_t i = 0; i < scores.size(); i++)
      if (scores[i] == score)
        return (int)i + 1;
    return 0;
  }

private:
  void Sort() {
    std::sort(scores.begin(), scores.end(), std::greater<int>());
  }
};

// ---------------------------------------------------------------------------
// Screens
// ---------------------------------------------------------------------------

enum class Screen { Attract, Playing, Results };

enum class EndReason { TimeUp, ClearedBoard, OutOfLives };

// What changed since the last Tick(), so the game knows when to reset itself.
enum class Change { None, StartRound, FinishRound };

class Session {
  double enteredScreen = 0.0;
  double lastInputAt = -999.0;

public:
  Screen screen = Screen::Attract;
  float roundLeft = kRoundSeconds;
  float resultsLeft = kResultsSeconds;
  Leaderboard board;

  // Used by the HUD to beep once per second while counting down.
  int lastTickSecond = -1;

  // Filled in by the game when a round ends, read by the results screen.
  EndReason reason = EndReason::TimeUp;
  int finalScore = 0;
  int percentCleared = 0;
  Reward reward;
  bool newBest = false;

  void Begin() {
    enteredScreen = GetTime();
    lastInputAt = GetTime();
    screen = Screen::Attract;
    board.Load(kScoreFile);
  }

  // Any key or click counts as "I want to play".
  static bool AnyInput() {
    return GetKeyPressed() != 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
  }

  bool InputLocked() const {
    return GetTime() - lastInputAt < kInputLockSeconds;
  }

  // Call once per frame before drawing. Returns what the game must react to.
  // The score and board progress are supplied by the game, which is the only
  // thing that knows them.
  Change Tick(int currentScore, int percentCleared) {
    if (screen == Screen::Attract) {
      if (AnyInput() && !InputLocked()) {
        lastInputAt = GetTime();
        screen = Screen::Playing;
        roundLeft = kRoundSeconds;
        enteredScreen = GetTime();
        return Change::StartRound;
      }
      return Change::None;
    }

    if (screen == Screen::Playing) {
      // Direction keys double as activity, so the lockout never swallows the
      // first move of a round.
      if (AnyInput())
        lastInputAt = GetTime();
      roundLeft -= GetFrameTime();
      if (roundLeft <= 0.0f) {
        roundLeft = 0.0f;
        Finish(EndReason::TimeUp, currentScore, percentCleared);
        return Change::FinishRound;
      }
      return Change::None;
    }

    // Results
    resultsLeft -= GetFrameTime();
    bool wantsOut = resultsLeft <= 0.0f ||
                    (AnyInput() && !InputLocked() &&
                     GetTime() - enteredScreen > 1.0f);
    if (wantsOut) {
      lastInputAt = GetTime();
      screen = Screen::Attract;
      enteredScreen = GetTime();
    }
    return Change::None;
  }

  // The game calls this when the board is cleared or lives run out.
  void Finish(EndReason why, int score, int clearedPercent) {
    reason = why;
    finalScore = score;
    percentCleared = clearedPercent;
    bool won = (why == EndReason::ClearedBoard);
    reward = EvaluateReward(clearedPercent, won);
    newBest = board.Submit(score);
    board.Save(kScoreFile);
    resultsLeft = kResultsSeconds;
    screen = Screen::Results;
    enteredScreen = GetTime();
  }
};

// ---------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------

// The one session the game loop and the screens share.
inline Session session;

// raylib's Color leaves alpha at 0 when you brace-initialise only three
// channels, which silently draws nothing. Always go through this instead.
inline Color RGB(unsigned char r, unsigned char g, unsigned char b,
                 unsigned char a = 255) {
  return Color{r, g, b, a};
}

inline void DrawCenteredText(const char *text, int centerX, int y, int size,
                             Color color) {
  int w = MeasureText(text, size);
  DrawText(text, centerX - w / 2, y, size, color);
}

// A soft drop shadow keeps light text readable over the maze.
inline void DrawCenteredTextShadow(const char *text, int centerX, int y,
                                   int size, Color color) {
  int w = MeasureText(text, size);
  int x = centerX - w / 2;
  DrawText(text, x + 3, y + 3, size, Color{0, 0, 0, 160});
  DrawText(text, x, y, size, color);
}

inline void Panel(int x, int y, int w, int h, float alpha = 0.92f) {
  DrawRectangleRec(Rectangle{(float)x, (float)y, (float)w, (float)h},
                   Color{8, 8, 20, (unsigned char)(255 * alpha)});
  DrawRectangleLinesEx(Rectangle{(float)x, (float)y, (float)w, (float)h}, 3,
                      Color{255, 255, 255, 40});
}

// A big tappable-looking block that follows the mouse, for attract screens.
inline bool Hovering(int x, int y, int w, int h) {
  return CheckCollisionPointRec(GetMousePosition(),
                                Rectangle{(float)x, (float)y, (float)w,
                                          (float)h});
}

}  // namespace event