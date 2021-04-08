/****************************************************************************** 

*  Name: Ahmed Tanvir Mahdad                                                  *
*  BlazerID: mahdad                                                           *
*  Course Section: CS 732                                                     *
*  Homework #1                                                                *
*  To Compile: gcc -O mahdad_hw1.c (to print matrices add -DINTPRINT)         * 
*  To run: ./a.out <problem_size> <maximum_step>                              *                                               *
******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>



//comparison_flag will track if the present state of game is same as next state. 
//Default value is 0, will set to 1 when match found
int comparison_flag=0;

double gettime(void) {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

double **allocarray(int P, int Q) {
  int i;
  double *p, **a;
  
  p = (double *)malloc(P*Q*sizeof(double));
  a = (double **)malloc(P*sizeof(double*));

  if (p == NULL || a == NULL) 
    printf("Error allocating memory\n");

  /* for row major storage */
  for (i = 0; i < P; i++)
    a[i] = &p[i*Q];
  
  return a;
}

double **initarray(double **a, int mrows, int ncols, int value) {
  int i,j;



  for (i=0; i<mrows; i++)
    for (j=0; j<ncols; j++)
      if(i==0 || j==0 || i==mrows-1 || j==ncols-1)
        a[i][j]=0;
      else
        a[i][j] = (int) (drand48()*value);

       
      //a[i][j] = value;
  
  return a;
}

void printarray(double **a, int mrows, int ncols) {
  int i,j;
  
  for (i=0; i<mrows; i++) {
    for (j=0; j<ncols; j++)
      printf("%f ", a[i][j]);
    printf("\n");
  }
}

double **arrayCopy(double **a,double **b,int local_row,int N,int my_rank){
  int i,j;
  
  for (i=1; i<=local_row; i++){
      for (j=0; j<N; j++){
        a[i][j]=b[i-1][j];
       }
     }
   
 // if(my_rank==0){
   // printarray(a,local_row+2,N);
   // printf("\n");
 // }
  return a;
}

int isLevelSame(double **a,double **b,int local_row,int N){
  int i,j;
  int isSame=1;
  for (i=1; i<local_row+1; i++)
      for (j=0; j<N; j++)
        if(a[i][j]!=b[i-1][j]){
            isSame=0;
        }

  return isSame;
}


/* Game of Life Function*/
int gameOfLife(double **a, double **b,int N,int columnNum,int my_rank,int comm_size) 
{
    //flip -- It will track if the next array is different then the present array
    int i, j, k;
    int neighbourNumber;
    int flip=0;
   

    
    
    for (i=1; i<N+1; i++){
      for (j=0; j<columnNum; j++){
        if(j==0 || j==columnNum-1 || (i==1 && my_rank==0)|| (i==N && my_rank==comm_size-1)){
              b[i-1][j]=0;
        }else{
               neighbourNumber=a[i-1][j-1]+a[i-1][j]+a[i-1][j+1]+a[i][j-1]+a[i][j+1]+a[i+1][j-1]+a[i+1][j]+a[i+1][j+1];
               
               
                 if(a[i][j]==1){

                if(neighbourNumber==2 || neighbourNumber==3)
                    b[i-1][j]=1;
                else{
                    flip=1;
                    b[i-1][j]=0;
                    }

              }else{
                if(neighbourNumber==3){
                  flip=1;
                  b[i-1][j]=1;
                }else
                  b[i-1][j]=0;
                }
             }
                  
        }
      //here the end
    }
    return flip;
   }
 



