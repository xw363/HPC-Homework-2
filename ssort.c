/*
 * Parallel sample sort
 */
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>


static int compare(const void *a, const void *b)
{
  int *da = (int *)a;
  int *db = (int *)b;

  if (*da > *db)
    return 1;
  else if (*da < *db)
    return -1;
  else
    return 0;
}

int main( int argc, char *argv[])
{
  int rank, P, root = 0, tag = 1;
  int i, N, S;
  int *vec;
  int *local_splitter;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &P);

  /* Number of random numbers per processor (this should be increased
   * for actual tests or could be made a passed in through the command line */
  N = 100;

  vec = calloc(N, sizeof(int));
  /* seed random number generator differently on every core */
  srand48((unsigned int) rank);

  /* fill vector with random integers */
  for (i = 0; i < N; ++i) {
    vec[i] = rand();
  }

  /* sort locally */
  qsort(vec, N, sizeof(int), compare);

  /* randomly sample s entries from vector or select local splitters,
   * i.e., every N/P-th entry of the sorted vector */
  S = 9;
  if (rank == root) {
    local_splitter = malloc(S * P * sizeof(int));
  }
  else {
    local_splitter = malloc(S * sizeof(int));
  }
  for (i = 0; i < S; ++i) {
    local_splitter[i] = vec[N / S * (i + 1)];
    printf("rank = %d, s[%d] = %d\n", rank, i, local_splitter[i]);
  }

  /* every processor communicates the selected entries
   * to the root processor */
  if (rank == root) {
    for (i = 1; i < P; ++i) {
      MPI_Recv(&local_splitter[i * S], S, MPI_INT, i, tag, MPI_COMM_WORLD,
        &status);
    }
  }
  else {
    MPI_Send(local_splitter, S, MPI_INT, root, tag, MPI_COMM_WORLD);
  }

  /* root processor does a sort, determinates splitters and broadcasts them */
  if (rank == root) {
    qsort(local_splitter, S * P, sizeof(int), compare);
    for (i = 0; i < S * P; ++i)
      printf("%d,", local_splitter[i]);
    printf("\n");
  }

  /* every processor uses the obtained splitters to decide to send
   * which integers to whom */

  /* send and receive: either you use MPI_AlltoallV, or
   * (and that might be easier), use an MPI_Alltoall to share
   * with every processor how many integers it should expect,
   * and then use MPI_Send and MPI_Recv to exchange the data */

  /* local sort */

  /* every processor writes its result to a file */

  free(vec);
  free(local_splitter);
  MPI_Finalize();
  return 0;
}
