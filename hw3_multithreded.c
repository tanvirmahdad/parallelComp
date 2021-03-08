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
#include <omp.h>




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



/* Game of Life Function*/
double **gameOfLife(double **a, double **b,int N,int thread_count) 
{
    //flip -- It will track if the next array is different then the present array
    int i, j, k;
    int flip=0;
    int neighbourNumber;
#   pragma omp parallel num_threads(thread_count)\
     default(none) shared(a,b,flip,N,comparison_flag) private(neighbourNumber,i,j)
    for (i=0; i<N; i++)
    {
#     pragma omp for
      for (j=0; j<N; j++)
      { 
        if(i==0 || j==0 || i==N-1 || j==N-1)
          b[i][j]=0;
        else
        {
          neighbourNumber=a[i-1][j-1]+a[i-1][j]+a[i-1][j+1]+a[i][j-1]+a[i][j+1]+a[i+1][j-1]+a[i+1][j]+a[i+1][j+1];
	#ifdef THREADID
	 printf("i:%d,j:%d,thread_id:%d of %d threads;",i,j,omp_get_thread_num(),omp_get_num_threads());
	printf("\n");
	#endif

          if(a[i][j]==1)
          {
            if(neighbourNumber==2 || neighbourNumber==3)
              b[i][j]=1;
            else{
              b[i][j]=0;
              flip=1;
            }

          }else
            {
            	if(neighbourNumber==3)
            	{
              		b[i][j]=1;
              		flip=1;
            	}
            	else
              		b[i][j]=0;

             }	
           }
          }
         }

      if(flip==0){
         comparison_flag=1;
    }
       
    return b;
}


int main(int argc, char **argv) 
{
    srand(123456);
    int N, i, j, k,max, comparison,thread_count;
    int count=0;
    double **present=NULL, **nextArr=NULL;
    double starttime, endtime;

    if (argc != 4) {
      printf("Usage: %s <N>\n", argv[0]);
      exit(-1);
    }
    
    N = atoi(argv[1]);
    max=atoi(argv[2]);
    thread_count=atoi(argv[3]);
    /* Allocate memory for all three matrices and temporary arrays */
    present = allocarray(N+2, N+2);
    nextArr = allocarray(N+2, N+2);

    
    /* Initialize the matrices */
    srand48(123456);
    present = initarray(present, N+2, N+2, 2);
    nextArr = initarray(nextArr, N+2, N+2, 0);

#ifdef INTPRINT
    printarray(present,N+2,N+2);
    printf("\n");
#endif
   

    //caluclating time on main loop
    starttime = gettime();
   
    //while loop will implement game of life until maximum iteration or present or next state is same
    while(count<max){
      nextArr=gameOfLife(present,nextArr,N+2,thread_count);
      count++;

    #ifdef HALT_CHECKER
      if(comparison_flag==1)
        printf("Game halted at: %d\n",count );
    #endif
      
    #ifdef INTPRINT
      
      printarray(nextArr,N+2,N+2);
      printf("\n");

    #endif
      //If present and next state is same, exit from the while loop
      if(comparison_flag==1)
        break;
      present=arrayCopy(present,nextArr,N+2);
      

    }


    endtime = gettime();

    #ifdef TOTALSTEP

    printf("Total Step is: %d\n",count );

    #endif

    //Freeing the memory
     free(present);
     free(nextArr);
     
    printf("Time taken = %lf seconds\n", endtime-starttime);

    return 0;
}
