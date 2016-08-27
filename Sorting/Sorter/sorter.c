// sorter.c ...sorting algorithm analysis
// Written by John Shepherd, April 2013
// Last modified August 2016 (added file reading)

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

// ints
#define show(A) { printf("%02d",(A)); }
int less(int, int);
void swap(int *, int, int);
void move(int *, int, int);

// book-keeping functions
void usage(char *);

// functions on arrays of ints
void display(int *, int, int);
int isSorted(int *, int, int);
void bubbleSort(int *, int, int);
void insertionSort(int *, int, int);
void mergeSort(int *, int, int);
void quickSort(int *, int, int);
void selectionSort(int *, int, int);
void shellSort(int *, int, int);

// global(!) variables for monitoring
int ncompares = 0;
int nswaps = 0;
int nmoves = 0;
int *A; // array to be sorted
int N;  // number of elements
int MAX_IN = 100000;

int main (int argc, char *argv[])
{ 
	int i;     // index
	int x;     // random number
	int s;     // starting value
	char ord;  // initial order
	FILE *inf; // input file
	
	if (argc < 4) usage(argv[0]);
	N = atoi(argv[2]);
	ord = argv[3][0];
	if (N == 0) { ord = 'I'; N = MAX_IN; }
	// if (N < 5 || N > 90) usage(argv[0]);
	if (argc == 5) s = atoi(argv[4]);
	A = malloc(N*sizeof(int));
	assert(A != NULL);

	// seed random number generator
	srand(getpid());

	// generate data
	switch (ord) {
	case 'A':
		// ascending sequence in range 0..N
		x = (s > 0) ? s : 1;
		for (i = 0; i < N; i++)
			A[i] = x++;
		break;
	case 'N':
		// nearly ascending sequence in range 0..N
		x = (s > 0) ? s : 1;
		for (i = 0; i < N; i++)
			A[i] = x++;
		for (i = 0; i < 4; i++) {
			int t,x,y;
			x = rand()%N;
			y = rand()%N;
			t = A[x]; A[x] = A[y]; A[y] = t;
		}
		break;
	case 'D':
		// descending sequence in range 0..N
		x = (s > 0) ? s : N;
		for (i = 0; i < N; i++)
			A[i] = x--;
		break;
	case 'R':
		// random sequence in range 0..N, no duplicates
		x = (s > 0) ? s : 1;
		for (i = 0; i < N; i++)
			A[i] = x++;
		for (i = 0; i < N; i++) {
			x = rand()%N;
			int t = A[i]; A[i] = A[x]; A[x] = t;
		}
		break;
	case 'I':
		// read from stdin
		inf = fopen(argv[3],"r");
		if (inf == NULL) {
			fprintf(stderr,
			        "Can't open file %s\n",
			        argv[3]);
			exit(EXIT_FAILURE);
		}
		for (i = 0; i < MAX_IN; i++) {
			int v, status;
			status = fscanf(inf, "%d", &v);
			if (status != 1) break;
			A[i] = v;
		}
		fclose(inf);
		N = i;
		break;
	default:
		usage(argv[0]);
		break;
	}
	//printf("Before: "); display(A, 0, N-1);

	// sort data
	switch (argv[1][0]) {
	case 'b': bubbleSort(A, 0, N-1); break;
	case 'i': insertionSort(A, 0, N-1); break;
	case 'h': shellSort(A, 0, N-1); break;
	case 'm': mergeSort(A, 0, N-1); break;
	case 'q': quickSort(A, 0, N-1); break;
	case 's': selectionSort(A, 0, N-1); break;
	default: usage(argv[0]); break;
	}

	// display results
	if (ord == 'I') display(A, 0, N-1);
	printf(isSorted(A, 0, N-1) ? "OK" : "Fail");
	printf("  #C: %d, #S: %d, #M: %d\n",
	       ncompares, nswaps, nmoves);

	return 0;
}

// functions on ints

int less(int x, int y)
{
	ncompares++;
	return (x < y);
}

int eq(int x, int y)
{
	ncompares++;
	return (x == y);
}

void swap(int a[], int i, int j)
{
	nswaps++;
	int tmp = a[i]; a[i] = a[j]; a[j] = tmp;
}

void move(int a[], int i, int j)
{
	nmoves++;
	a[i] = a[j];
}

void copy(int a[], int *i, int b[], int *j)
{
	nmoves++;
	a[*i] = b[*j];
    *i = *i + 1;
    *j = *j + 1;
}


