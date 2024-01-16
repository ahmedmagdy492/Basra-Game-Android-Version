#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <raylib.h>
#include <limits.h>

#include "Utils.h"
#include "constants.h"
#include "Card.h"
#include "Stack.h"
#include "Pile.h"
#include "LinkedList.h"
#include "Player.h"
#include "Ground.h"
#include "GameRules.h"
#include "Button.h"

#include "Recomend_System.h"

// Game State
struct
{
  unsigned int show_dialog : 1;
  unsigned int is_playing : 1;
  unsigned int is_dimmed : 1;
  unsigned int someone_won : 1;
  unsigned int debug_mode : 1;
} game_flags;

uint8_t current_player = 0;
uint32_t frames_counter = 0;
Card *cur_selected_card = NULL;
GameRules cur_play_rule = GameRuleNone;
ScreenMode cur_mode = MenuScreen;
ComputerPlayMode cur_computer_playmode = EasyMode;

Player player1, computer;
Stack pile;
LinkedList ground = {0};

// drawing section

// drawing dialogs
int cards_to_show_no = 1;
Card *cards_to_show[3];
char text_to_show[100];
Font font;
void DrawFullScreenDialog()
{
  DrawRectangle(0, 0, WIDTH, HEIGHT, CLITERAL(Color){0, 0, 0, 200});
  int i;
  for (i = 0; i < cards_to_show_no; ++i)
  {
    if (cards_to_show[i] != NULL)
    {
      int x = cards_to_show[i]->texture.width * i + 40;
      DrawTexture(cards_to_show[i]->texture, x, 40, WHITE);
    }
  }

  Vector2 size = MeasureTextEx(font, text_to_show, 60, 0);
  DrawTextEx(font, text_to_show, (Vector2){(WIDTH - size.x) / 2, (HEIGHT - 300) / 2}, 60, 0, WHITE);

  if (current_player == 0 && game_flags.someone_won == 0)
  {
    DrawOkDialogButton(&font, 1);
  }
  else
  {
    DrawOkDialogButton(&font, 0);
  }
}
void CloseDialog()
{
  game_flags.show_dialog = 0;
  game_flags.is_dimmed = 0;
}

// drawing turn and score
void DrawGameText()
{
  if (current_player == 0)
  {
    // player1
    DrawTextEx(font, "Turn: Player1", (Vector2){5, 5}, 30, 0, WHITE);
  }
  else
  {
    // computer
    DrawTextEx(font, "Turn: Computer", (Vector2){5, 5}, 30, 0, WHITE);
  }

  // drawing pocket count
  DrawTextEx(font, TextFormat("your pocket: %i", StackCountWithBasra(&player1.pocket)), (Vector2){10, HEIGHT - 40}, 40, 0, WHITE);
  DrawTextEx(font, TextFormat("Com pocket: %i, Cards Left: %i, Difficulty: %s", StackCountWithBasra(&computer.pocket), CountLL(&computer.cur_set), (cur_computer_playmode == HardMode) ? "Hard" : "Easy"), (Vector2){(WIDTH / 2 - 150), 10}, 30, 0, WHITE);
  DrawGenericButton("Back", (Vector2){WIDTH - 300, HEIGHT - 140}, &font);
  DrawTextEx(font, TextFormat("Pile Cards: %i", StackCount(&pile)), (Vector2){10, GROUND_X_START + (HEIGHT - 200) + 80}, 40, 0, WHITE);
}

// handling ground staff
void DrawGroundBG()
{
  DrawRectangleLines(0, GROUND_X_START, WIDTH, HEIGHT - 200 - GROUND_X_START, WHITE);
  DrawTextEx(font, "Ground", (Vector2){WIDTH / 2 - 40, HEIGHT / 2 - 30}, 40, 0, WHITE);
}

