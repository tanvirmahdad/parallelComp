/****************************************************************************** 

*  Name: Ahmed Tanvir Mahdad                                                  *
*  BlazerID: mahdad                                                           *
*  Course Section: CS 732                                                     *
*  Homework #3                                                                *
*  To Compile: icc -fopenmp -O3 -o hw3_2d hw3_2d.c (to print matrices add -DINTPRINT)         * 
*  To run: ./hw3_2d <problem_size> <maximum_step> <thread_count><thread_count>                              *                                               *
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

double **arrayCopy(double **a,double **b,int N,int pThreads,int qThreads){
  int i,j;
  
#   pragma omp parallel num_threads(pThreads*qThreads)\
     default(none) shared(a,b,N,pThreads,qThreads) private(i,j)
{
  int dummyP,dummyQ,pFactor,qFactor;
  pFactor=N % pThreads;
  if(pFactor !=0){
     dummyP=N+(pThreads-(N % pThreads));
  }
  else{
    dummyP=N;
  }

  qFactor=N % qThreads;
  if(qFactor !=0){
     dummyQ=N+(qThreads-(N % qThreads));
  }
  else{
    dummyQ=N;
  }
  int thread_id=omp_get_thread_num();
  int p= thread_id / qThreads;
  int q= thread_id % qThreads;
  int pthread_range=dummyP / pThreads;
  int iStart=p*pthread_range;
  int iEnd=iStart+pthread_range;
  if(p == pThreads-1){
	iEnd=N;
  }
  for (i=iStart; i<iEnd; i++){
      int qthread_range=dummyQ / qThreads;
      int jStart=q*qthread_range;
      int jEnd=jStart+qthread_range;
      if(q == qThreads-1){
           jEnd=N;
       }
      for (j=jStart; j<jEnd; j++) 
        a[i][j]=b[i][j];
  }
}
  return a;
}



/* Game of Life Function*/
double **gameOfLife(double **a, double **b,int N,int pThreads,int qThreads) 
{
    //flip -- It will track if the next array is different then the present array
    int i, j, k;
    int flip=0;
    int neighbourNumber;
#   pragma omp parallel num_threads(pThreads*qThreads)\
     default(none) shared(a,b,flip,N,comparison_flag,pThreads,qThreads) private(neighbourNumber,i,j)
  {
    int dummyP,dummyQ,pFactor,qFactor;
    pFactor=N % pThreads;
    if(pFactor !=0){
       dummyP=N+(pThreads-(N % pThreads));
    }
    else{
      dummyP=N;
    }

    qFactor=N % qThreads;
    if(qFactor !=0){
       dummyQ=N+(qThreads-(N % qThreads));
    }
    else{
      dummyQ=N;
    }
    int thread_id=omp_get_thread_num();
    int p= thread_id / qThreads;
    int q= thread_id % qThreads;
    int pthread_range=dummyP / pThreads;
    int iStart=p*pthread_range;
    int iEnd=iStart+pthread_range;
    if(p == pThreads-1){
          iEnd=N;
    }
    for (i=iStart; i<iEnd; i++)
    {
      int qthread_range=dummyQ / qThreads;
      int jStart=q*qthread_range;
      int jEnd=jStart+qthread_range;
      if(q == qThreads-1){
           jEnd=N;
       }

      for (j=jStart; j<jEnd; j++)
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
     }
      if(flip==0){
         comparison_flag=1;
    }
       
    return b;
}


int main(int argc, char **argv) 
{
    srand(123456);
    int N, i, j, k,max, comparison,pThreads,qThreads;
    int count=0;
    double **present=NULL, **nextArr=NULL;
    double starttime, endtime;

    if (argc != 5) {
      printf("Usage: %s <N>\n", argv[0]);
      exit(-1);
    }
    
    N = atoi(argv[1]);
    max=atoi(argv[2]);
    pThreads=atoi(argv[3]);
    qThreads=atoi(argv[4]);
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
   // starttime = gettime();
     starttime = omp_get_wtime(); 
   
    //while loop will implement game of life until maximum iteration or present or next state is same
    while(count<max){
      nextArr=gameOfLife(present,nextArr,N+2,pThreads,qThreads);
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
      present=arrayCopy(present,nextArr,N+2,pThreads,qThreads);

      

    }


    //endtime = gettime();
    endtime = omp_get_wtime(); 

    #ifdef TOTALSTEP

    printf("Total Step is: %d\n",count );

    #endif

    //Freeing the memory
     free(present);
     free(nextArr);
     
    printf("Time taken = %lf seconds\n", endtime-starttime);

    return 0;
}
