#include <stdio.h>
#include <stdlib.h>

char decode(char c) {
  if (c == 'A' || c == 'X')
    return 'R';
  if (c == 'B' || c == 'Y')
    return 'P';
  if (c == 'C' || c == 'Z')
    return 'S';

  return 'E';
}

int calculate_score(char a, char b) {
  int score = 0;

  switch (b) {
  case 'X':
    score += 1;
    switch (a) {
    case 'A':
      score += 3;
      break;
    case 'B':
      break;
    case 'C':
      score += 6;
      break;
    }
    break;
  case 'Y':
    score += 2;
    switch (a) {
    case 'A':
      score += 6;
      break;
    case 'B':
      score += 3;
      break;
    case 'C':
      break;
    }
    break;
  case 'Z':
    score += 3;
    switch (a) {
    case 'A':
      break;
    case 'B':
      score += 6;
      break;
    case 'C':
      score += 3;
      break;
    }
    break;
  }

  return score;
}

int calculate_score2(char a, char b) {
  int score = 0;

  switch (b) {
  case 'X':
    switch (a) {
    case 'A':
      score += 3;
      break;
    case 'B':
      score += 1;
      break;
    case 'C':
      score += 2;
      break;
    }
    break;
  case 'Y':
    score += 3;
    switch (a) {
    case 'A':
      score += 1;
      break;
    case 'B':
      score += 2;
      break;
    case 'C':
      score += 3;
      break;
    }
    break;
  case 'Z':
    score += 6;
    switch (a) {
    case 'A':
      score += 2;
      break;
    case 'B':
      score += 3;
      break;
    case 'C':
      score += 1;
      break;
    }
    break;
  }

  return score;
}

int main(int argc, char *argv[]) {
  FILE *fp;
  int n_lines = 0;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  char *filename = argv[1];

  fp = fopen(filename, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  // int scores[1024] = {0};
  int total = 0;

  int current = 0;

  while ((read = getline(&line, &len, fp)) != -1) {
    // current = calculate_score(line[0], line[2]);
    current = calculate_score2(line[0], line[2]);
    // printf("%c, %c -> %d\n", decode(line[0]), decode(line[2]), current);
    total += current;
  }

  printf("%d\n", total);
}
