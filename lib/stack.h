#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stdlib.h>

#define QUEUE(name, type, size)                                                \
  typedef struct {                                                             \
    type *data;                                                                \
    size_t read;                                                               \
    size_t write;                                                              \
    size_t count;                                                              \
  } queue_##name##_t;                                                          \
  queue_##name##_t *queue_##name##_new() {                                     \
    queue_##name##_t *_queue = calloc(1, sizeof(queue_##name##_t));            \
    _queue->data = calloc(size, sizeof(type));                                 \
    _queue->read = 0;                                                          \
    _queue->write = 0;                                                         \
    _queue->count = 0;                                                         \
    return _queue;                                                             \
  }                                                                            \
  void queue_##name##_push(queue_##name##_t *queue, type item) {               \
    if ((queue->write + 1 == queue->read) ||                                   \
        (queue->read == 0 && queue->write + 1 == size)) {                      \
      perror("Overflow\n");                                                    \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
    queue->data[queue->write] = item;                                          \
    queue->write++;                                                            \
    queue->count++;                                                            \
    if (queue->write >= size)                                                  \
      queue->write = 0;                                                        \
  }                                                                            \
  type queue_##name##_pop(queue_##name##_t *queue) {                           \
    if (queue->read == queue->write) {                                         \
      perror("Overflow\n");                                                    \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
    type res = queue->data[queue->read];                                       \
    queue->read++;                                                             \
    queue->count--;                                                            \
    if (queue->read >= size)                                                   \
      queue->read = 0;                                                         \
    return res;                                                                \
  }                                                                            \
  size_t queue_##name##_len(queue_##name##_t *queue) { return queue->count; }  \
  bool queue_##name##_empty(queue_##name##_t *queue) {                         \
    return queue->read == queue->write;                                        \
  }

#define STACK(name, type, size)                                                \
  typedef struct {                                                             \
    type data[size];                                                           \
    size_t top;                                                                \
  } stack_##name##_t;                                                          \
  stack_##name##_t *stack_##name##_new() {                                     \
    stack_##name##_t *_stack = calloc(1, sizeof(stack_##name##_t));            \
    _stack->top = 0;                                                           \
    return _stack;                                                             \
  }                                                                            \
  void stack_##name##_push(stack_##name##_t *stack, type item) {               \
    if (stack->top + 1 > size) {                                               \
      perror("Overflow\n");                                                    \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
    stack->data[stack->top] = item;                                            \
    stack->top++;                                                              \
  }                                                                            \
  type stack_##name##_pop(stack_##name##_t *stack) {                           \
    if (stack->top == 0) {                                                     \
      perror("Underflow\n");                                                   \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
    stack->top--;                                                              \
    type res = stack->data[stack->top];                                        \
    return res;                                                                \
  }                                                                            \
  size_t stack_##name##_len(stack_##name##_t *stack) { return stack->top; }    \
  bool stack_##name##_empty(stack_##name##_t *stack) { return stack->top == 0; }

// typedef int type;
// #define size 64

#define HEAP(name, type, size)                                                 \
  typedef struct {                                                             \
    type data[size];                                                           \
    int (*cmp)(const type, const type);                                        \
    size_t count;                                                              \
  } heap_##name##_t;                                                           \
  heap_##name##_t *heap_##name##_new(int (*cmp)(const type, const type)) {     \
    heap_##name##_t *_heap = calloc(1, sizeof(heap_##name##_t));               \
    _heap->count = 0;                                                          \
    _heap->cmp = cmp;                                                          \
    return _heap;                                                              \
  }                                                                            \
  void heap_##name##_down(heap_##name##_t *heap, size_t index) {               \
    if (index >= heap->count)                                                  \
      return;                                                                  \
    int left = (int)index * 2 + 1;                                             \
    int right = (int)index * 2 + 2;                                            \
    int leftflag = 0;                                                          \
    int rightflag = 0;                                                         \
                                                                               \
    type minimum = heap->data[index];                                          \
    if (left < heap->count && heap->cmp(minimum, heap->data[left]) == 1) {     \
      minimum = heap->data[left];                                              \
      leftflag = 1;                                                            \
    }                                                                          \
    if (right < heap->count && heap->cmp(minimum, heap->data[right]) == 1) {   \
      minimum = heap->data[right];                                             \
      leftflag = 0;                                                            \
      rightflag = 1;                                                           \
    }                                                                          \
    if (leftflag) {                                                            \
      heap->data[left] = heap->data[index];                                    \
      heap->data[index] = minimum;                                             \
      heap_##name##_down(heap, left);                                          \
    }                                                                          \
    if (rightflag) {                                                           \
      heap->data[right] = heap->data[index];                                   \
      heap->data[index] = minimum;                                             \
      heap_##name##_down(heap, right);                                         \
    }                                                                          \
  }                                                                            \
  void heap_##name##_up(heap_##name##_t *heap, size_t index) {                 \
    int parent = ((int)index - 1) / 2;                                         \
    if (parent < 0) {                                                          \
      return;                                                                  \
    }                                                                          \
    if (heap->cmp(heap->data[index], heap->data[parent]) == -1) {              \
      type temp = heap->data[index];                                           \
      heap->data[index] = heap->data[parent];                                  \
      heap->data[parent] = temp;                                               \
      heap_##name##_up(heap, parent);                                          \
    }                                                                          \
  }                                                                            \
  void heap_##name##_push(heap_##name##_t *heap, type item) {                  \
    heap->data[heap->count] = item;                                            \
    heap->count++;                                                             \
    heap_##name##_up(heap, heap->count - 1);                                   \
  }                                                                            \
  type heap_##name##_pop(heap_##name##_t *heap) {                              \
    if (heap->count <= 0) {                                                    \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
    if (heap->count == 1) {                                                    \
      heap->count--;                                                           \
      return heap->data[0];                                                    \
    }                                                                          \
    type root = heap->data[0];                                                 \
    heap->data[0] = heap->data[heap->count - 1];                               \
    heap->count--;                                                             \
    heap_##name##_down(heap, 0);                                               \
    return root;                                                               \
  }                                                                            \
  size_t heap_##name##_len(heap_##name##_t *heap) { return heap->count; }      \
  bool heap_##name##_empty(heap_##name##_t *heap) { return heap->count == 0; }

#endif
