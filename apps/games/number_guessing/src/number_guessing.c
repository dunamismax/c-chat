#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Include our math utilities
#include "../../../../libs/math_utils/include/math_utils.h"

/**
 * Number Guessing Game
 * Demonstrates: random numbers, loops, conditional logic, user input
 */

typedef struct {
  int secret_number;
  int min_range;
  int max_range;
  int max_attempts;
  int attempts_used;
  int game_won;
  int difficulty;
} Game;

typedef enum { EASY = 1, MEDIUM = 2, HARD = 3, CUSTOM = 4 } Difficulty;

/**
 * Initialize game based on difficulty
 */
void init_game(Game *game, int difficulty) {
  game->difficulty = difficulty;
  game->attempts_used = 0;
  game->game_won = 0;

  switch (difficulty) {
  case EASY:
    game->min_range = 1;
    game->max_range = 50;
    game->max_attempts = 10;
    break;
  case MEDIUM:
    game->min_range = 1;
    game->max_range = 100;
    game->max_attempts = 8;
    break;
  case HARD:
    game->min_range = 1;
    game->max_range = 200;
    game->max_attempts = 6;
    break;
  default: // CUSTOM will be set separately
    break;
  }

  // Generate random number in range
  game->secret_number =
      game->min_range + rand() % (game->max_range - game->min_range + 1);
}

/**
 * Set custom difficulty
 */
void set_custom_difficulty(Game *game, int min_range, int max_range,
                           int max_attempts) {
  game->min_range = min_range;
  game->max_range = max_range;
  game->max_attempts = max_attempts;
  game->difficulty = CUSTOM;
  game->attempts_used = 0;
  game->game_won = 0;

  // Generate random number in range
  game->secret_number =
      game->min_range + rand() % (game->max_range - game->min_range + 1);
}

/**
 * Display game information
 */
void display_game_info(const Game *game) {
  const char *difficulty_names[] = {"", "Easy", "Medium", "Hard", "Custom"};

  printf("\n=== Number Guessing Game ===\n");
  printf("Difficulty: %s\n", difficulty_names[game->difficulty]);
  printf("Range: %d - %d\n", game->min_range, game->max_range);
  printf("Attempts remaining: %d\n", game->max_attempts - game->attempts_used);
  printf("Guess the secret number!\n\n");
}

/**
 * Give hint based on guess
 */
void give_hint(const Game *game, int guess) {
  int difference = math_abs(guess - game->secret_number);

  if (difference == 0) {
    printf("🎉 Correct! You guessed it!\n");
  } else if (difference <= 5) {
    printf("🔥 Very close! ");
  } else if (difference <= 10) {
    printf("🌡️  Close! ");
  } else if (difference <= 20) {
    printf("❄️  Getting warmer... ");
  } else {
    printf("🧊 Cold! ");
  }

  if (difference > 0) {
    if (guess < game->secret_number) {
      printf("Try higher!\n");
    } else {
      printf("Try lower!\n");
    }
  }
}

/**
 * Check if guess is valid
 */
int is_valid_guess(const Game *game, int guess) {
  return (guess >= game->min_range && guess <= game->max_range);
}

/**
 * Process a guess
 */
int process_guess(Game *game, int guess) {
  if (!is_valid_guess(game, guess)) {
    printf("Invalid guess! Please enter a number between %d and %d.\n",
           game->min_range, game->max_range);
    return 0; // Don't count this as an attempt
  }

  game->attempts_used++;

  printf("Attempt %d/%d: %d - ", game->attempts_used, game->max_attempts,
         guess);

  if (guess == game->secret_number) {
    game->game_won = 1;
    give_hint(game, guess);
    return 1;
  } else {
    give_hint(game, guess);
    return 0;
  }
}

/**
 * Display game statistics
 */
void display_statistics(const Game *game) {
  printf("\n=== Game Statistics ===\n");
  printf("Secret number was: %d\n", game->secret_number);
  printf("Attempts used: %d/%d\n", game->attempts_used, game->max_attempts);

  if (game->game_won) {
    printf("Result: Victory! 🎉\n");

    // Calculate score based on attempts and difficulty
    int base_score = 100;
    int attempt_penalty = (game->attempts_used - 1) * 10;
    int difficulty_bonus = game->difficulty * 10;
    int score = math_max(0, base_score - attempt_penalty + difficulty_bonus);

    printf("Score: %d points\n", score);

    if (game->attempts_used == 1) {
      printf("🏆 Perfect! First try!\n");
    } else if (game->attempts_used <= game->max_attempts / 3) {
      printf("🌟 Excellent guessing!\n");
    } else if (game->attempts_used <= game->max_attempts / 2) {
      printf("👍 Good job!\n");
    } else {
      printf("💪 You made it!\n");
    }
  } else {
    printf("Result: Game Over 😔\n");
    printf("Better luck next time!\n");
  }
}

