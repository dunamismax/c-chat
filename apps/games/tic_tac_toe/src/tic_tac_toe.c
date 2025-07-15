#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Tic-Tac-Toe Game
 * Demonstrates: 2D arrays, game logic, input validation, AI
 */

#define BOARD_SIZE 3
#define EMPTY ' '
#define PLAYER_X 'X'
#define PLAYER_O 'O'

typedef struct {
  char board[BOARD_SIZE][BOARD_SIZE];
  char current_player;
  int moves_made;
  int game_over;
  char winner;
} Game;

/**
 * Initialize the game
 */
void init_game(Game *game) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      game->board[i][j] = EMPTY;
    }
  }
  game->current_player = PLAYER_X;
  game->moves_made = 0;
  game->game_over = 0;
  game->winner = EMPTY;
}

/**
 * Display the game board
 */
void display_board(const Game *game) {
  printf("\n  Current Board:\n");
  printf("     1   2   3\n");
  printf("   +---+---+---+\n");

  for (int i = 0; i < BOARD_SIZE; i++) {
    printf(" %c ", 'A' + i);
    for (int j = 0; j < BOARD_SIZE; j++) {
      printf("| %c ", game->board[i][j]);
    }
    printf("|\n");
    printf("   +---+---+---+\n");
  }
  printf("\n");
}

/**
 * Check if a position is valid and empty
 */
int is_valid_move(const Game *game, int row, int col) {
  return (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE &&
          game->board[row][col] == EMPTY);
}

/**
 * Make a move on the board
 */
int make_move(Game *game, int row, int col, char player) {
  if (!is_valid_move(game, row, col)) {
    return 0; // Invalid move
  }

  game->board[row][col] = player;
  game->moves_made++;
  return 1; // Valid move
}

/**
 * Check for three in a row
 */
char check_line(char a, char b, char c) {
  if (a == b && b == c && a != EMPTY) {
    return a;
  }
  return EMPTY;
}

/**
 * Check if the game is won
 */
char check_winner(const Game *game) {
  // Check rows
  for (int i = 0; i < BOARD_SIZE; i++) {
    char winner =
        check_line(game->board[i][0], game->board[i][1], game->board[i][2]);
    if (winner != EMPTY)
      return winner;
  }

  // Check columns
  for (int j = 0; j < BOARD_SIZE; j++) {
    char winner =
        check_line(game->board[0][j], game->board[1][j], game->board[2][j]);
    if (winner != EMPTY)
      return winner;
  }

  // Check diagonals
  char winner =
      check_line(game->board[0][0], game->board[1][1], game->board[2][2]);
  if (winner != EMPTY)
    return winner;

  winner = check_line(game->board[0][2], game->board[1][1], game->board[2][0]);
  if (winner != EMPTY)
    return winner;

  return EMPTY;
}

/**
 * Check if the board is full
 */
int is_board_full(const Game *game) {
  return game->moves_made >= BOARD_SIZE * BOARD_SIZE;
}

/**
 * Update game state
 */
void update_game_state(Game *game) {
  game->winner = check_winner(game);

  if (game->winner != EMPTY) {
    game->game_over = 1;
  } else if (is_board_full(game)) {
    game->game_over = 1;
    game->winner = 'T'; // Tie
  }
}

/**
 * Switch to the other player
 */
void switch_player(Game *game) {
  game->current_player =
      (game->current_player == PLAYER_X) ? PLAYER_O : PLAYER_X;
}

/**
 * Parse move from user input (e.g., "A1", "B2", "C3")
 */
int parse_move(const char *input, int *row, int *col) {
  if (strlen(input) != 2) {
    return 0;
  }

  char row_char = toupper(input[0]);
  char col_char = input[1];

  if (row_char < 'A' || row_char > 'C') {
    return 0;
  }

  if (col_char < '1' || col_char > '3') {
    return 0;
  }

  *row = row_char - 'A';
  *col = col_char - '1';

  return 1;
}

/**
 * Simple AI: find winning move, block opponent, or pick center/corner
 */
