# include <stdlib.h>
# include <stdio.h>
# include <math.h>
# include <time.h>
# include <string.h>
# include <sys/time.h>
# include <stdbool.h>
# include <omp.h>

//#ifndef NTHREADS
//#define NTHREADS 1
//#endif

/* ***************************************************************
Name: FNU SHALINI (Shalini Saini)
BlazerId: sshalini
Course Section: CS 732
Homework#: 3

Compile: gcc 
Run: 
User Input:  N to create (N+2)(N+2) grid, G for Max. Generations

*****************************************************************/


int main ( );
void filename_inc ( char *filename );
int *gol_init ( double prob, int m, int n, int *seed );
void gol_update ( int m, int n, int grid[], int P, int Q );
void gol_write ( char *output_filename, int m, int n, int grid[] );
double rand_01 ( int *seed );
int s_len_trim ( char *s );
bool isPreAndCurrentSame(int m, int n, int grid[], int pre_grid[],int P,int Q );

double gettime(void) {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}


/******************************************************************************/

int main (int argc, char **argv )

{
  char filename[] = "gol_000.txt";
  char str[10000];
  int it;
  int it_max;
  int m;
  int n;
  int *grid;
  int *pre_grid;
  double prob;
  int seed;
  int i;
  int j;
  int thread_count;
  double starttime, endtime;
  int p,q;
  if (argc != 5) 
	{      printf("Usage: %s <N>\n", argv[0]);      
		exit(-1);    
	}   
	n = atoi(argv[1]); //N for grid N+2xN+2
	it_max=atoi(argv[2]); // Max generations
	p=atoi(argv[3]); //thread count
        q=atoi(argv[4]); //thread count

  
  m=n; /* simplifying but can be used separately to define row and column */
  prob = 0.20;
  seed = 123456789;

  starttime = omp_get_wtime();

  //Allocating Grid for N+2 and N+2 to fill boarder cells with dead cells to avoid boundary issues
  grid = ( int * ) malloc ( ( m + 2 ) * ( n + 2 ) * sizeof ( int ) );
  //Another grid to copy and compare change between two consecutive generations
  pre_grid = ( int * ) malloc ( ( m + 2 ) * ( n + 2 ) * sizeof ( int ) );
  
  //Initialze the grid
  for ( it = 0; it <= it_max; it++ )
  {
    if ( it == 0 )
    {
      grid = gol_init ( prob, m, n, &seed );
    }
    else
    {
      gol_update ( m, n, grid,p,q );
    }
	endtime = omp_get_wtime();
    /*****************************************************************
     
    // Print the GOL grid on console and/or write to the file
    // Check if there is any change between two consecutive generations, 
    //if not, stop the GOL and get the number of genrations executed. 
    // If change continues, execute to the max generations defined.
    
    ******************************************************************/
    /* Writing to the file or displaying to the console or both*/
	gol_write ( filename, m, n, grid ); 
    //printf ( "  %s\n", filename );
    //filename_inc ( filename );
	
	/*/Checking if two generations are same or not, 
    if same before max gen #, stop the game and get 
    the number of the generations executed.*/
    bool result = isPreAndCurrentSame(m, n, grid, pre_grid, p,q);
    
    if (result){
      
      printf("Generations executed: %d", it);
      break;
    }
  }
/*
  release grid memory.
*/
  free ( grid );

  
  printf ( "\n" );
  printf ( "Game Ended Normally\n" );
  printf("Time taken = %lf seconds\n", endtime-starttime);
  return 0;
}



/*************************************************
Initialize the grid for GOL size with 0 and 1
0- Dead, 1= Alive
Inner Grid- Random 0 and 1
Outer border ghost cells- all 0s
*************************************************/
int *gol_init ( double prob, int m, int n, int *seed )

{
  int *grid;
  int i;
  int j;
  double r;

  grid = ( int * ) malloc ( ( m + 2 ) * ( n + 2 ) * sizeof ( int ) );
  for ( j = 0; j <= n + 1; j++ )
  {
    for ( i = 0; i <= m + 1; i++ )
    {
      grid[i+j*(m+2)] = 0;
    }
  }

  for ( j = 1; j <= n; j++ )
  {
    for ( i = 1; i <= m; i++ )
    {
      r = rand_01 ( seed );
      if ( r <= prob )
      {
        grid[i+j*(m+2)] = 1;
      }
    }
  }
 
  return grid;

  
}
/***********************************************
Updating Base grid as per Game of Life rules
1. If a cell is “alive” (1) in the current generation, then depending on its neighbor’s state, in
the next generation the cell will either live or die based on the following conditions:
    o Each cell with one or no neighbor dies, as if by loneliness.
    o Each cell with four or more neighbors dies, as if by overpopulation.
    o Each cell with two or three neighbors survives.
2. If a cell is “dead” (0) in the current generation, then if there are exactly three neighbors
"alive" then it will change to the "alive" state in the next generation, as if the
neighboring cells gave birth to a new organism.
***********************************************/