/**
 * Play a single game
 */
void play_game(Game *game) {
  display_game_info(game);

  while (game->attempts_used < game->max_attempts && !game->game_won) {
    printf("Enter your guess: ");

    int guess;
    if (scanf("%d", &guess) != 1) {
      // Clear invalid input
      while (getchar() != '\n')
        ;
      printf("Invalid input! Please enter a number.\n");
      continue;
    }

    if (process_guess(game, guess)) {
      break; // Game won
    }

    if (game->attempts_used < game->max_attempts && !game->game_won) {
      printf("\n");
    }
  }

  display_statistics(game);
}

/**
 * Display difficulty menu
 */
void show_difficulty_menu() {
  printf("\n=== Choose Difficulty ===\n");
  printf("1. Easy   (1-50,   10 attempts)\n");
  printf("2. Medium (1-100,   8 attempts)\n");
  printf("3. Hard   (1-200,   6 attempts)\n");
  printf("4. Custom (choose your own range)\n");
  printf("5. Back to main menu\n");
  printf("Choose difficulty: ");
}

/**
 * Handle custom difficulty setup
 */
void setup_custom_game(Game *game) {
  int min_range, max_range, max_attempts;

  printf("\n=== Custom Game Setup ===\n");

  printf("Enter minimum number: ");
  if (scanf("%d", &min_range) != 1) {
    printf("Invalid input!\n");
    return;
  }

  printf("Enter maximum number: ");
  if (scanf("%d", &max_range) != 1) {
    printf("Invalid input!\n");
    return;
  }

  if (min_range >= max_range) {
    printf("Error: Maximum must be greater than minimum!\n");
    return;
  }

  printf("Enter maximum attempts: ");
  if (scanf("%d", &max_attempts) != 1 || max_attempts < 1) {
    printf("Invalid input! Attempts must be at least 1.\n");
    return;
  }

  set_custom_difficulty(game, min_range, max_range, max_attempts);
  play_game(game);
}

/**
 * Display main menu
 */
void show_main_menu() {
  printf("\n=== Number Guessing Game ===\n");
  printf("1. Play Game\n");
  printf("2. How to Play\n");
  printf("3. Quit\n");
  printf("Choose an option: ");
}

/**
 * Show how to play instructions
 */
void show_instructions() {
  printf("\n=== How to Play ===\n");
  printf("🎯 Objective: Guess the secret number!\n\n");
  printf("📋 Rules:\n");
  printf("• The computer picks a random number in the given range\n");
  printf("• You have a limited number of attempts to guess it\n");
  printf("• After each guess, you'll get a hint:\n");
  printf("  🔥 Very close (within 5)\n");
  printf("  🌡️  Close (within 10)\n");
  printf("  ❄️  Getting warmer (within 20)\n");
  printf("  🧊 Cold (more than 20 away)\n");
  printf("• You'll also be told if your guess is too high or low\n\n");
  printf("🏆 Scoring:\n");
  printf("• Base score: 100 points\n");
  printf("• -10 points for each attempt after the first\n");
  printf("• Difficulty bonus: +10/20/30 for Easy/Medium/Hard\n\n");
  printf("Good luck! 🍀\n");
}

int main() {
  // Seed random number generator
  srand((unsigned int)time(NULL));

  Game game;
  int choice;

  printf("Welcome to the Number Guessing Game!\n");

  while (1) {
    show_main_menu();

    if (scanf("%d", &choice) != 1) {
      // Clear invalid input
      while (getchar() != '\n')
        ;
      printf("Invalid input! Please enter a number.\n");
      continue;
    }

    switch (choice) {
    case 1: {
      // Play game - choose difficulty
      int difficulty_choice;

      while (1) {
        show_difficulty_menu();

        if (scanf("%d", &difficulty_choice) != 1) {
          while (getchar() != '\n')
            ;
          printf("Invalid input! Please enter a number.\n");
          continue;
        }

        if (difficulty_choice >= 1 && difficulty_choice <= 4) {
          if (difficulty_choice == 4) {
            setup_custom_game(&game);
          } else {
            init_game(&game, difficulty_choice);
            play_game(&game);
          }
          break;
        } else if (difficulty_choice == 5) {
          break; // Back to main menu
        } else {
          printf("Invalid choice! Please try again.\n");
        }
      }
      break;
    }
    case 2:
      show_instructions();
      break;
    case 3:
      printf("Thanks for playing! 👋\n");
      return 0;
    default:
      printf("Invalid choice! Please try again.\n");
      break;
    }
  }

  return 0;
}