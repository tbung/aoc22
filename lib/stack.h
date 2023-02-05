#ifndef STACK_H
#define STACK_H

#define QUEUE(name, type, size)                                                \
  typedef struct {                                                             \
    type data[size];                                                           \
    size_t read;                                                               \
    size_t write;                                                              \
    size_t count;                                                              \
  } queue_##name##_t;                                                          \
  queue_##name##_t *queue_##name##_new() {                                     \
    queue_##name##_t *_queue = calloc(1, sizeof(queue_##name##_t));            \
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

#endif
