/*
 * FILE: mpi_solved5.c
 * DESCRIPTION: 
 *   Re-structured the while loop and set a MPI_Barrier for both ranks.
 */
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define MSGSIZE 2000

int main (int argc, char *argv[])
{
int        numtasks, rank, i, tag=111, dest=1, source=0, count=0;
char       data[MSGSIZE];
double     start, end, result;
MPI_Status status;

MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);

if (rank == 0) {
  printf ("mpi_solved5 has started...\n");
  if (numtasks > 2) 
    printf("INFO: Number of tasks= %d. Only using 2 tasks.\n", numtasks);

  /* Initialize send data */
  for(i=0; i<MSGSIZE; i++)
     data[i] =  'x';
  }

while (1) {
  /***************************** Send task ********************************/
  if (rank == 0) {
    start = MPI_Wtime();
    MPI_Send(data, MSGSIZE, MPI_BYTE, dest, tag, MPI_COMM_WORLD);
    count++;
    if (count % 10 == 0) {
      end = MPI_Wtime();
      printf("Count= %d  Time= %f sec.\n", count, end-start);
      start = MPI_Wtime();
      }
    }

  /**************************** Receive task ******************************/

  if (rank == 1) {
    MPI_Recv(data, MSGSIZE, MPI_BYTE, source, tag, MPI_COMM_WORLD, &status);
    /* Do some work  - at least more than the send task */
    result = 0.0;
    for (i=0; i < 1000000; i++) 
      result = result + (double)random();
    }

  MPI_Barrier(MPI_COMM_WORLD);
  }

MPI_Finalize();
}

