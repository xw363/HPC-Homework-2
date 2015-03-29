/*
 * FILE: mpi_solved7.c
 * DESCRIPTION:
 *   Fixed incorrect count that is passed to MPI_Bcast().
 */
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
int   numtasks, taskid, len, buffer, root, count;
char hostname[MPI_MAX_PROCESSOR_NAME];

MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Get_processor_name(hostname, &len);

printf ("Task %d on %s starting...\n", taskid, hostname);
buffer = 23;
root = 0;
count = 1;  /* Fixed count of numbers to be broadcast */
if (taskid == root)
   printf("Root: Number of MPI tasks is: %d\n",numtasks);

MPI_Bcast(&buffer, count, MPI_INT, root, MPI_COMM_WORLD);


MPI_Finalize();

}

