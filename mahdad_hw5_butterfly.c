/****************************************************************************** *******************
 *  *
 *   * *  Name: Ahmed Tanvir Mahdad                                                                    *
 *    * *  BlazerID: mahdad                                                                             *
 *     * *  Course Section: CS 732                                                                       *
 *      * *  Homework #5 (Allgather,(Gather+broadcast version))                                           *
 *       * *  To Compile: mpicc -g -o mahdad_hw5_butterfly mahdad_hw5_butterfly.c (to print matrices add -DINTPRINT) * 
 *        * *  To run: mpiexec -n <process_number> ./mahdad_hw5_butterfly <number_of_bytes>                                                *                                               *
 *         * ***********************************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>
#include <string.h>


void myallgather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
               void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);
void tester();
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
   // It is my implemented myallgather which is a allgather fucntion implemented by butterfly distribution algorithm
    myallgather(recvbuf,N,MPI_INT,finalbuf,N,MPI_INT,MPI_COMM_WORLD);
  //  tester();
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
       printf("Maximum time taken among all processes = %g\n", t1);
      // printarray(finalbuf,N*size);
      
    }


    MPI_Finalize ();

    free(sendbuf);
    free(recvbuf);
    free(finalbuf);

    return 0;
}

void tester(){

     int i,j,starting_point,limit;
     int processnumber=16;
     int iterator,pair,pairfactor;
     for(i=1;i<=processnumber/2;i=i*2){
         iterator=0;
         while(iterator<processnumber){
            limit=iterator+i;
            starting_point=iterator;
            
            while(iterator<limit){
               pair=iterator+i;
               printf("%d is sending to %d at starting point %d and reverse starting point: %d\n",iterator,pair,starting_point,limit);
               iterator=iterator+1;
            }
           // printf("Out of inner loop \n");
            iterator=iterator+i;
         }
         printf("\n\n");
     }

}



void myallgather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
               void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
   


    int rank, size,offset,rightOffset,leftOffset,starting_point; 
    MPI_Aint lb, sizeofsendtype, sizeofrecvtype;

    MPI_Comm_rank (comm, &rank);
    MPI_Comm_size (comm, &size);
    
    int i,j,limit;
     int iterator,pair;
   
     
     for(j=0;j<size;j++){
       MPI_Type_get_extent(sendtype, &lb, &sizeofsendtype);
       MPI_Type_get_extent(recvtype, &lb, &sizeofrecvtype);
        if(rank==j){
         // printf("I am here \n");
          offset=sizeofrecvtype*recvcount*j;
          char *sendptr=recvbuf+offset;
          MPI_Sendrecv(sendbuf,sizeofsendtype*sendcount,MPI_CHAR,j,0,sendptr,sizeofrecvtype*recvcount,MPI_CHAR,j,0,comm,MPI_STATUS_IGNORE);
        }
     }
    
     
     for(i=1;i<=size/2;i=i*2){
         iterator=0;
         while(iterator<size){
            limit=iterator+i;
            starting_point=iterator;
            while(iterator<limit){
               pair=iterator+i;
               MPI_Type_get_extent(recvtype, &lb, &sizeofrecvtype);
               MPI_Type_get_extent(sendtype, &lb, &sizeofsendtype);
              // printf("%d is sending to %d\n",iterator,pair);
              rightOffset = sizeofrecvtype*recvcount*starting_point;
              char *rightbufptr = recvbuf + rightOffset;
              leftOffset = sizeofrecvtype*recvcount*limit;
              char *leftbufptr = recvbuf + leftOffset;
              if(rank==iterator){
                MPI_Sendrecv(rightbufptr,sizeofsendtype*sendcount*i,MPI_CHAR,pair,0,leftbufptr,sizeofrecvtype*recvcount*i,MPI_CHAR,pair,0,comm,MPI_STATUS_IGNORE);
              }
              if(rank==pair){
                MPI_Sendrecv(leftbufptr,sizeofsendtype*sendcount*i,MPI_CHAR,iterator,0,rightbufptr,sizeofrecvtype*recvcount*i,MPI_CHAR,iterator,0,comm,MPI_STATUS_IGNORE);
              }
               iterator=iterator+1;
            }
            iterator=iterator+i;
         }
     }
   
}

