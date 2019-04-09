#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>

// Required libraries for Linux and Windows
#ifdef UNIX
	#include <unistd.h>
	#include <pthread.h>
#endif

#ifdef WINDOWS
	#include <windows.h>
#endif

#define MAXGRIDSIZE 	10
#define MAXTHREADS	1000
#define NO_SWAPS	20

extern int errno;

typedef enum {GRID, ROW, CELL, NONE} grain_type;
int gridsize = 0;
int grid[MAXGRIDSIZE][MAXGRIDSIZE];
int threads_left = 0;

/*
Need to add -DUNIX or -DWINDOWS in make file in order for the gridapp.c file to compile correctly
DEADLOCK PREVENTION SCHEME: preventing circular wait by always trying to acquire lowest numbered resource first (similar as in dining philosopher's problem)
*/

time_t start_t, end_t;

// Lock declarations for the different locking granularities for Linux, Windows and for the threads_left shared state
#ifdef UNIX
	pthread_mutex_t linuxGridLock;
	pthread_mutex_t linuxRowLock[MAXGRIDSIZE];
	pthread_mutex_t linuxCellLock[MAXGRIDSIZE][MAXGRIDSIZE];
        pthread_mutex_t linuxThreadsLock;
#endif

#ifdef WINDOWS
	HANDLE windowsGridLock;
	HANDLE windowsRowLock[MAXGRIDSIZE];
	HANDLE windowsCellLock[MAXGRIDSIZE][MAXGRIDSIZE];
        HANDLE windowsThreadsLock;
#endif

int PrintGrid(int grid[MAXGRIDSIZE][MAXGRIDSIZE], int gridsize)
{
	int i;
	int j;

	for (i = 0; i < gridsize; i++)
	{
		for (j = 0; j < gridsize; j++)
			fprintf(stdout, "%d\t", grid[i][j]);
		fprintf(stdout, "\n");
	}
	return 0;
}


long InitGrid(int grid[MAXGRIDSIZE][MAXGRIDSIZE], int gridsize)
{
	int i;
	int j;
	long sum = 0;
	int temp = 0;

	srand( (unsigned int)time( NULL ) );


	for (i = 0; i < gridsize; i++)
		for (j = 0; j < gridsize; j++) {
			temp = rand() % 100;
			grid[i][j] = temp;
			sum = sum + temp;
		}

	return sum;

}

/* 
Helper function that initializes the Linux, Windows and threads locks.
This could have been done more efficiently by passing in the locking granularity
as an argument and initializing the required locks only. I tried implementing it
this way but I ran into issues so I reverted back to simply initalizing all the locks
no matter what locking scheme is specified by the user. Also, there is probably a
more efficient way to initialize the cell level locks than by using nested for loops.
*/

void InitLocks(int gridsize)
{
	#ifdef UNIX
		pthread_mutex_init(&linuxGridLock, NULL);
                pthread_mutex_init(&linuxThreadsLock, NULL);
		for(int i = 0; i < gridsize; i++)
		{
			pthread_mutex_init(&linuxRowLock[i], NULL);
			for(int j = 0; j < gridsize; j++)
			{
				pthread_mutex_init(&linuxCellLock[i][j], NULL);
			}
		}
	#endif

	#ifdef WINDOWS
		windowsGridLock = CreateMutex(NULL, FALSE, NULL);
                windowsThreadsLock = CreateMutex(NULL, FALSE, NULL);
		for(int i = 0; i < gridsize; i++)
		{
			windowsRowLock[i] = CreateMutex(NULL, FALSE, NULL);
			for(int j = 0; j < gridsize; j++)
			{
				windowsCellLock[i][j] = CreateMutex(NULL, FALSE, NULL);
			}
		}
	#endif
}

long SumGrid(int grid[MAXGRIDSIZE][MAXGRIDSIZE], int gridsize)
{
	int i;
	int j;
	long sum = 0;


	for (i = 0; i < gridsize; i++){
		for (j = 0; j < gridsize; j++) {
			sum = sum + grid[i][j];
		}
	}
	return sum;

}

