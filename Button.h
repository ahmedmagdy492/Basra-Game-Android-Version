#pragma once

#include <raylib.h>
#include "constants.h"

void DrawOkDialogButton(Font* font, int draw_cancel) {
  if(draw_cancel) {
    Vector2 size = MeasureTextEx(*font, "OK", 60, 0);
    int btn_x = (WIDTH-size.x-200)/2, btn_y = HEIGHT-400;
    DrawRectangle(btn_x, btn_y, size.x+60, 80+20, WHITE);
    DrawTextEx(*font, "OK", (Vector2){btn_x+20, btn_y+20}, 60, 0, BLACK);
    
    Vector2 size2 = MeasureTextEx(*font, "Cancel", 60, 0);
    int btn_x2 = ((WIDTH-size.x-200)/2)+size.x+100, btn_y2 = HEIGHT-400;
    DrawRectangle(btn_x2, btn_y2, size2.x+60, 80+20, WHITE);
    DrawTextEx(*font, "Cancel", (Vector2){btn_x2+20, btn_y2+20}, 60, 0, BLACK);
  }
  else {
    Vector2 size = MeasureTextEx(*font, "OK", 60, 0);
    int btn_x = (WIDTH-size.x)/2, btn_y = HEIGHT-400;
    DrawRectangle(btn_x, btn_y, size.x+60, 80+20, WHITE);
    DrawTextEx(*font, "OK", (Vector2){btn_x+20, btn_y+20}, 60, 0, BLACK);
  }
}

int IsOkDialogButtonClicked(Font* font, int draw_cancel) {
  if(draw_cancel) {
    Vector2 size = MeasureTextEx(*font, "OK", 60, 0);
    Vector2 mouse_pos = GetMousePosition();
    int btn_x = (WIDTH-size.x-200)/2, btn_y = HEIGHT-400;
    int width = size.x+40, height = 60+20;
    if((mouse_pos.x >= btn_x && mouse_pos.x < (btn_x + width)) && (mouse_pos.y >= btn_y && mouse_pos.y < (btn_y + height))) {
      return 1;
    }
  }
  else {
    Vector2 size = MeasureTextEx(*font, "OK", 60, 0);
    Vector2 mouse_pos = GetMousePosition();
    int btn_x = (WIDTH-size.x)/2, btn_y = HEIGHT-400;
    int width = size.x+40, height = 60+20;
    if((mouse_pos.x >= btn_x && mouse_pos.x < (btn_x + width)) && (mouse_pos.y >= btn_y && mouse_pos.y < (btn_y + height))) {
      return 1;
    }
  }
  
  return 0;
}

int IsCancelDialogButtonClicked(Font *font) {
  Vector2 size1 = MeasureTextEx(*font, "OK", 60, 0);
  Vector2 size = MeasureTextEx(*font, "Cancel", 60, 0);
  Vector2 mouse_pos = GetMousePosition();
  int btn_x = ((WIDTH-size1.x-200)/2)+size1.x+100, btn_y = HEIGHT-400;
  int width = size.x+40, height = 60+20;
  if((mouse_pos.x >= btn_x && mouse_pos.x < (btn_x + width)) && (mouse_pos.y >= btn_y && mouse_pos.y < (btn_y + height))) {
    return 1;
  }
  
  return 0;
}

void DrawGenericButton(char* text, Vector2 pos, Font* font) {
  Vector2 size = MeasureTextEx(*font, text, 60, 0);
  DrawRectangle(pos.x, pos.y, size.x+60, 100+20, WHITE);
  DrawTextEx(*font, text, (Vector2){pos.x+30, pos.y+30}, 60, 0, BLACK);
}

int IsButtonClicked(Vector2 *btn_pos, Vector2 *dim) {
  Vector2 mouse_pos = GetMousePosition();
  if((mouse_pos.x >= btn_pos->x && mouse_pos.x < (btn_pos->x + dim->x)) && (mouse_pos.y >= btn_pos->y && mouse_pos.y < (btn_pos->y + dim->y))) {
    return 1;
  }
  
  return 0;
}
