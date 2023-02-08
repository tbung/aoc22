#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


const uint64_t pieces[5] = {
   (uint64_t)0b00011110,

  ((uint64_t)0b00001000) << 16 |
  ((uint64_t)0b00011100) << 8 |
  ((uint64_t)0b00001000),

  ((uint64_t)0b00000100) << 16 |
  ((uint64_t)0b00000100) << 8 |
  ((uint64_t)0b00011100),

  ((uint64_t)0b00010000) << 24 |
  ((uint64_t)0b00010000) << 16 |
  ((uint64_t)0b00010000) << 8 |
  ((uint64_t)0b00010000),

  ((uint64_t)0b00011000) << 8 |
  ((uint64_t)0b00011000),
};

const uint64_t edges = 
  ((uint64_t)0b01000001) << 56 |
  ((uint64_t)0b01000001) << 48 |
  ((uint64_t)0b01000001) << 40 |
  ((uint64_t)0b01000001) << 32 |
  ((uint64_t)0b01000001) << 24 |
  ((uint64_t)0b01000001) << 16 |
  ((uint64_t)0b01000001) << 8 |
  ((uint64_t)0b01000001);

void print_piece(uint64_t piece) {
  char s[64 + 8 + 1] = {0};

  for (int i = 0; i < 8; i++) {
    int j = 0;
    uint8_t mask = (uint8_t)1 << ((sizeof(uint8_t) << 3) - 1);
    while(mask) {
      s[i * 9 + j] = edges & ((uint64_t)mask << (64 - 8 * (i + 1))) ? 'o' : '.';
      mask >>= 1;
      j++;
    }
    s[i * 9 + j] = '\n';
  }

  for (int i = 0; i < 8; i++) {
    int j = 0;
    uint8_t mask = (uint8_t)1 << ((sizeof(uint8_t) << 3) - 1);
    while(mask) {
      if (piece & ((uint64_t)mask << (64 - 8 * (i + 1)))) {
        s[i * 9 + j] = '@';
      }
      mask >>= 1;
      j++;
    }
  }

  printf("%s", s);
}

void move_right(uint64_t *piece) {
  if (((*piece) & edges) == 0) {
    *piece >>= 1;
  }
}

void move_left(uint64_t *piece) {
  if (((*piece) & edges) == 0) {
    *piece <<= 1;
  }
}

typedef struct {
} cave_t;

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

  printf("%s\n", jets->jets);

  fclose(fp);

  if (line) {
    free(line);
  }

  // for (int i_rock = 0; i_rock < 1; i_rock++) {
  //   bool stopped = false;
  // }
  
  uint64_t piece = pieces[1];
  print_piece(piece);
  printf("\n");
  for (int i = 0; i < 4; i++) {
    move_left(&piece);
    print_piece(piece);
    printf("\n");
  }

  jets_free(jets);

  return 0;
}