int main(int argc, char **argv) 
{
    srand(123456);
    int N, i, j, k,max,local_row_end, comparison,comm_size,*row_count,*row_tracker,*displs,offset,totalrow,m,p,remainder,local_row;
    int my_rank;
    int prev,next;
    int tag1=1;
    int tag2=2;
    int comp_result,comp_result_unified;
    int count=0;
    int flip;
    double **present=NULL, **nextArr=NULL, **local_present=NULL, **local_nextArr=NULL;
    double starttime, endtime;
    //MPI initialization


    if (argc != 3) {
      printf("Usage: %s <N>\n", argv[0]);
      exit(-1);
    }
    
    N = atoi(argv[1]);
    max=atoi(argv[2]);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Request request[4];
    
    /* Allocate memory for all three matrices and temporary arrays */
    present = allocarray(N+2, N+2);
    nextArr = allocarray(N+2, N+2);
    local_present = allocarray(N+4, N+2);
    local_nextArr = allocarray(N+4, N+2);

    
    /* Initialize the matrices */
    srand48(123456);
    present = initarray(present, N+2, N+2, 2);
    nextArr = initarray(nextArr, N+2, N+2, 0);


    //printarray(present,N+2,N+2);
    //printf("\n");
#ifdef INTPRINT
    if(my_rank==0){
      printarray(present,N+2,N+2);
      printf("\n");
     }
#endif
   
     //calculating some counts needed for mpi
     totalrow=N+2;
     remainder=totalrow % comm_size;
     row_count=(int *)malloc(comm_size*sizeof(int));
     row_tracker=(int *)malloc(comm_size*sizeof(int));
     displs=(int *)malloc(comm_size*sizeof(int));
     for(m=0;m<comm_size;m++){
       row_count[m]=(totalrow / comm_size + ((m<remainder)? 1:0))*(N+2);
       row_tracker[m]=totalrow / comm_size + ((m<remainder)? 1:0);   
     }

     displs[0]=0;
     for(p=1;p<comm_size;p++){
       displs[p]=displs[p-1]+row_count[p-1];          
     }
     local_row=row_count[my_rank]/(N+2);
     local_row_end=row_tracker[my_rank];

     if(my_rank==0){
        prev=MPI_PROC_NULL;
        }else{
        prev=my_rank-1;
      }
    if(my_rank==(comm_size-1)){
        next=MPI_PROC_NULL;
        }else{
        next=my_rank+1;
      }

    
    //caluclating time on main loop
   // MPI_Barrier(MPI_COMM_WORLD);
   // starttime = MPI_Wtime();
    MPI_Scatterv(&present[0][0],row_count,displs,MPI_DOUBLE,&local_present[1][0],row_count[my_rank],MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Scatterv(&nextArr[0][0],row_count,displs,MPI_DOUBLE,&local_nextArr[0][0],row_count[my_rank],MPI_DOUBLE,0,MPI_COMM_WORLD);   
    starttime = MPI_Wtime();

 //while loop will implement game of life until maximum iteration or present or next state is same
    for(i=0;i<max;i++){
      /*
      if(my_rank==0){
        printf("Before Sendrecv at rank 0");
        printf("\n");
        printarray(local_present,local_row+2,N+2);
cal_present&local_present&local_present&local_present
        printf("\n");
      }    
      */
     /*
      if(my_rank==1){
        printf("Before Sendrecv at rank 1");
        printf("\n");
        printarray(local_present,local_row+2,N+2);
      */
      MPI_Sendrecv(&local_present[local_row_end][0],N+2,MPI_DOUBLE,next,tag2,&local_present[0][0],N+2,MPI_DOUBLE,prev,tag2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
      MPI_Sendrecv(&local_present[1][0],N+2,MPI_DOUBLE,prev,tag1,&local_present[local_row_end+1][0],N+2,MPI_DOUBLE,next,tag1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);     
      
       /*
       if(my_rank==0){
        printf("After Sendrecv at rank 1");
        printf("\n");
        printarray(local_present,local_row+2,N+2);
        printf("\n");
      }
       */    
      /*
      if(my_rank==1){
        printf("After Sendrecv at rank 1");
        printf("\n");
        printarray(local_present,local_row+2,N+2);
        printf("\n");
      }
       */

      flip=gameOfLife(local_present,local_nextArr,local_row,N+2,my_rank,comm_size);
       
     
     // MPI_Barrier(MPI_COMM_WORLD);
    #ifdef HALT_CHECKER
      if(my_rank==1){
         comparison_flag=isLevelSame(present,nextArr,N+2);
         if(comparison_flag==1)
             printf("Game halted at: %d\n",count );
      }
    #endif
      
    #ifdef INTPRINT
      if(my_rank==1){
         printarray(local_nextArr,local_row,N+2);
         printf("\n");
     }

    #endif
      //If present and next state is same, exit from the while loop
      // MPI_Barrier(MPI_COMM_WORLD);
     //  comp_result=isLevelSame(local_present,local_nextArr,local_row,N+2);
       MPI_Allreduce(&flip, &comp_result_unified, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
      // printf("\n");
      // printf("The comparison Flag is: %d",comp_result_unified);
      // printf("\n");
       if(comp_result_unified==0){
          break;
       }
       
        // printarray(present,N+2,N+2);
	//comparison_flag=isLevelSame(present,nextArr,N+2);
        // printf("\n");
        // printf("The comparison Flag is: %d",comparison_flag);
        // printf("\n");
       // comparison_flag=isLevelSame(present,nextArr,N+2);
       // if(comparison_flag==1)
          // break;
       local_present=arrayCopy(local_present,local_nextArr,local_row,N+2,my_rank);
     /*  
      if(my_rank==1){
         printarray(local_present,local_row+2,N+2);
         printf("\n");
         printarray(local_nextArr,local_row,N+2);
         printf("\n");
     } 
    */

    }

   // MPI_Gatherv(&local_present[1][0],row_count[my_rank],MPI_DOUBLE,&present[0][0],row_count,displs,MPI_DOUBLE,0,MPI_COMM_WORLD);
   // MPI_Barrier(MPI_COMM_WORLD);
    endtime = MPI_Wtime();
    MPI_Finalize();

    #ifdef TOTALSTEP

    printf("Total Step is: %d\n",count );

    #endif

    //Freeing the memory
     free(present);
     free(nextArr);
     free(row_count);
     free(row_tracker);
     free(displs);
     free(local_present);
     free(local_nextArr);
     
     if(my_rank==0){ 
        printf("Time taken = %lf seconds\n", endtime-starttime);
     }
    return 0;
}