void DrawGroundCards()
{
  int i = 0;
  Node *ptr = ground.head;
  while (ptr != NULL)
  {
    Card *card = ptr->card;
    if (card != NULL)
    {
      int x = i * card->width + GROUND_X_START, y = GROUND_X_START;
      if ((x + card->width) >= WIDTH)
      {
        x = GROUND_X_START;
        y += card->height + 5;
      }
      card->x = x;
      card->y = y;
      DrawCard(card);
      ++i;
    }
    ptr = ptr->next;
  }
}

void Init()
{
  pile.top = 0;
  CreateThePile(&pile);
  ShuffleThePile(&pile);

  // creating the players
  player1.score = computer.score = 0;
  player1.basra_count = computer.basra_count = 0;
  memset(&player1.cur_set, 0, sizeof(sizeof(LinkedList)));
  memset(&computer.cur_set, 0, sizeof(sizeof(LinkedList)));

  // distributing 4 cards to each player
  DistributeCards(&pile, &player1, &computer);

  // disteribute 4 cards to the ground
  int i;
  for (i = 0; i < 4; ++i)
  {
    Card *card = Pop(&pile);
    AppendToLL(&ground, card);
  }

  cur_play_rule = GameRuleNone;
  game_flags.show_dialog = 0;
  game_flags.is_playing = 0;
  game_flags.is_dimmed = 0;
  game_flags.someone_won = 0;
  frames_counter = 0;
  game_flags.is_playing = 1;
  current_player = 0;
}

Card *GetClickedCard(Vector2 *mouse_pos, int *from_ground)
{
  Card *card = NULL;

  // checking cards on the ground
  Node *ptr = ground.head;

  while (ptr != NULL)
  {
    if ((mouse_pos->x >= ptr->card->x && mouse_pos->x < (ptr->card->x + ptr->card->width)) && (mouse_pos->y >= ptr->card->y && mouse_pos->y < (ptr->card->y + ptr->card->height)))
    {
      card = ptr->card;
      *from_ground = 1;
      return card;
    }
    ptr = ptr->next;
  }

  // checking player1's cards
  ptr = player1.cur_set.head;

  while (ptr != NULL)
  {
    if ((mouse_pos->x >= ptr->card->x && mouse_pos->x < (ptr->card->x + ptr->card->width)) && (mouse_pos->y >= ptr->card->y && mouse_pos->y < (ptr->card->y + ptr->card->height)))
    {
      card = ptr->card;
      *from_ground = 0;
      return card;
    }
    ptr = ptr->next;
  }

  return card;
}

void PerformAction(GameRules game_rule)
{
  Player *player = current_player == 0 ? &player1 : &computer;
  cur_selected_card->selected = 0;

  switch (game_rule)
  {
  case Basra:
  case TwoCardsMatch:
  {
    if (game_rule == Basra)
    {
      cur_selected_card->is_basra = 1;
    }
    RemoveFromLL(&player->cur_set, cur_selected_card);
    Card *matching_card = FindAMatchFromGround(&ground, cur_selected_card);

    RemoveFromLL(&ground, matching_card);
    Push(&player->pocket, cur_selected_card);
    Push(&player->pocket, matching_card);
  }
  break;
  case TakeAllGroundCards:
  {
    TakeAllCards(&ground, player, cur_selected_card);
  }
  break;
  case NoMatch:
  {
    RemoveFromLL(&player->cur_set, cur_selected_card);
    AppendToLL(&ground, cur_selected_card);
  }
  break;
  case SumMatch:
  {
    RemoveFromLL(&player->cur_set, cur_selected_card);
    Push(&player->pocket, cur_selected_card);
    int i = 0;
    for (; i < cards_to_show_no; ++i)
    {
      RemoveFromLL(&ground, cards_to_show[i]);
      Push(&player->pocket, cards_to_show[i]);
    }
  }
  break;
  }
}

int GetWhoWon()
{
  if (CountLL(&ground) > 0)
  {
    // NOTE: switching players order because the turn has already been switched when reached this point
    if (current_player == 0)
    {
      // pass null for card to remove from player because it's the end of the game
      TakeAllCards(&ground, &computer, NULL);
    }
    else
    {
      TakeAllCards(&ground, &player1, NULL);
    }
  }

  int player1_count = StackCountWithBasra(&player1.pocket);
  int computer_count = StackCountWithBasra(&computer.pocket);

  if (player1_count > computer_count)
  {
    return 0;
  }
  else if (computer_count > player1_count)
  {
    return 1;
  }

  // draw
  return 2;
}

