#pragma once

// ---------------------------------------------------------------------------
// raylib version compatibility shims
//
// The game sources were written against the raylib 4.x API. raylib 5.0
// reshuffled a couple of drawing helpers, which breaks the build on a modern
// raylib. These wrappers keep a single call style working on raylib 4.x *and*
// raylib 5.x so the project builds against whatever raylib the machine (or
// the CI image) happens to provide.
// ---------------------------------------------------------------------------

#include "raylib.h"

#ifndef RAYLIB_VERSION_MAJOR
#if defined(RAYLIB_VERSION) && RAYLIB_VERSION[0] >= '5'
#define RAYLIB_VERSION_MAJOR 5
#else
#define RAYLIB_VERSION_MAJOR 4
#endif
#endif

// raylib 4.x: DrawRectangleRoundedLines(rec, roundness, segments, lineThick, color)
// raylib 5.x: DrawRectangleRoundedLines(rec, roundness, lineThick, color)
//             DrawRectangleRoundedLinesEx(rec, roundness, segments, lineThick, color)
//
// Implemented as an inline function rather than a macro so that braced
// initialiser lists can be passed straight through as an argument.
inline void DrawRectangleRoundedLinesCompat(Rectangle rec, float roundness,
                                             int segments, float lineThick,
                                             Color color) {
#if RAYLIB_VERSION_MAJOR >= 5
  DrawRectangleRoundedLinesEx(rec, roundness, segments, lineThick, color);
#else
  (void)segments;
  DrawRectangleRoundedLines(rec, roundness, lineThick, color);
#endif
}