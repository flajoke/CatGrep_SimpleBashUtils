#ifndef SCHRADER_COMMON
#define SCHRADER_COMMON

#define false (0 != 0)
#define true (0 == 0)
#define byte_value(x) (*(byte *)&(x))
#define bit(x) (1 << x)

#define as(type) *(type *)&

typedef unsigned char byte;

typedef struct Status {
  byte malloc_failure : 1;  // 1
  byte wrong_argument : 1;  // 2
  byte file_failure : 1;    // 4
} Status;

typedef struct arg_data {
  int count;
  char **vector;
} arg_data;

enum {
  success = 0,
  fail = 1,
  end = -1,
};

#endif  // SCHRADER_COMMON