#define SWITCH_PLAYER() (current_player = current_player == 0 ? 1 : 0)

int main()
{
  InitWindow(0, 0, GAME_NAME);
  // SetConfigFlags(FLAG_FULLSCREEN_MODE);
  WIDTH = GetScreenWidth();
  HEIGHT = GetScreenHeight();

  InitAudioDevice();

  SetTargetFPS(60);

  font = LoadFontEx("resources/fonts/OleoScript-nj4P.ttf", 40, NULL, 0);

  // loading screen
  BeginDrawing();
  Vector2 loading_txt_size = MeasureTextEx(font, "Ahmed Magdy", 40, 0);
  DrawTextEx(font, "Ahmed Magdy", (Vector2){(WIDTH - loading_txt_size.x) / 2, (HEIGHT - loading_txt_size.y) / 2}, 40, 0, WHITE);
  EndDrawing();

  Sound sound = LoadSound("resources/sound/gameplay.ogg");
  Image logo = LoadImage("resources/logo.png");
  Texture logo_texture = LoadTextureFromImage(logo);

  Init();

  SetExitKey(0);

  PlaySound(sound);

  while (!WindowShouldClose())
  {
    if (cur_mode == PlayScreen)
    {
      BeginDrawing();
      ClearBackground(DARKGREEN);

      if (!IsSoundPlaying(sound))
      {
        PlaySound(sound);
      }

      if (game_flags.is_playing)
      {
        DrawGroundBG();
        DrawGroundCards();
        DrawGameText();
        DrawPlayerCards(&player1);

        Vector2 back_btn_pos = (Vector2){WIDTH - 300, HEIGHT - 140};
        Vector2 back_txt_size = MeasureTextEx(font, "Back", 60, 0);
        Vector2 back_btn_dim = (Vector2){back_btn_pos.x + back_txt_size.x + 60, back_btn_pos.y + back_txt_size.y + 120};

        if (game_flags.is_dimmed == 0)
        {
          if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
          {
            if (current_player == 0)
            {
              if (IsButtonClicked(&back_btn_pos, &back_btn_dim))
              {
                cur_mode = MenuScreen;
              }

              int from_ground = 0;
              Vector2 mouse_pos = GetMousePosition();
              Card *card = GetClickedCard(&mouse_pos, &from_ground);

              if (!from_ground && card != NULL)
              {
                if (cur_selected_card != card)
                {
                  card->selected = 1;
                  if (cur_selected_card != NULL)
                  {
                    cur_selected_card->selected = 0;
                  }
                  cur_selected_card = card;
                }
                else
                {
                  if (IsJackOr7Diamond(cur_selected_card))
                  {
                    cur_play_rule = TakeAllGroundCards;
                    cards_to_show_no = 1;
                    cards_to_show[0] = cur_selected_card;
                  }
                  else
                  {
                    cards_to_show_no = FindMatchingCardsFromGround(&ground, card, cards_to_show);

                    if (cards_to_show_no == 0)
                    {
                      // did not find any sum matching
                      Card *ground_card = FindAMatchFromGround(&ground, card);
                      cur_play_rule = DetermineGameRule(&ground, &player1, ground_card, card);
                      cards_to_show[0] = cur_selected_card;
                      cards_to_show_no = 1;
                    }
                    else
                    {
                      // found some sum matching
                      cur_play_rule = SumMatch;
                    }
                  }

                  GetGameRuleName(current_player, cur_play_rule, text_to_show);
                  game_flags.show_dialog = 1;
                  game_flags.is_dimmed = 1;
                }
              }
            }
          }
        }
        else {
          if(current_player == 0 && game_flags.someone_won == 0) {
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
              if (IsCancelDialogButtonClicked(&font))
              {
                if (game_flags.show_dialog == 1)
                {
                  CloseDialog();
                }
              }
            }
          }
        }

        if (current_player == 1) // computer's turn
        {
          Card *card = GetBestCard(&computer, cur_computer_playmode, &ground);
          if (CountLL(&computer.cur_set) > 0)
          {
            cur_selected_card = card;
            if (IsJackOr7Diamond(cur_selected_card))
            {
              cur_play_rule = TakeAllGroundCards;
              cards_to_show_no = 1;
              cards_to_show[0] = cur_selected_card;
            }
            else
            {
              cards_to_show_no = FindMatchingCardsFromGround(&ground, cur_selected_card, cards_to_show);

              if (cards_to_show_no == 0)
              {
                Card *matching_card = FindAMatchFromGround(&ground, card);
                cur_play_rule = DetermineGameRule(&ground, &computer, matching_card, card);
                cards_to_show[0] = card;
                cards_to_show_no = 1;
              }
              else
              {
                cur_play_rule = SumMatch;
                cards_to_show[cards_to_show_no++] = cur_selected_card;
              }
            }

            GetGameRuleName(current_player, cur_play_rule, text_to_show);
            game_flags.show_dialog = 1;
            game_flags.is_dimmed = 1;
          }
        }
      }

      if (game_flags.show_dialog == 1)
      {
        DrawFullScreenDialog();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
          int is_ok_dialogbtn_clicked = 0;

          if(game_flags.someone_won == 1) {
            is_ok_dialogbtn_clicked = IsOkDialogButtonClicked(&font, 0);
          }
          else {
            is_ok_dialogbtn_clicked = IsOkDialogButtonClicked(&font, current_player == 0 ? 1 : 0);
          }

          if (is_ok_dialogbtn_clicked)
          {
            if (game_flags.someone_won == 1)
            {
              cur_mode = MenuScreen;
              Init();
            }
            else
            {
              CloseDialog();
              PerformAction(cur_play_rule);
              SWITCH_PLAYER();
            }
          }
        }
      }
      else
      {
        // check if the both players card sets are empty
        if (StackCount(&pile) > 0)
        {
          if (CountLL(&player1.cur_set) == 0 && CountLL(&computer.cur_set) == 0)
          {
            DistributeCards(&pile, &player1, &computer);
          }
        }
        else if (StackCount(&pile) == 0 && CountLL(&player1.cur_set) == 0 && CountLL(&computer.cur_set) == 0)
        {
          cards_to_show_no = 0;
          memset(text_to_show, 0, 100);
          int who_won = GetWhoWon();
          if (who_won == 0)
          {
            strncpy(text_to_show, TextFormat("You Won, Com: %i, You: %i", StackCountWithBasra(&computer.pocket), StackCountWithBasra(&player1.pocket)), strlen(TextFormat("You Won, Com: %i, You: %i", StackCountWithBasra(&computer.pocket), StackCountWithBasra(&player1.pocket))));
          }
          else if (who_won == 1)
          {
            strncpy(text_to_show, TextFormat("Computer Won, Com: %i, You: %i", StackCountWithBasra(&computer.pocket), StackCountWithBasra(&player1.pocket)), strlen(TextFormat("Computer Won, Com: %i, You: %i", StackCountWithBasra(&computer.pocket), StackCountWithBasra(&player1.pocket))));
          }
          else
          {
            strncpy(text_to_show, "Draw", strlen("Draw"));
          }
          game_flags.show_dialog = 1;
          game_flags.is_dimmed = 1;
          game_flags.someone_won = 1;
        }
      }

      EndDrawing();

      if (frames_counter < INT_MAX)
      {
        ++frames_counter;
      }
      else
      {
        frames_counter = 0;
      }
    }
    else if (cur_mode == MenuScreen)
    {
      BeginDrawing();
      ClearBackground(BLACK);

      DrawTexture(logo_texture, (WIDTH - logo_texture.width) / 2, 80, WHITE);

      Vector2 size1 = MeasureTextEx(font, "Start Game", 60, 0);
      int y1 = (HEIGHT - size1.y) / 2 + 20;
      Vector2 pos1 = (Vector2){(WIDTH - (size1.x + 60)) / 2, y1};
      DrawGenericButton("Start Game", pos1, &font);

      Vector2 size3 = MeasureTextEx(font, "How To Play", 60, 0);
      int y3 = y1 + size3.y + 80;
      Vector2 pos3 = (Vector2){(WIDTH - (size3.x + 60)) / 2, y3};
      DrawGenericButton("How to Play", pos3, &font);

      Vector2 size4 = MeasureTextEx(font, "Exit", 60, 0);
      Vector2 pos4 = (Vector2){(WIDTH - (size4.x + 60)) / 2, y3 + size4.y + 80};
      DrawGenericButton("Exit", pos4, &font);

      Vector2 dim1 = (Vector2){size1.x + 60, 120};
      Vector2 dim3 = (Vector2){size3.x + 60, 120};
      Vector2 dim4 = (Vector2){size4.x + 60, 120};

      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      {
        if (IsButtonClicked(&pos1, &dim1))
        {
          // change mode to play and start the game
          cur_computer_playmode = HardMode;
          cur_mode = PlayScreen;
        }
        else if (IsButtonClicked(&pos3, &dim3))
        {
          // show how to play screen
          cur_mode = HowToPlayScreen;
        }
        else if (IsButtonClicked(&pos4, &dim4))
        {
          // just exiting the game
          break;
        }
      }

      EndDrawing();
    }
    else if (cur_mode == HowToPlayScreen)
    {
      BeginDrawing();
      ClearBackground(BLACK);

      DrawTextEx(font, "How to Play", (Vector2){40, 40}, 60, 0, WHITE);

      DrawTextEx(font, "- Each player takes 4 cards and the main goal is to win as many cards as he can. ", (Vector2){40, 130}, 60, 0, WHITE);
      DrawTextEx(font, "- You can win all of the cards on the ground if we have 7 diamond or the Jack", (Vector2){40, 180}, 60, 0, WHITE);
      DrawTextEx(font, "- You can also match a card you have with a card on the ground if both have ", (Vector2){40, 230}, 60, 0, WHITE);
      DrawTextEx(font, "the same number or the same type (i.e. queen and queen)", (Vector2){40, 280}, 60, 0, WHITE);
      DrawTextEx(font, "- You can win 10 points at once if there are only one card on the ground ", (Vector2){40, 330}, 60, 0, WHITE);
      DrawTextEx(font, "and you have a match for it this is called a Basra.", (Vector2){40, 400}, 60, 0, WHITE);

      Vector2 size = MeasureTextEx(font, "Back", 40, 0);
      Vector2 pos = (Vector2){(GetScreenWidth() - size.x) / 2, 480};
      DrawGenericButton("Back", pos, &font);
      Vector2 dim = (Vector2){size.x + 60, 120};

      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (IsButtonClicked(&pos, &dim))
        {
          cur_mode = MenuScreen;
        }
      }

      EndDrawing();
    }
    else if (cur_mode == PauseScreen)
    {
      BeginDrawing();
      ClearBackground(BLACK);

      Vector2 size = MeasureTextEx(font, "Pause, Press Q to back to main menu", 50, 0);
      DrawRectangle(0, 0, WIDTH, HEIGHT, CLITERAL(Color){0, 0, 0, 128});
      DrawTextEx(font, "Pause, Press Q to back to main menu", (Vector2){(WIDTH - size.x) / 2, (HEIGHT - size.y) / 2}, 50, 0, WHITE);

      if (IsKeyPressed(KEY_ESCAPE))
      {
        cur_mode = PlayScreen;
      }

      if (IsKeyPressed(KEY_Q))
      {
        cur_mode = MenuScreen;
      }

      EndDrawing();
    }
  }

  CleanupLL(&ground);
  CleanThePile(&pile);

  UnloadImage(logo);
  UnloadTexture(logo_texture);

  UnloadSound(sound);
  UnloadFont(font);

  CloseAudioDevice();

  CloseWindow();
  return 0;
}
