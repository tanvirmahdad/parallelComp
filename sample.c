/* To scatter a buffer to all processes using non-blocking communication operations.  */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

#define NTIMES 100

void myscatter(void *sendbuf, int sendcount, MPI_Datatype sendtype, 
               void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);

int main(int argc, char **argv)
{
    int         i, rank,N, size, root=0, offset;
    int         *sendbuf=NULL, *recvbuf;
    double      t1, t2;

    MPI_Init (&argc, &argv);

    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
   N=8;
    if (rank == root) {
	sendbuf = (int *)malloc(sizeof(int)*N*size);
	for (i=0; i<N*size; i++)
	    sendbuf[i] = i;
    }
    recvbuf = (int *)malloc(sizeof(int)*N);
    
   if(rank==0){
      for(i=0;i<N*size;i++){
         printf("%d\n",sendbuf[i]);
      }
   }
    
    /* setup a synchronization point */
    MPI_Barrier(MPI_COMM_WORLD);
    t1 = MPI_Wtime();

    

    /* program end here */
    t2 = MPI_Wtime() - t1;

    MPI_Reduce(&t2, &t1, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
       printf("Time taken in process 0 = %g\n", t2);
       printf("Maximum time taken among all processes = %g\n", t1);
    }

    MPI_Finalize ();

    return 0;
}

void myscatter(void *sendbuf, int sendcount, MPI_Datatype sendtype, 
               void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) {
    int rank, size, i, offset;
    MPI_Status  *status; 
    MPI_Request *request;
    MPI_Aint lb, sizeofsendtype, sizeofrecvtype;

    MPI_Comm_rank (comm, &rank);
    MPI_Comm_size (comm, &size);

    status = malloc(sizeof(MPI_Status)*(size+1));
    request = malloc(sizeof(MPI_Request)*(size+1));

    if (rank == root) { /* If I'm the root node, send it to others */
	for (i = 0; i < size; i++) {
	    MPI_Type_get_extent(sendtype, &lb, &sizeofsendtype);
	    offset = sizeofsendtype*sendcount*i;
	    char *bufptr = sendbuf + offset;
            MPI_Isend(bufptr, sizeofsendtype*sendcount, MPI_CHAR, i, 0, comm, &request[i]);
        }
        MPI_Type_get_extent(recvtype, &lb, &sizeofrecvtype);
        MPI_Irecv(recvbuf, sizeofrecvtype*recvcount, MPI_CHAR, root, 0, comm, &request[i]);
        MPI_Waitall(size+1, request, status);
    } else { /* all other processes receive from process with rank root */
        MPI_Type_get_extent(recvtype, &lb, &sizeofrecvtype);
        MPI_Recv(recvbuf, sizeofrecvtype*recvcount, MPI_CHAR, root, 0, comm, MPI_STATUS_IGNORE);
    }

    free(request);
    free(status);
}

