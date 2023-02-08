#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define N_ROCKS 2022
#define ROW 8UL

typedef uint32_t piece_t;
typedef uint8_t row_t;

const piece_t pieces[] = {
    (piece_t)0b00011110,

    ((piece_t)0b00001000) << 16 | ((piece_t)0b00011100) << 8 |
        ((piece_t)0b00001000),

    ((piece_t)0b00000100) << 16 | ((piece_t)0b00000100) << 8 |
        ((piece_t)0b00011100),

    ((piece_t)0b00010000) << 24 | ((piece_t)0b00010000) << 16 |
        ((piece_t)0b00010000) << 8 | ((piece_t)0b00010000),

    ((piece_t)0b00011000) << 8 | ((piece_t)0b00011000),
};

const piece_t right_edge = ((piece_t)0b00000001) << 24 |
                           ((piece_t)0b00000001) << 16 |
                           ((piece_t)0b00000001) << 8 | ((piece_t)0b00000001);

const piece_t left_edge = ((piece_t)0b01000000) << 24 |
                          ((piece_t)0b01000000) << 16 |
                          ((piece_t)0b01000000) << 8 | ((piece_t)0b01000000);

const piece_t edges = right_edge | left_edge;

void print_piece(piece_t piece) {
  for (int i = 0; i < 4; i++) {
    uint8_t mask = (uint8_t)1 << ((sizeof(uint8_t) << 3) - 1);
    while (mask) {
      printf("%c", piece & mask ? '#' : '.');
      mask >>= 1;
    }
    printf("\n");
  }
}

void piece_move_right(uint32_t *piece, uint32_t mask) {
  if ((((*piece) & right_edge) == 0) && ((((*piece) >> 1) & mask) == 0)) {
    // printf("yes right\n");
    (*piece) >>= 1;
  }
}

void piece_move_left(uint32_t *piece, uint32_t mask) {
  if ((((*piece) & left_edge) == 0) && ((((*piece) << 1) & mask) == 0)) {
    // printf("yes left\n");
    (*piece) <<= 1;
  }
}

typedef struct {
  uint8_t *rows;
  size_t size;
  size_t capacity;
} cave_t;

cave_t *cave_new() {
  cave_t *cave = calloc(1, sizeof(cave_t));
  cave->size = 0;
  cave->capacity = 2;
  cave->rows = calloc(cave->capacity, sizeof(uint8_t));

  return cave;
}

void cave_push(cave_t *cave, uint8_t row) {

  if (cave->size == cave->capacity) {
    cave->capacity *= 2;
    cave->rows = reallocarray(cave->rows, cave->capacity, sizeof(uint8_t));
    memset(cave->rows + cave->size, 0, cave->size);
  }

  cave->rows[cave->size] = row;
  cave->size++;
}

uint32_t cave_mask(cave_t *cave, size_t height) {
  if (height >= cave->size) {
    return 0;
  }
  uint32_t result = 0;
  for (int i = 0; i < 4; i++) {
    result <<= ROW;
    if (height + 3 - i >= cave->size) {
      continue;
    }
    result |= cave->rows[height + 3 - i];
  }

  return result;
}

void cave_print(cave_t *cave) {
  for (int i = cave->size - 1; i >= 0; i--) {
    uint8_t mask = (uint8_t)1 << ((sizeof(uint8_t) << 3) - 1);
    while (mask) {
      printf("%c", cave->rows[i] & mask ? '#' : '.');
      mask >>= 1;
    }
    printf("\n");
  }
}

typedef struct {
  char *jets;
  size_t size;
  size_t current;
} jets_t;

jets_t *jets_new() {
  jets_t *jets = calloc(1, sizeof(jets_t));
  jets->jets = NULL;
  jets->size = 0;
  jets->current = 0;

  return jets;
}

void jets_free(jets_t *jets) {
  free(jets->jets);
  free(jets);
}

char jets_next(jets_t *jets) {
  char result = jets->jets[jets->current];
  jets->current = (jets->current + 1) % jets->size;
  return result;
}

void piece_move(uint32_t *piece, uint32_t mask, jets_t *jets) {
  switch (jets_next(jets)) {
  case '<':
    // printf("left\n");
    piece_move_left(piece, mask);
    break;
  case '>':
    // printf("right\n");
    piece_move_right(piece, mask);
    break;
  default:
    exit(EXIT_FAILURE);
  }
}

void cave_drop(cave_t *cave, jets_t *jets, uint32_t piece) {
  size_t height = cave->size + 3;

  while (true) {
    // print_piece(piece);
    // printf("\n");
    // cave_print(cave);
    // printf("\n");
    // printf("\n");
    uint32_t current_mask = cave_mask(cave, height);
    piece_move(&piece, current_mask, jets);

    if (height > cave->size) {
      height--;
    } else if ((height == 0) || ((piece & cave_mask(cave, height - 1)) != 0)) {
      for (int i = 0; i < 4; i++) {
        uint8_t byte = (uint8_t)(piece >> (ROW * i));
        if (byte == 0) {
          break;
        }
        if (height < cave->size) {
          cave->rows[height] |= byte;
        } else {
          cave_push(cave, byte);
        }
        height++;
      }
      return;
    } else {
      height--;
    }
  }
}

int main(int argc, char *argv[]) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;

  char *filename = argv[1];

  fp = fopen(filename, "r");
  if (fp == NULL) {
    exit(EXIT_FAILURE);
  }

  jets_t *jets = jets_new();

  jets->size = getline(&(jets->jets), &len, fp);
  assert(jets->size != -1);
  assert(getline(&line, &len, fp) == -1);

  jets->size--;
  jets->jets[jets->size] = 0;

  // printf("%s\n", jets->jets);

  fclose(fp);

  if (line) {
    free(line);
  }

  cave_t *cave = cave_new();
  for (int i = 0; i < N_ROCKS; i++) {
    cave_drop(cave, jets, pieces[i % ARRAY_SIZE(pieces)]);
  }
  // cave_print(cave);
  printf("%ld\n", cave->size);

  jets_free(jets);

  return 0;
}
