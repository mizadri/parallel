/* Brute Force Sudoku Solver
   Copyright (C) 2005 John D. Ramsdell

   The program use brute force to solve a Sudoku Puzzle given as
   input.  It ignores white space, and the characters hyphen, vertical
   bar, and plus sign.  What should be left is eighty-one non-zero
   digits and periods, which are entered into the initial grid.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details at
   http://www.gnu.org/copyleft/gpl.html. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "omp.h"

#define N 3
#define N2 (N * N)
#define N4 (N2 * N2)

static int limit;		/* Maximum number of solutions */
static int solutions;		/* Solutions currently found */

typedef unsigned short set_t;

#define ALL ((set_t)((1 << N2) - 1))

/* Each cell in the grid contains a set of allowed values.  The set
   has a bit set for each value of the cell that has yet to be
   eliminated as a possible value for this cell.  When a cell contains
   the empty set, the grid is inconsistent.

   A grid is a N*N*N*N array of cells.  In a grid->set[i][j][k][l]
   reference, i is the major row, j is in minor row, k is the major
   column, and l is the minor column. */

typedef struct grid {
/*          R  r  C  c  */
  set_t set[N][N][N][N];
} *grid_t;

/* Returns true if the grid is consistent after determining the
   effects of setting the cell i, j, k, l to set, where set contains
   exactly one element. */

static int uniq(grid_t grid, set_t set, int i, int j, int k, int l)
{
int ii, jj, kk, ll;
int row = 1, square = 1, column = 1; 
  grid->set[i][j][k][l] = set;
  set = ~set;
	
	#pragma omp parallel
	{
	  /* handle row at i, j */
	#pragma omp task firstprivate(kk,ll)
	  for (kk = 0; kk < N; kk++)
		for (ll = 0; ll < N; ll++)
		  if (k != kk) {
		grid->set[i][j][kk][ll] &= set;
		if (!grid->set[i][j][kk][ll])
		  row = 0;
		  }
		
	  /* handle column at k, l */
	#pragma omp task firstprivate(jj,ii)
	  for (ii = 0; ii < N; ii++)
		for (jj = 0; jj < N; jj++)
		  if (i != ii) {
		grid->set[ii][jj][k][l] &= set;
		if (!grid->set[ii][jj][k][l])
		  column = 0;
		  }

	  /* handle square at i, k */
	#pragma omp task firstprivate(ll,jj)
	  for (jj = 0; jj < N; jj++)
		for (ll = 0; ll < N; ll++)
		  if (j != jj || l != ll ) {//j != jj || l != ll 
		grid->set[i][jj][k][ll] &= set;
		if (!grid->set[i][jj][k][ll])
		  square =  0;
		  }
	#pragma omp taskwait
	}
	
	if( !square || !column || !row )
		return 0;
	else
		return 1;
}

/* Input a grid from stdin. */
static int
read_grid(grid_t grid)
{
  int i, j, k, l;
  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++)
      for (k = 0; k < N; k++)
	for (l = 0; l < N; l++)
	  grid->set[i][j][k][l] = ALL;

  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++)
      for (k = 0; k < N; k++)
	for (l = 0; l < N; l++)
	  for (;;) {
	    int ch = getchar();
	    if (ch == EOF)
	      return -1;
	    if (ch == '.')	/* All nine possibilities. */
	      break;
	    if ('1' <= ch && ch <= '9') { /* One possibility. */
	      uniq(grid, 1 << (ch - '1'), i, j, k, l);
	      break;
	    }
	    if (!isspace(ch) && ch != '|' && ch != '-' && ch != '+')
	      return -1;
	  }

  for (;;) {
    int ch = getchar();
    if (ch == EOF)
      return 0;
    if (!isspace(ch) && ch != '|' && ch != '-' && ch != '+')
      return -1;
  }
}

/* Output a grid to stdout. */
static void
print_grid(grid_t grid)
{
  int i, j, k, l, m;
  printf("+---+---+---+\n");
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      for (k = 0; k < N; k++) {
	printf("|");
	for (l = 0; l < N; l++) {
	  set_t set = grid->set[i][j][k][l];
	  int q = -1;
	  int p = 0;
	  for (m = 0; m < N2; m++)
	    if ((1 << m) & set) {
	      p++;
	      q = m;
	    }
	  if (p == 1)
	    printf("%d", q + 1);
	  else if (p > 1)
	    printf(".");
	  else
	    printf("?");
	}
      }
      printf("|\n");
    }
    printf("+---+---+---+\n");
  }
}

/* Use depth first search.  Returns true when a solution is found.
   The parameter p determines the cell which is the focus of this
   recursive call.  From it are derived values for i, j, k, and l. */
static int recur(grid_t old, int p) {
  struct grid new[1];
  set_t m;
  

  if (p >= N4) {		/* Solution found. */
    print_grid(old);
    solutions++;
    return solutions >= limit;
  }
  int q = p++;
  int l = q % N;
  q = q / N;
  int k = q % N;
  q = q / N;
  int j = q % N;
  int i = q / N;
  set_t set = old->set[i][j][k][l];

  for (m = 1; m < ALL; m <<= 1)
    if (m & set) {		/* Try only possible values. */
      memcpy(new, old, sizeof(struct grid));
      if (uniq(new, m, i, j, k, l) && recur(new, p))
	return 1;		/* Success */
    }

  return 0;			/* Failure */
}

static int filter(int bound)
{
  double tinicio, tfin;
  struct grid grid[1];
  if (read_grid(grid)) {
    fprintf(stderr, "Bad input\n");
    return 1;
  }

  limit = bound;
  solutions = 0;
  tinicio = omp_get_wtime();
  recur(grid, 0);
  tfin = omp_get_wtime();
  printf("t(recur)=%g\n", tfin - tinicio);
  if (solutions == 0) {
    fprintf(stderr, "No solution found\n");
    return 1;
  }
  if (solutions > 1) {
    fprintf(stderr, "More than one solution found\n");
    return 1;
  }

  return 0;
}

/* Generic filtering main and usage routines. */

static void
usage(const char *prog)
{
  fprintf(stderr,
	  "Usage: %s [options] [input]\n"
	  "Options:\n"
	  "  -o file -- output to file (default is standard output)\n"
	  "  -l num  -- limit on number of solutions (default is one)\n"
	  "  -h      -- print this message\n",
	  prog);
}

int main(int argc, char **argv)
{
  extern char *optarg;
  extern int optind;

  char *output = 0;
  int limit = 1;

  for (;;) {
    int c = getopt(argc, argv, "o:l:h");
    if (c == -1)
      break;
    switch (c) {
    case 'o':
      output = optarg;
      break;
    case 'l':
      limit = atoi(optarg);
      if (limit <= 0) {
	fprintf(stderr, "Bad limit %d\n", limit);
	return 1;
      }
      break;
    case 'h':
      usage(argv[0]);
      return 0;
    default:
      usage(argv[0]);
      return 1;
    }
  }

  switch (argc - optind) {
  case 0:			/* Use stdin */
    break;
  case 1:
    if (!freopen(argv[optind], "r", stdin)) {
      perror(argv[optind]);
      return 1;
    }
    break;
  default:
    fprintf(stderr, "Bad arg count\n");
    usage(argv[0]);
    return 1;
  }


  if (output && !freopen(output, "w", stdout)) {
    perror(output);
    return 1;
  }

  return filter(limit);
}