void* do_swaps(void* args)
{

	int i, row1, column1, row2, column2;
	int temp;
	grain_type* gran_type = (grain_type*)args;
        
        // Acquire the lock to increment the threads_left count  
        #ifdef UNIX
       	  pthread_mutex_lock(&linuxThreadsLock);
        #endif

        #ifdef WINDOWS
          WaitForSingleObject(windowsThreadsLock);
        #endif

        threads_left++;

        // Release the threads_left lock
        #ifdef UNIX
          pthread_mutex_unlock(&linuxThreadsLock);
        #endif

        #ifdef WINDOWS
          ReleaseMutex(windowsThreadsLock);
        #endif

	for(i=0; i<NO_SWAPS; i++)
	{
		row1 = rand() % gridsize;
		column1 = rand() % gridsize;
		row2 = rand() % gridsize;
		column2 = rand() % gridsize;

		// If the two cells to be swapped (randomly generated) are the same, nothing needs to be done, skip to next swap
		if(row1 == row2 && column1 == column2)
		{
			continue;
		}

		if (*gran_type == ROW)
		{
		  // Acquire row level locks
			#ifdef UNIX
				// If two randomly generated cells are in the same row, only one lock is needed
				if(row1 == row2)
				{
					pthread_mutex_lock(&linuxRowLock[row1]);
				}
				else
				{
                                        // Lower-numbered resource always first to avoid circular wait 
					if(row1 < row2)
					{
						pthread_mutex_lock(&linuxRowLock[row1]);
						pthread_mutex_lock(&linuxRowLock[row2]);
					}
					else
					{
						pthread_mutex_lock(&linuxRowLock[row2]);
						pthread_mutex_lock(&linuxRowLock[row1]);
					}
				}
			#endif

			#ifdef WINDOWS
			// If two randomly generated cells are in the same row, only one lock is needed
			if(row1 == row2)
			{
				WaitForSingleObject(windowsRowLock[row1]);
			}
			else
			{
                                // Lower-numbered resource first to avoid circular wait
				if(row1 < row2)
				{
					WaitForSingleObject(windowsRowLock[row1]);
					WaitForSingleObject(windowsRowLock[row2]);
				}
				else
				{
					WaitForSingleObject(windowsRowLock[row2]);
					WaitForSingleObject(windowsRowLock[row1]);
				}
			}
			#endif			
		}
		else if (*gran_type == CELL)
		{
		  // Acquire cell level locks
			#ifdef UNIX
                                // Lower-numbered resource first to avoid circular wait
				if(row1 == row2)
				{
					if(column1 <= column2)
					{
						pthread_mutex_lock(&linuxCellLock[row1][column1]);
						pthread_mutex_lock(&linuxCellLock[row2][column2]);
					}
					else
					{
						pthread_mutex_lock(&linuxCellLock[row2][column2]);
						pthread_mutex_lock(&linuxCellLock[row1][column1]);
					}
				}
				else if(row1 < row2)
				{
					pthread_mutex_lock(&linuxCellLock[row1][column1]);
					pthread_mutex_lock(&linuxCellLock[row2][column2]);
				}
				else
				{
					pthread_mutex_lock(&linuxCellLock[row2][column2]);
					pthread_mutex_lock(&linuxCellLock[row1][column1]);
				}
			#endif

			#ifdef WINDOWS
                                // Lower-numbered resource first to avoid circular wait
				if(row1 == row2)
				{
					if(column1 <= column2)
					{
						WaitForSingleObject(windowsCellLock[row1][column1]);
						WaitForSingleObject(windowsCellLock[row2][column2]);
					}
					else
					{
						WaitForSingleObject(windowsCellLock[row2][column2]);
						WaitForSingleObject(windowsCellLock[row1][column1]);
					}
				}
				else if(row1 < row2)
				{
					WaitForSingleObject(windowsCellLock[row1][column1]);
					WaitForSingleObject(windowsCellLock[row2][column2]);
				}
				else
				{
					WaitForSingleObject(windowsCellLock[row2][column2]);
					WaitForSingleObject(windowsCellLock[row1][column1]);
				}
			#endif			
		}
		else if (*gran_type == GRID)
		{
		  // Acquire grid level lock
			#ifdef UNIX
				pthread_mutex_lock(&linuxGridLock);
			#endif

			#ifdef WINDOWS
				WaitForSingleObject(windowsGridLock);
			#endif
		}


		temp = grid[row1][column1];
                
		#ifdef UNIX
			sleep(1);
		#endif

		#ifdef Windows
			Sleep(1000);
		#endif
                
		grid[row1][column1]=grid[row2][column2];
		grid[row2][column2]=temp;


		if (*gran_type == ROW)
		{
		  // Release row level locks
			#ifdef UNIX
				// If two randomly generated cells are in the same row, only one lock is needed
				if(row1 == row2)
				{
					pthread_mutex_unlock(&linuxRowLock[row1]);
				}
				else
				{
					pthread_mutex_unlock(&linuxRowLock[row1]);
					pthread_mutex_unlock(&linuxRowLock[row2]);
				}
			#endif

			#ifdef WINDOWS
			// If two randomly generated cells are in the same row, only one lock is needed
			if(row1 == row2)
			{
				ReleaseMutex(windowsRowLock[row1]);
			}
			else
			{
				ReleaseMutex(windowsRowLock[row1]);
				ReleaseMutex(windowsRowLock[row2]);
			}
			#endif
		}
		else if (*gran_type == CELL)
		{
		  // Release cell level locks
			#ifdef UNIX
				pthread_mutex_unlock(&linuxCellLock[row1][column1]);
				pthread_mutex_unlock(&linuxCellLock[row2][column2]);
			#endif

			#ifdef WINDOWS
				ReleaseMutex(windowsCellLock[row1][column1]);
				ReleaseMutex(windowsCellLock[row2][column2]);
			#endif
		}
		else if (*gran_type == GRID)
		{
		  // Release grid level lock
			#ifdef UNIX
				pthread_mutex_unlock(&linuxGridLock);
			#endif

			#ifdef WINDOWS
				ReleaseMutex(windowsGridLock);
			#endif
		}


	}

	/* Does this need protection? ---> YES, it is a shared state */

        #ifdef UNIX
          pthread_mutex_lock(&linuxThreadsLock);
        #endif

        #ifdef WINDOWS
          WaitForSingleObject(windowsThreadsLock);
        #endif
  
        threads_left--;   

        #ifdef UNIX
          pthread_mutex_unlock(&linuxThreadsLock);
        #endif

        #ifdef WINDOWS
          ReleaseMutex(windowsThreadsLock);
        #endif

	if (threads_left == 0){  /* if this is last thread to finish*/
	  time(&end_t);         /* record the end time*/
	}
	return NULL;
}




