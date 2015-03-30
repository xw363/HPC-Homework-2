/*
 * Parallel sample sort
 */
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>


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
  int rank, P, root = 0;
  int i, j, N, N_recv, S;
  int *vec, *vec_recv;
  int *local_splitter, *global_splitter;
  int *send_count, *recv_count;
  int *send_displacement, *recv_displacement;
  int splitter_tag = 1, count_tag = 2;
  FILE *fid;
  char filename[25], buf[5];

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &P);
  MPI_Status status;
  MPI_Request request[2 * P];

  /* Number of random numbers per processor (this should be increased
   * for actual tests or could be made a passed in through the command line */
  if (argc > 1) {
    N = atoi(argv[1]);
  } else {
    N = 100;
  }

  vec = calloc(N, sizeof(int));
  /* seed random number generator differently on every core */
  srand((unsigned int) rank + 393919);

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
    local_splitter[i] = vec[N / (S + 1) * (i + 1)];
  }

  /* every processor communicates the selected entries
   * to the root processor */
  if (rank == root) {
    for (i = 1; i < P; ++i) {
      MPI_Recv(&local_splitter[i * S], S, MPI_INT, i, splitter_tag,
        MPI_COMM_WORLD, &status);
    }
  }
  else {
    MPI_Send(local_splitter, S, MPI_INT, root, splitter_tag, MPI_COMM_WORLD);
  }

  /* root processor does a sort, determinates splitters and broadcasts them */
  global_splitter = malloc((P - 1) * sizeof(int));

  if (rank == root) {
    qsort(local_splitter, S * P, sizeof(int), compare);
    for (i = 0; i < P - 1; ++i) {
      global_splitter[i] = local_splitter[(i + 1) * S];
    }
  }

  MPI_Bcast(global_splitter, P - 1, MPI_INT, root, MPI_COMM_WORLD);

  /* every processor uses the obtained splitters to decide to send
   * which integers to whom */
  send_count = malloc(P * sizeof(int));
  send_displacement = malloc(P * sizeof(int));
  recv_count = malloc(P * sizeof(int));
  recv_displacement = malloc(P * sizeof(int));

  for (i = 0; i < P; ++i) {
    send_count[i] = 0;
    send_displacement[i] = 0;
  }
  j = 0;
  for (i = 0; i < N; ++i) {
    if (j < P - 1 && vec[i] > global_splitter[j]) {
      j++;
    }

    send_count[j]++;
  }

  for (i = 0; i < P; ++i) {
    MPI_Isend(&send_count[i], 1, MPI_INT, i, count_tag, MPI_COMM_WORLD,
      &request[i]);
  }

  for (i = 0; i < P; ++i) {
    MPI_Irecv(&recv_count[i], 1, MPI_INT, i, count_tag, MPI_COMM_WORLD,
      &request[i + P]);
  }

  MPI_Waitall(2 * P, request, MPI_STATUSES_IGNORE);

  for (i = 0; i < P; ++i) {
    if (i == 0) {
      send_displacement[i] = 0;
    } else {
      send_displacement[i] = send_displacement[i - 1] + send_count[i - 1];
    }

    if (i == 0) {
      recv_displacement[i] = 0;
    } else {
      recv_displacement[i] = recv_displacement[i - 1] + recv_count[i - 1];
    }
  }

  N_recv = recv_displacement[P - 1] + recv_count[P - 1];
  vec_recv = malloc(N_recv * sizeof(int));

  /* send and receive: either you use MPI_AlltoallV, or
   * (and that might be easier), use an MPI_Alltoall to share
   * with every processor how many integers it should expect,
   * and then use MPI_Send and MPI_Recv to exchange the data */
  MPI_Alltoallv(vec, send_count, send_displacement, MPI_INT, vec_recv,
    recv_count, recv_displacement, MPI_INT, MPI_COMM_WORLD);

  /* local sort */
  qsort(vec_recv, N_recv, sizeof(int), compare);

  /* every processor writes its result to a file */
  strcpy(filename, "sorted_vec_");
  sprintf(buf, "%03d", rank);
  strcat(filename, buf);
  strcat(filename, ".txt");
  fid = fopen(filename, "w");
  for (i = 0; i < N_recv; ++i) {
    fprintf(fid, "%d ", vec_recv[i]);
  }
  fclose(fid);

  free(vec);
  free(vec_recv);
  free(local_splitter);
  free(global_splitter);
  free(send_count);
  free(send_displacement);
  free(recv_count);
  free(recv_displacement);

  MPI_Finalize();
  return 0;
}
