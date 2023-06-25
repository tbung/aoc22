#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long snafutol(const char *str) {
  /* printf("%s\n", str); */
  size_t len = strlen(str);
  long result = 0;
  for (size_t i = 0; i < len; i++) {
    /* printf("%ld: %c\n", (long)pow(5, i), str[len - 1 - i]); */
    switch (str[len - 1 - i]) {
    case '=':
      result += -2.0 * pow(5, i);
      break;
    case '-':
      result += -1.0 * pow(5, i);
      break;
    case '0':
      break;
    case '1':
      result += pow(5, i);
      break;
    case '2':
      result += 2.0 * pow(5, i);
      break;
    default:
      printf("Error: \"%c\"\n", str[len - 1 - i]);
      exit(EXIT_FAILURE);
    }
  }

  return result;
}

static const char rem2char[] = {'0', '1', '2', '=', '-'};

char *ltosnafu(const unsigned long x) {
  unsigned long result = x;
  char *str = calloc(128, sizeof(char));
  size_t n = 0;
  while (result > 0) {
    double rem = fmod(result, 5.0);
    result = result / 5;
    str[n++] = rem2char[(int)rem];
    if (rem > 2) {
      result++;
    }
  }

  char *rev = calloc(n, sizeof(char));
  for (size_t i = 0; i < n; i++) {
    rev[i] = str[n - 1 - i];
  }

  return rev;
}

int main(int argc, char *argv[]) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;

  char *filename = argv[1];

  fp = fopen(filename, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  unsigned long result = 0;
  while (getline(&line, &len, fp) != -1) {
    line[strcspn(line, "\n")] = 0;
    result += snafutol(line);
    /* printf("%lu\n", snafutol(line)); */
  }

  /* printf("%lu\n", result); */
  printf("%s\n", ltosnafu(result));

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
