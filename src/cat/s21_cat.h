#ifndef S21_GREP_SCHRADER
#define S21_GREP_SCHRADER

#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

void usage();
void scanfiles(char *argv[], int cooked);
void ready(FILE *);
void unready(int);

#endif  // S21_GREP_SCHRADER