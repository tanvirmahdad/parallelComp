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

double **arrayCopy(double **a,double **b,int N){
  int i,j;
  for (i=0; i<N; i++)
      for (j=0; j<N; j++) 
        a[i][j]=b[i][j];

  return a;
}

int isLevelSame(double **a,double **b,int N){
  int i,j;
  int isSame=1;
  for (i=0; i<N; i++)
      for (j=0; j<N; j++)
        if(a[i][j]!=b[i][j]){
            isSame=0;
        }

  return isSame;
}



/* Game of Life Function*/
void **gameOfLife(double **a, double **b,int N,int columnNum,int my_rank,int comm_size) 
{
    //flip -- It will track if the next array is different then the present array
    int i, j, k;
    int prevVal,nextVal;
    int tag1=1;
    int tag2=2;
    int neighbourNumber;
    int prev,next;
    int borderlineUp=0;
    int borderlineDown=0;
    double **prevArray,**nextArray;
    prevArray = allocarray(1, columnNum);
    nextArray = allocarray(1, columnNum);
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
    MPI_Send(&a[0][0],columnNum,MPI_DOUBLE,prev,tag1,MPI_COMM_WORLD);
    MPI_Recv(&prevArray[0][0],columnNum,MPI_DOUBLE,prev,tag2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    MPI_Send(&a[N-1][0],columnNum,MPI_DOUBLE,next,tag2,MPI_COMM_WORLD);
    MPI_Recv(&nextArray[0][0],columnNum,MPI_DOUBLE,next,tag1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
  
   if(prev==MPI_PROC_NULL){
     // prevArray=initarray(prevArray, 1, N+2, 0);
      borderlineUp=1;
     // printf("I am here prev");
   } 
   if(next==MPI_PROC_NULL){
     // nextArray=initarray(nextArray, 1, N+2, 0);
      borderlineDown=1;
     // printf("I am here next");
   }
    for (i=0; i<N; i++){
      for (j=0; j<columnNum; j++){
        if(i==N-1 && borderlineDown==1){
                b[i][j]=0;
        }
        else if(i==0 && borderlineUp==1){
		b[i][j]=0;
	} 
	else if(j==0 || j==columnNum-1){
	     b[i][j]=0;
	   } else {
          if(i==0){
		prevVal=prevArray[0][j-1]+prevArray[0][j]+prevArray[0][j+1];
          }else{
                prevVal=a[i-1][j-1]+a[i-1][j]+a[i-1][j+1];
          }
          if(i==N-1){
                nextVal=nextArray[0][j-1]+nextArray[0][j]+nextArray[0][j+1];
          }
          else{
                nextVal=a[i+1][j-1]+a[i+1][j]+a[i+1][j+1];
          }
                
             neighbourNumber=prevVal+a[i][j-1]+a[i][j+1]+nextVal;
             if(a[i][j]==1){
                if(neighbourNumber==2 || neighbourNumber==3)
         	    b[i][j]=1;
                else{
                    b[i][j]=0;
                    }

              }else{
                if(neighbourNumber==3){
                  b[i][j]=1;
                }else
                  b[i][j]=0;
                }        
          
        }
    }
   }
 
 // prevArray=NULL;
 // nextArray=NULL; 
  free(prevArray);
   free(nextArray);
}


int main(int argc, char **argv) 
{
    srand(123456);
    int N, i, j, k,max, comparison,comm_size,*row_count,*displs,offset,totalrow,m,p,remainder,local_row;
    int my_rank;
    int comp_result,comp_result_unified;
    int count=0;
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

    
    /* Allocate memory for all three matrices and temporary arrays */
    present = allocarray(N+2, N+2);
    nextArr = allocarray(N+2, N+2);
    local_present = allocarray(N+2, N+2);
    local_nextArr = allocarray(N+2, N+2);

    
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
     displs=(int *)malloc(comm_size*sizeof(int));
     for(m=0;m<comm_size;m++){
       row_count[m]=(totalrow / comm_size + ((m<remainder)? 1:0))*(N+2);   
     }

     displs[0]=0;
     for(p=1;p<comm_size;p++){
       displs[p]=displs[p-1]+row_count[p-1];          
     }
     local_row=row_count[my_rank]/(N+2);
    
    //caluclating time on main loop
    MPI_Barrier(MPI_COMM_WORLD);
    starttime = MPI_Wtime();
   
    //while loop will implement game of life until maximum iteration or present or next state is same
    for(i=0;i<max;i++){
      
      MPI_Scatterv(&present[0][0],row_count,displs,MPI_DOUBLE,&local_present[0][0],row_count[my_rank],MPI_DOUBLE,0,MPI_COMM_WORLD);
      MPI_Scatterv(&nextArr[0][0],row_count,displs,MPI_DOUBLE,&local_nextArr[0][0],row_count[my_rank],MPI_DOUBLE,0,MPI_COMM_WORLD);

      gameOfLife(local_present,local_nextArr,local_row,N+2,my_rank,comm_size);
      
      MPI_Gatherv(&local_present[0][0],row_count[my_rank],MPI_DOUBLE,&present[0][0],row_count,displs,MPI_DOUBLE,0,MPI_COMM_WORLD);
      MPI_Gatherv(&local_nextArr[0][0],row_count[my_rank],MPI_DOUBLE,&nextArr[0][0],row_count,displs,MPI_DOUBLE,0,MPI_COMM_WORLD);
      count++;

    #ifdef HALT_CHECKER
      if(my_rank==0){
         comparison_flag=isLevelSame(present,nextArr,N+2);
         if(comparison_flag==1)
             printf("Game halted at: %d\n",count );
      }
    #endif
      
    #ifdef INTPRINT
      if(my_rank==0){
         printarray(nextArr,N+2,N+2);
         printf("\n");
     }

    #endif
      //If present and next state is same, exit from the while loop
      // MPI_Barrier(MPI_COMM_WORLD);
       comp_result=isLevelSame(present,nextArr,N+2);
       MPI_Allreduce(&comp_result, &comp_result_unified, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
      // printf("\n");
      // printf("The comparison Flag is: %d",comp_result_unified);
      // printf("\n");
       if(comp_result_unified==1){
          break;
       }
       if(my_rank==0){
        // printarray(present,N+2,N+2);
	//comparison_flag=isLevelSame(present,nextArr,N+2);
        // printf("\n");
        // printf("The comparison Flag is: %d",comparison_flag);
        // printf("\n");
       // comparison_flag=isLevelSame(present,nextArr,N+2);
       // if(comparison_flag==1)
          // break;
        present=arrayCopy(present,nextArr,N+2);
        }
       

    }
    MPI_Barrier(MPI_COMM_WORLD);
    endtime = MPI_Wtime();
    MPI_Finalize();

    #ifdef TOTALSTEP

    printf("Total Step is: %d\n",count );

    #endif

    //Freeing the memory
     free(present);
     free(nextArr);
     free(row_count);
     free(displs);
     free(local_present);
     free(local_nextArr);
     
     if(my_rank==0){ 
        printf("Time taken = %lf seconds\n", endtime-starttime);
     }
    return 0;
}
