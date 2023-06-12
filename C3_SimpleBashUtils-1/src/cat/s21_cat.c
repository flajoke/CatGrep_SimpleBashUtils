#include "s21_cat.h"

int bflag, eflag, nflag, sflag, tflag, vflag;
int rval;
const char *filename;

int main(int argc, char *argv[]) {
  static struct option long_options[] = {
      {"number-nonblank", no_argument, NULL, 'b'},
      {"number", no_argument, NULL, 'n'},
      {"squeeze-blank", no_argument, NULL, 's'},
  };
  int ch;
  while ((ch = getopt_long(argc, argv, "+benstvTE", long_options, NULL)) != -1)
    switch (ch) {
      case 'b':
        bflag = nflag = 1;
        break;
      case 'e':
        eflag = vflag = 1;
        break;
      case 'E':
        eflag = 1;
        break;
      case 'n':
        nflag = 1;
        break;
      case 's':
        sflag = 1;
        break;
      case 't':
        tflag = vflag = 1;
        break;
      case 'T':
        tflag = 1;
        break;
      case 'v':
        vflag = 1;
        break;
      default:
        usage();
    }

  argv += optind;

  if (bflag || eflag || nflag || sflag || tflag || vflag)
    scanfiles(argv, 1);
  else
    scanfiles(argv, 0);
  if (fclose(stdout)) err(1, "stdout");
  exit(rval);
}

void usage() {
  fprintf(stderr, "usage: cat [-benstTE] [file ...]\n");
  exit(1);
}

void scanfiles(char *argv[], int cooked) {
  int i = 0;
  char *path;
  FILE *fp;

  while ((path = argv[i]) != NULL || i == 0) {
    int fd;

    if (path == NULL || strcmp(path, "-") == 0) {
      filename = "stdin";
      fd = STDIN_FILENO;
    } else {
      filename = path;
      fd = open(path, O_RDONLY);
    }
    if (fd < 0) {
      warn("%s", path);
      rval = 1;
    } else if (cooked) {
      if (fd == STDIN_FILENO)
        ready(stdin);
      else {
        fp = fdopen(fd, "r");
        ready(fp);
        fclose(fp);
      }
    } else {
      unready(fd);
      if (fd != STDIN_FILENO) close(fd);
    }
    if (path == NULL) break;
    ++i;
  }
}

void ready(FILE *fp) {
  int ch, gobble, line, prev;
  if (fp == stdin && feof(stdin)) clearerr(stdin);
  line = gobble = 0;
  for (prev = '\n'; (ch = getc(fp)) != EOF; prev = ch) {
    if (prev == '\n') {
      if (sflag) {
        if (ch == '\n') {
          if (gobble) continue;
          gobble = 1;
        } else
          gobble = 0;
      }
      if (nflag && (!bflag || ch != '\n')) {
        fprintf(stdout, "%6d\t", ++line);
        if (ferror(stdout)) break;
      }
    }
    if (ch == '\n') {
      if (eflag && putchar('$') == EOF) break;
    } else if (ch == '\t') {
      if (tflag) {
        if (putchar('^') == EOF || putchar('I') == EOF) break;
        continue;
      }
    } else if (vflag) {
      if (!isascii(ch) && !isprint(ch)) {
        if (putchar('M') == EOF || putchar('-') == EOF) break;
        ch = toascii(ch);
      }
      if (iscntrl(ch)) {
        if (putchar('^') == EOF ||
            putchar(ch == '\177' ? '?' : ch | 0100) == EOF)
          break;
        continue;
      }
    }
    if (putchar(ch) == EOF) break;
  }
}

void unready(int rfd) {
  int off, wfd;
  ssize_t nr, nw;
  size_t bsize;
  char *buf = NULL;
  struct stat sbuf;

  wfd = fileno(stdout);
  if (buf == NULL) {
    if (fstat(wfd, &sbuf)) err(1, "%s", filename);
    bsize = MAX(sbuf.st_blksize, 1024);
    if ((buf = malloc(bsize)) == NULL) err(1, "buffer");
  }
  while ((nr = read(rfd, buf, bsize)) > 0)
    for (off = 0; nr; nr -= nw, off += nw)
      if ((nw = write(wfd, buf + off, (size_t)nr)) < 0) err(1, "stdout");
}