int main(int argc, char **argv)
{


	int nthreads = 0;
	pthread_t threads[MAXTHREADS];
	grain_type rowGranularity = NONE;
	long initSum = 0, finalSum = 0;
	int i;


	if (argc > 3)
	{
		gridsize = atoi(argv[1]);
		if (gridsize > MAXGRIDSIZE || gridsize < 1)
		{
			printf("Grid size must be between 1 and 10.\n");
			return(1);
		}
		nthreads = atoi(argv[2]);
		if (nthreads < 1 || nthreads > MAXTHREADS)
		{
			printf("Number of threads must be between 1 and 1000.");
			return(1);
		}

		if (argv[3][1] == 'r' || argv[3][1] == 'R')
			rowGranularity = ROW;
		if (argv[3][1] == 'c' || argv[3][1] == 'C')
			rowGranularity = CELL;
		if (argv[3][1] == 'g' || argv[3][1] == 'G')
		  rowGranularity = GRID;

	}
	else
	{
		printf("Format:  gridapp gridSize numThreads -cell\n");
		printf("         gridapp gridSize numThreads -row\n");
		printf("         gridapp gridSize numThreads -grid\n");
		printf("         gridapp gridSize numThreads -none\n");
		return(1);
	}

	printf("Initial Grid:\n\n");
	initSum =  InitGrid(grid, gridsize);
	// Initialize the Linux and Windows locks by calling helper function
	InitLocks(gridsize);
	PrintGrid(grid, gridsize);
	printf("\nInitial Sum:  %li\n", initSum);
	printf("Executing threads...\n");

	/* better to seed the random number generator outside
	   of do swaps or all threads will start with same
	   choice
	*/
	srand((unsigned int)time( NULL ) );

	time(&start_t);
	for (i = 0; i < nthreads; i++)
	{
		if (pthread_create(&(threads[i]), NULL, do_swaps, (void *)(&rowGranularity)) != 0)
		{
			perror("thread creation failed:");
			exit(-1);
		}
	}


	for (i = 0; i < nthreads; i++)
		pthread_detach(threads[i]);


	while (1)
	{
		sleep(2);
		if (threads_left == 0)
		  {
		    fprintf(stdout, "\nFinal Grid:\n\n");
		    PrintGrid(grid, gridsize);
		    finalSum = SumGrid(grid, gridsize);
		    fprintf(stdout, "\n\nFinal Sum:  %li\n", finalSum);
		    if (initSum != finalSum){
		      fprintf(stdout,"DATA INTEGRITY VIOLATION!!!!!\n");
		    } else {
		      fprintf(stdout,"DATA INTEGRITY MAINTAINED!!!!!\n");
		    }
		    fprintf(stdout, "Secs elapsed:  %g\n", difftime(end_t, start_t));

		    exit(0);
		  }
	}


	return(0);

}