void gol_update ( int m, int n, int grid[],int P,int Q )
{
  int i;
  int j;
  int *s;
 // int thread_count = omp_get_num_threads();
  s = ( int * ) malloc ( m * n * sizeof ( int ) );

#pragma omp parallel default(none) shared(s,grid,m,n,P,Q) private(i,j) num_threads(P*Q)
{ 
 int tid = omp_get_thread_num();
 int p = tid / Q;
 int q = tid % Q;
 int myM = n / P;
 int jstart = (p * myM)+1;
 int jend = jstart + myM;
 if (p == P-1) jend = n;
 for ( j = jstart; j <= jend; j++ )
  {
    int myN = m / Q;
    int istart = (q * myN)+1;
    int iend = istart + myN;
    if (q == Q-1) iend = m;
    for ( i = istart; i <= iend; i++ )
    {
      s[i-1+(j-1)*m] = 
          grid[i-1+(j-1)*(m+2)] + grid[i-1+j*(m+2)] + grid[i-1+(j+1)*(m+2)]
        + grid[i  +(j-1)*(m+2)]                     + grid[i  +(j+1)*(m+2)]
        + grid[i+1+(j-1)*(m+2)] + grid[i+1+j*(m+2)] + grid[i+1+(j+1)*(m+2)];
   
      if ( grid[i+j*(m+2)] == 0 )
      {
        if ( s[i-1+(j-1)*m] == 3 )
        {
          grid[i+j*(m+2)] = 1;
        }
      }
      else if ( grid[i+j*(m+2)] == 1 )
      {
        if ( s[i-1+(j-1)*m] < 2 || 3 < s[i-1+(j-1)*m] )
        {
          grid[i+j*(m+2)] = 0;
        }
      }
    }
	
  }
}
  free ( s );

  return;
}
/***************************************************
Output- writing to console or to a file (each generation)
***************************************************/

void gol_write ( char *output_filename, int m, int n, int grid[])

{
  int i;
  int j;
  //FILE *output_unit;
/* Open the file: No need to open or close file if not writing to the file*/
  //output_unit = fopen ( output_filename, "wt" ); // uncomment if want to write to the file

/*  Write the data.*/
  for ( j = 0; j <= n + 1; j++ )
  {
    for ( i = 0; i <= m + 1; i++ )
    {
        //fprintf ( output_unit, " %d", grid[i+j*(m+2)] );// write to file, uncomment if writing to the file
        //printf (" %d", grid[i+j*(m+2)] ); // to console
    }
        //fprintf ( output_unit, "\n" ); // write to file, uncomment if writing to the file
        //printf ( "\n" ); //to console
  }

  //printf ( "\n" );
/*  Close the file.*/
    //fclose ( output_unit ); /* uncomment if want to write to the file*/


  return;
}

  bool isPreAndCurrentSame(int m, int n, int grid[], int pre_grid[], int P,int Q)
  {
  bool result = true;
  int j;
  int i;

  if (pre_grid == NULL) {
    /*This is first one so we just need to move the current array data 
    to pre_grid and return false as it is not same*/

     result = false;
    }
  else {
#pragma omp parallel default(none) shared(grid,m,n, pre_grid,P,Q,result) private(i,j) num_threads(P*Q)
{
  int tid = omp_get_thread_num();
  int p = tid / Q;
  int q = tid % Q;
  int myM = n / P;
  int jstart = (p * myM)+1;
  int jend = jstart + myM;
  if (p == P-1) jend = n;  
  for ( j = jstart; j <= jend; j++ )
    {
      int myN = m / Q;
      int istart = (q * myN)+1;
      int iend = istart + myN;
      if (q == Q-1) iend = m;
      
      for ( i = istart; i <= iend; i++ )
      {
        if (pre_grid[i+j*(m+2)] != grid[i+j*(m+2)]){
          result = false;
          break;
        }
      }
    }
  }
}
  /* If there is a change, keep updating the second 
  grid with current generation data*/
  if (!result) {
  /*move the current array data to pre_grid */
#pragma omp parallel default(none) shared(grid,m,n, pre_grid,P,Q,result) private(i,j) num_threads(P*Q)
{
  int tid = omp_get_thread_num();
  int p = tid / Q;
  int q = tid % Q;
  int myM = n / P;
  int jstart = (p * myM)+1;
  int jend = jstart + myM;
  if (p == P-1) jend = n;  
    for ( j = jstart; j <= jend; j++ )
    {
      int myN = m / Q;
      int istart = (q * myN)+1;
      int iend = istart + myN;
      if (q == Q-1) iend = m;
      for ( i = istart; i <= iend; i++ )
      {
        pre_grid[i+j*(m+2)] = grid[i+j*(m+2)];
      }
    }
   }
  }
  return result;
}

/*********************************************
Random generation  to get grid data as 0 and 1
**********************************************/
double rand_01 ( int *seed )

{
  int i4_huge = 2147483647;
  int k;
  double r;

  k = *seed / 127773;

  *seed = 16807 * ( *seed - k * 127773 ) - k * 2836;

  if ( *seed < 0 )
  {
    *seed = *seed + i4_huge;
  }

  r = ( ( double ) ( *seed ) ) * 4.656612875E-10;

  return r;
}


/*******************************************
Generating filenames to store each generation
*******************************************/

void filename_inc ( char *filename )

{
  int change;
  int n;
  char *t;

  n = s_len_trim ( filename );

  if ( n <= 0 )
  {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "FILENAME_INC - Fatal error!\n" );
    fprintf ( stderr, "  The input string is empty.\n" );
    exit ( 1 );
  }

  change = 0;

  t = filename + n - 1;
  
  while ( 0 < n )
  {
    if ( '0' <= *t && *t <= '9' )
    {
      change = change + 1;

      if ( *t == '9' )
      {
        *t = '0';
      }
      else
      {
        *t = *t + 1;
        return;
      }
    }
    t--;
    n--;
  }
/* if no change */
  if ( change == 0 )
  {
    n = s_len_trim ( filename );
    t = filename + n - 1;
    while ( 0 < n )
    {
      *t = ' ';
      t--;
      n--;
    }
  }

  return;
}
/**************************************************
Returns the length of a string to the last nonblank.
***************************************************/

int s_len_trim ( char *s )

{
  int n;
  char *t;

  n = strlen ( s );
  t = s + strlen ( s ) - 1;

  while ( 0 < n ) 
  {
    if ( *t != ' ' )
    {
      return n;
    }
    t--;
    n--;
  }

  return n;
}
/******************************************************************************/