int ai_move(Game *game, int *row, int *col) {
  // Try to win
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (is_valid_move(game, i, j)) {
        game->board[i][j] = PLAYER_O;
        if (check_winner(game) == PLAYER_O) {
          game->board[i][j] = EMPTY; // Undo
          *row = i;
          *col = j;
          return 1;
        }
        game->board[i][j] = EMPTY; // Undo
      }
    }
  }

  // Block opponent from winning
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (is_valid_move(game, i, j)) {
        game->board[i][j] = PLAYER_X;
        if (check_winner(game) == PLAYER_X) {
          game->board[i][j] = EMPTY; // Undo
          *row = i;
          *col = j;
          return 1;
        }
        game->board[i][j] = EMPTY; // Undo
      }
    }
  }

  // Take center if available
  if (is_valid_move(game, 1, 1)) {
    *row = 1;
    *col = 1;
    return 1;
  }

  // Take a corner
  int corners[][2] = {{0, 0}, {0, 2}, {2, 0}, {2, 2}};
  for (int i = 0; i < 4; i++) {
    if (is_valid_move(game, corners[i][0], corners[i][1])) {
      *row = corners[i][0];
      *col = corners[i][1];
      return 1;
    }
  }

  // Take any available move
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (is_valid_move(game, i, j)) {
        *row = i;
        *col = j;
        return 1;
      }
    }
  }

  return 0; // No moves available
}

/**
 * Display game result
 */
void display_result(const Game *game) {
  printf("\n=== Game Over ===\n");
  if (game->winner == 'T') {
    printf("It's a tie!\n");
  } else {
    printf("Player %c wins!\n", game->winner);
  }
}

/**
 * Play against AI
 */
void play_vs_ai() {
  Game game;
  init_game(&game);

  printf("\n=== Tic-Tac-Toe vs AI ===\n");
  printf("You are X, AI is O\n");
  printf("Enter moves as row-column (e.g., A1, B2, C3)\n");

  while (!game.game_over) {
    display_board(&game);

    if (game.current_player == PLAYER_X) {
      // Human player
      printf("Your move (X): ");
      char input[10];

      if (!fgets(input, sizeof(input), stdin)) {
        break;
      }

      // Remove newline
      input[strcspn(input, "\n")] = 0;

      int row, col;
      if (!parse_move(input, &row, &col)) {
        printf("Invalid input! Use format like A1, B2, C3\n");
        continue;
      }

      if (!make_move(&game, row, col, PLAYER_X)) {
        printf("Invalid move! Position already taken.\n");
        continue;
      }
    } else {
      // AI player
      printf("AI is thinking...\n");
      int row, col;
      if (ai_move(&game, &row, &col)) {
        make_move(&game, row, col, PLAYER_O);
        printf("AI plays: %c%d\n", 'A' + row, col + 1);
      }
    }

    update_game_state(&game);

    if (!game.game_over) {
      switch_player(&game);
    }
  }

  display_board(&game);
  display_result(&game);
}

/**
 * Play two-player mode
 */
void play_two_player() {
  Game game;
  init_game(&game);

  printf("\n=== Two Player Tic-Tac-Toe ===\n");
  printf("Player 1 is X, Player 2 is O\n");
  printf("Enter moves as row-column (e.g., A1, B2, C3)\n");

  while (!game.game_over) {
    display_board(&game);

    printf("Player %c's turn: ", game.current_player);
    char input[10];

    if (!fgets(input, sizeof(input), stdin)) {
      break;
    }

    // Remove newline
    input[strcspn(input, "\n")] = 0;

    int row, col;
    if (!parse_move(input, &row, &col)) {
      printf("Invalid input! Use format like A1, B2, C3\n");
      continue;
    }

    if (!make_move(&game, row, col, game.current_player)) {
      printf("Invalid move! Position already taken.\n");
      continue;
    }

    update_game_state(&game);

    if (!game.game_over) {
      switch_player(&game);
    }
  }

  display_board(&game);
  display_result(&game);
}

/**
 * Main menu
 */
void show_menu() {
  printf("\n=== Tic-Tac-Toe ===\n");
  printf("1. Play vs AI\n");
  printf("2. Two Player Mode\n");
  printf("3. Quit\n");
  printf("Choose an option: ");
}

int main() {
  int choice;

  printf("Welcome to Tic-Tac-Toe!\n");

  while (1) {
    show_menu();

    if (scanf("%d", &choice) != 1) {
      // Clear invalid input
      while (getchar() != '\n')
        ;
      printf("Invalid input! Please enter a number.\n");
      continue;
    }

    // Clear input buffer
    while (getchar() != '\n')
      ;

    switch (choice) {
    case 1:
      play_vs_ai();
      break;
    case 2:
      play_two_player();
      break;
    case 3:
      printf("Thanks for playing!\n");
      return 0;
    default:
      printf("Invalid choice! Please try again.\n");
      break;
    }
  }

  return 0;
}