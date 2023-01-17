#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef union {
  int i;
  struct packet_t *sub_packet;
} packet_data_t;

typedef struct {
  packet_data_t data;
  bool is_sub_packet;
} packet_content_t;

typedef struct packet_t {
  packet_content_t content[64];
  size_t size;
  bool is_divider;
} packet_t;

packet_t *packet_new() { return calloc(1, sizeof(packet_t)); }

void packet_destroy(packet_t *packet) {
  for (int i = 0; i < packet->size; i++) {
    if (packet->content[i].is_sub_packet) {
      packet_destroy(packet->content[i].data.sub_packet);
    }
  }
  free(packet);
}

size_t packet_len(char *string) {
  size_t packet_len = 0;
  size_t depth = 0;
  for (; packet_len < strlen(string); packet_len++) {
    switch (string[packet_len]) {
    case '[':
      depth++;
      break;
    case ']':
      depth--;
      break;
    }

    if (depth == 0)
      break;
  }
  return packet_len;
}

packet_t *packet_parse(char *string, size_t len) {
  packet_t *packet = packet_new();
  size_t sublen;

  for (int i = 0; i < len; i++) {
    switch (string[i]) {
    case '[':
      if (i == 0)
        break;
      sublen = packet_len(string + i);
      packet_content_t content = {.data.sub_packet =
                                      packet_parse(string + i, sublen),
                                  .is_sub_packet = true};
      packet->content[packet->size] = content;
      packet->size++;
      i = i + sublen - 1;
      break;
    case ']':
      break;
    case ',':
      break;
    case '\n':
      break;
    default: // should be int
    {
      packet_content_t content = {.data = atoi(&string[i]),
                                  .is_sub_packet = false};
      packet->content[packet->size] = content;
      packet->size++;
      while (isdigit(string[i + 1]))
        i++;
    }; break;
    }
  }

  return packet;
}

void packet_print(packet_t *packet) {
  printf("[");
  for (int i = 0; i < packet->size; i++) {
    if (i > 0)
      printf(",");
    if (packet->content[i].is_sub_packet) {
      packet_print(packet->content[i].data.sub_packet);
    } else {
      printf("%d", packet->content[i].data.i);
    }
  }
  printf("]");
}

int packet_compare(packet_t *left, packet_t *right) {
  size_t min_size = left->size > right->size ? right->size : left->size;

  packet_t temp_packet = {.content = {{.data.i = 0, .is_sub_packet = false}},
                          .size = 1};

  for (int i = 0; i < min_size; i++) {
    packet_content_t cleft = left->content[i];
    packet_content_t cright = right->content[i];

    if (cleft.is_sub_packet && cright.is_sub_packet) {

      int result =
          packet_compare(cleft.data.sub_packet, cright.data.sub_packet);
      if (result != 0)
        return result;

    } else if (cleft.is_sub_packet || cright.is_sub_packet) {

      if (!cleft.is_sub_packet) {
        temp_packet.content[0].data.i = cleft.data.i;
        int result = packet_compare(&temp_packet, cright.data.sub_packet);
        if (result != 0)
          return result;
      } else {
        temp_packet.content[0].data.i = cright.data.i;
        int result = packet_compare(cleft.data.sub_packet, &temp_packet);
        if (result != 0)
          return result;
      }

    } else {

      if (cleft.data.i == cright.data.i)
        continue;
      return cleft.data.i < cright.data.i ? -1 : 1;
    }
  }

  if (left->size == right->size) {
    return 0;
  } else if (left->size > right->size) {
    return 1;
  } else {
    return -1;
  }
}

int packetpcmp(const void *p1, const void *p2) {
  return packet_compare(*(packet_t **)p1, *(packet_t **)p2);
}

int main(int argc, char *argv[]) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  char *filename = argv[1];

  fp = fopen(filename, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  packet_t *packets[512];

  size_t packet_idx = 0;

  while ((read = getline(&line, &len, fp)) != -1) {
    if (line[0] == '\n')
      continue;

    size_t len = packet_len(line);
    packets[packet_idx] = packet_parse(line, len);

    packet_idx++;
  }

  fclose(fp);

  if (line)
    free(line);

  size_t total = 0;

  for (int i = 0; i < packet_idx; i += 2) {
    // packet_print(packets[i]);
    // printf("\n");
    // packet_print(packets[i + 1]);
    // printf("\n");
    int res = packet_compare(packets[i], packets[i + 1]);
    // printf("%d\n", res);
    // printf("\n");

    if (res == -1)
      total += (i / 2) + 1;
  }

  printf("Part 1: %d\n", total);

  char *divider1 = "[[2]]";
  char *divider2 = "[[6]]";
  packets[packet_idx] = packet_parse(divider1, packet_len(divider1));
  packets[packet_idx]->is_divider = true;
  packet_idx++;
  packets[packet_idx] = packet_parse(divider2, packet_len(divider2));
  packets[packet_idx]->is_divider = true;
  packet_idx++;

  qsort(packets, packet_idx, sizeof(packet_t *), &packetpcmp);

  total = 1;
  for (int i = 0; i < packet_idx; i++) {
    // packet_print(packets[i]);
    // printf("\n");
    if (packets[i]->is_divider) {
      total *= i + 1;
    }
  }
  printf("Part 2: %d\n", total);

  for (int i = 0; i < packet_idx; i++) {
    packet_destroy(packets[i]);
  }

  exit(EXIT_SUCCESS);

  return 0;
}