// Book-keeping functions

void usage(char *prog)
{
	fprintf(stderr,
	        "Usage: %s Algo Size Dist [Seed]\n", prog);
	fprintf(stderr,
	        "Algo = b|i|h|m|q|s, Dist = A|D|R|N\n");
	exit(1);
}


// Functions on arrays of ints

// show a[lo..hi] on stdout
void display(int a[], int lo, int hi)
{
	int i;
	for (i = lo; i <= hi; i++) {
		printf(" %d",a[i]);
	}
	putchar('\n');
}

// show A[0..N-1] on stdout
void monitor(char *label)
{
	printf("%s: ",label); display(A, 0, N-1);
}

// is a[lo..hi] sorted?
int isSorted(int a[], int lo, int hi)
{
	int i;

	for (i = lo; i < hi-1; i++)
		if (a[i] > a[i+1]) return 0;
	return 1;
}


// Sorting Algorithms

// bubble sort
void bubbleSort(int a[], int lo, int hi)
{
   int i, j, nswaps;
   for (i = lo; i < hi; i++) {
      nswaps = 0;
      for (j = hi; j > i; j--) {
         if (less(a[j], a[j-1])) {
            swap(a,j,j-1);
            nswaps++;
         }
      }
      if (nswaps == 0) break;
   }
}

// insertion sort

void insertionSort(int a[], int lo, int hi)
{
   int i, j, min, val;
   min = lo;
   for (i = lo+1; i <= hi; i++)
      if (less(a[i],a[min])) min = i;
   swap(a, lo, min);
   for (i = lo+2; i <= hi; i++) {
      val = a[i];
      for (j = i; less(val,a[j-1]); j--)
         move(a, j, j-1);
      a[j] = val;
   }
}

// shell sort

void shellSort(int a[], int lo, int hi)
{
	int gaps[8] = {701, 301, 132, 57, 23, 10, 4, 1};
	//int gaps[6] = {1073, 281, 77, 23, 8, 1};
	int g, gap, start, i, j, val;

	for (g = 0; g < 8; g++) {
		gap = gaps[g];
		start = lo + gap;
		for (i = start; i < hi; i++) {
			val = a[i];
			for (j = i; j >= start && less(val,a[j-gap]); j -= gap)
				move(a, j, j-gap);
			a[j] = val;
		}
	}
}

// mergesort

void merge(int a[], int lo, int mid, int hi)
{
   int  i, j, k, nitems = hi-lo+1;
   int *tmp = malloc(nitems*sizeof(int));

   i = lo; j = mid+1; k = 0;
   while (i <= mid && j <= hi) {
     if (less(a[i],a[j]))
        copy(tmp, &k, a, &i);
	 else
        copy(tmp, &k, a, &j);
   }
   while (i <= mid) copy(tmp, &k, a, &i);
   while (j <= hi) copy(tmp, &k, a, &j);

   //copy back
   for (i = lo, k = 0; i <= hi; i++, k++) {
      a[i] = tmp[k];
      nmoves++;
   }
   free(tmp);
}
void mergeSort(int a[], int lo, int hi)
{
   int mid = (lo+hi)/2; // mid point
   if (hi <= lo) return;
   mergeSort(a, lo, mid);
   mergeSort(a, mid+1, hi);
   merge(a, lo, mid, hi);
}

// quicksort

int partition(int a[], int lo, int hi)
{
   int v = a[lo];  // pivot
   int i = lo+1, j = hi;
   for (;;) {
      while (!less(v,a[i]) && i < j) i++;
      while (!less(a[j],v) && j > i) j--;
      if (i == j) break;
      swap(a,i,j);
   }
   j = less(a[i],v) ? i : i-1;
   swap(a,lo,j);
   return j;
}

void quickSort(int a[], int lo, int hi)
{
   int i; // index of pivot
   //printf("qsort(a,%d,%d)\n",lo,hi);
   if (lo >= hi) return;
   i = partition(a, lo, hi);
   quickSort(a, lo, i-1);
   quickSort(a, i+1, hi);
}

// selection sort
void selectionSort(int a[], int lo, int hi)
{
   int i, j, min;
   for (i = lo; i < hi; i++) {
      min = i;
      for (j = i+1; j <= hi; j++) {
         if (less(a[j],a[min])) min = j;
      }
      swap(a,i,min);
   }
}