/****************************************************************************** *******************
 *
 * *  Name: Ahmed Tanvir Mahdad                                                                    *
 * *  BlazerID: mahdad                                                                             *
 * *  Course Section: CS 732                                                                       *
 * *  Homework #5 (Allgather,(Gather+broadcast version))                                           *
 * *  To Compile: mpicc -g -o mahdad_hw5_alg1 mahdad_hw5_alg1.c (to print matrices add -DINTPRINT) * 
 * *  To run: mpiexec -n <process_number> ./mahdad_hw5_alg1 <number_of_bytes>                                                *                                               *
 * ***********************************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>


void myallgather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
               void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);
void printarray(int *a, int n) {
  int i;

  for (i=0; i<n; i++) {
    printf("%d ", a[i]);
    printf("\n");
  }
}
int main(int argc, char **argv)
{
    int         i, rank, size, root=0, offset, maxint,N,byteNum,comm_size;
    int         *sendbuf=NULL, *recvbuf, *finalbuf;
    double      t1, t2;
   
    if (argc != 2) {
      printf("Usage: %s <N>\n", argv[0]);
      exit(-1);
    }

    byteNum = atoi(argv[1]);

    MPI_Init (&argc, &argv);

    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    N=byteNum/4;
    if (rank == root) {
	sendbuf = (int *)malloc(sizeof(int)*N*size);
	for (i=0; i<N*size; i++)
	    sendbuf[i] = i;
    }
    finalbuf = (int *)malloc(sizeof(int)*N*size);
    recvbuf = (int *)malloc(sizeof(int)*N);


   
    MPI_Scatter(sendbuf, N, MPI_INT, recvbuf, N, MPI_INT, root, MPI_COMM_WORLD);


    /* setup a synchronization point */
    MPI_Barrier(MPI_COMM_WORLD);
    t1 = MPI_Wtime();
   // MPI_Allgather(recvbuf,N,MPI_INT,finalbuf,N,MPI_INT,MPI_COMM
    myallgather(recvbuf,N,MPI_INT,finalbuf,N,MPI_INT,MPI_COMM_WORLD);

    /* program end here */
    t2 = MPI_Wtime() - t1;
    MPI_Reduce(&t2, &t1, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
#ifdef INTPRINT
    if (rank == 0) {
      printarray(finalbuf,N*size);
      
     }
#endif
 
    if (rank == 0) {
      // printf("Time taken in process 0 = %g\n", t2);
       printf("Process Number : %d, Byte Number, %d, Maximum time taken among all processes = %g\n",size,byteNum,t1);

      
    }


    MPI_Finalize ();
    free(sendbuf);
    free(recvbuf);
    free(finalbuf);

    return 0;
}


void myallgather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
               void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
    int rank, size, i,j, offset;
    MPI_Status  *status;
    MPI_Request *request;
    MPI_Request *another_request;
    MPI_Status *another_status;
    MPI_Request *bc_request;
    MPI_Status *bc_status;
    MPI_Request *bc_rcv_req;
    MPI_Status *bc_rcv_sts;
    MPI_Aint lb, sizeofsendtype, sizeofrecvtype;

    MPI_Comm_rank (comm, &rank);
    MPI_Comm_size (comm, &size);

   //now implementing Gather

    another_request=malloc(sizeof(MPI_Request)*1);
    another_status=malloc(sizeof(MPI_Status)*1);
    status = malloc(sizeof(MPI_Status)*(size+1));
    request = malloc(sizeof(MPI_Request)*(size+1));
    bc_status = malloc(sizeof(MPI_Status)*(size-1));
    bc_request = malloc(sizeof(MPI_Request)*(size-1));
    bc_rcv_req=malloc(sizeof(MPI_Request)*1);
    bc_rcv_sts=malloc(sizeof(MPI_Status)*1);

    if (rank == 0) { /* If I'm the root node, send it to others */
        MPI_Type_get_extent(sendtype, &lb, &sizeofsendtype);
        MPI_Isend(sendbuf, sizeofsendtype*sendcount, MPI_CHAR, 0, 0, comm, &request[size]);
        for (i = 0; i < size; i++) {
            MPI_Type_get_extent(recvtype, &lb, &sizeofrecvtype);
            offset = sizeofrecvtype*recvcount*i;
            char *bufptr = recvbuf + offset;
            MPI_Irecv(bufptr, sizeofrecvtype*recvcount, MPI_CHAR, i, 0, comm, &request[i]);
           // printf("The i i: %d and the offset is: %d \n:",i,offset);
        }
        MPI_Waitall(size+1, request, status);
    } else { /* all other processes receive from process with rank root */
        MPI_Type_get_extent(sendtype, &lb, &sizeofsendtype);
        MPI_Isend(sendbuf, sizeofsendtype*sendcount, MPI_CHAR, 0, 0, comm, &another_request[0]);
        MPI_Wait(another_request,another_status);
    }

   //now implementing broadcast
   MPI_Barrier(comm);
   if(rank == 0){
     for(j=1;j<size;j++){
        MPI_Isend(recvbuf, sizeofsendtype*sendcount*size, MPI_CHAR, j, 0, comm, &bc_request[j-1]);
     }
     MPI_Waitall(size-1, bc_request, bc_status);
   }else{
      MPI_Irecv(recvbuf, sizeofsendtype*sendcount*size, MPI_CHAR, 0, 0, comm, &bc_rcv_req[0]);
      MPI_Wait(bc_rcv_req,bc_rcv_sts);
   }
    

    free(request);
    free(another_request);
    free(another_status);
    free(status);
    free(bc_request);
    free(bc_status);
    free(bc_rcv_req);

    free(bc_rcv_sts);
}

