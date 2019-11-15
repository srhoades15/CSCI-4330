/*
	Shawn Rhoades
	Parallel Processing - CSCI 4330
	Project 2
	Due 9/30/2019
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>


typedef struct {
	int startRow;
	int endRow;
	int numCols;
	float maxDiff;
}threadVar;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

//function prototypes
void* iterateArrays(void *);
void initializeTables(int, int, int, int, int, int);
float setAverage(int, int);
void initializeVarArray(int,int,int);
double timer();
//global variables
float **A;
float **B;
threadVar **varArray;
pthread_barrier_t barr;
int done = 0;
int counter = 0;


int main(int argc,char *argv[])
{
	
	alarm(360);
	//test to make sure the correct number of command line arguments were entered
	if (argc != 9)
	{
		printf("Invalid # of command line args entered\n");
		return 0;
	}
	//Values entered by user onto command line
	int numRows = atoi(argv[1]);
	int numCols = atoi(argv[2]);
	int top_temp = atoi(argv[3]);
	int left_temp = atoi(argv[4]);
	int right_temp = atoi(argv[5]);
	int bottom_temp = atoi(argv[6]);
	float epsilon = atof(argv[7]);
	int numThreads = atoi(argv[8]);
	pthread_t tid[numThreads];
	//set int values to track iteration values
	int iterations = 0;
	int powerOfTwo = 0;
	//float value to track the overall maxDifference after each Iteration
	float trackMaxDiff = 0.0;
	//pointer to float array will be used in swapping B to A after each iteration
	float **tempArr;
	//get our initial time value
	double t1 = timer();	
	//generate our array of float values
	initializeTables(numRows, numCols,top_temp,left_temp,right_temp,bottom_temp);
	initializeVarArray(numRows,numCols,numThreads);
	pthread_barrier_init(&barr, NULL, numThreads);
	//initialize our pthreads
	for (int i = 1; i < numThreads; i++)
	{
		pthread_create(&tid[i],NULL,iterateArrays,(void*)(long int)i);
	}
	//start While loop that iterates from the main thread
	//until the done marker is activated, loop will continue which also controls other threads
	while(done == 0) 
	{
		//Thread will go index by index for it's assigned rows, storing the average of all neighbors into 
		//the B table element
	        for (int i = varArray[0]->startRow; i <= varArray[0]->endRow; i++)
		{
			for (int j = 1; j < varArray[0]->numCols; j++)
			{
				B[i][j] = (A[i-1][j] + A[i+1][j] + A[i][j-1] + A[i][j+1]) / 4.0;
				//fine the difference between A and New B value
				//if greater than maxDiff, update maxDiff
				if (fabs(A[i][j] - B[i][j]) > varArray[0]->maxDiff)
					varArray[0]->maxDiff = fabs(A[i][j] - B[i][j]);
			}
		}
		//have the main thread wait until threads say they have updated the
		//counter, if all other threads are done, proceed
		pthread_mutex_lock(&mutex);
		while (counter < numThreads-1)
		{
			pthread_cond_wait(&cond, &mutex);
		}
		pthread_mutex_unlock(&mutex);
		//all threads except main are in the barrier
		//get the max difference from all threads
		for (int n = 0; n < numThreads; n++)
		{
			if (varArray[n]->maxDiff > trackMaxDiff)
				trackMaxDiff = varArray[n]->maxDiff;
			varArray[n]->maxDiff = 0.0;
		}
		//compare trackMaxDiff to Epsilon, if Equal or less, mark as done
		if (trackMaxDiff < epsilon)
			done = 1;
		//output the iteration number if the count is a power of 2, or 
		//the maxDiff is <= epsilon
		if (iterations == pow(2, powerOfTwo)  || done == 1)
		{
			printf("%5d\t%8f\n", iterations, trackMaxDiff);
			powerOfTwo++;
		}
		//increment # of iterations
		iterations++;
		//swap our Arrays, moving updated data in B to A
		tempArr = B;
		B = A;
		A = tempArr;
		//reset our tracked max difference
		trackMaxDiff = 0.0;
		//reset the variable used to track # of threads in barrier
		counter = 0;
		//have main process enter barrier with waiting threads, 
		//move to next iteration
		pthread_barrier_wait(&barr);	
	}
	//once all work is done, output the time spent
	double t2 = timer();
	printf("\nTOTAL TIME: %f\n", t2-t1);
	return 0;
}

void* iterateArrays(void *arg)
{
	long int rank = (long int)arg;
	//threads will loop through their iterations, updating B table
	while(  done == 0 )
	{
                for (int i = varArray[rank]->startRow; i <= varArray[rank]->endRow; i++)
                {
                         for (int j = 1; j < varArray[rank]->numCols; j++)
                         {
                                 B[i][j] = (A[i-1][j] + A[i+1][j] + A[i][j-1] + A[i][j+1]) / 4.0;
				 //find the difference between A and New B value
                                 //if greater than maxDiff, update maxDiff
                                 if (fabs(A[i][j] - B[i][j]) > varArray[rank]->maxDiff)
                                         varArray[rank]->maxDiff = fabs(A[i][j] - B[i][j]);
 	                 }
                }
		//mark your work as done and signal the main thread to check the count variable 
		pthread_mutex_lock(&mutex);
		counter++;
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
		//enter the barrier and wait until released by mainthread
		pthread_barrier_wait(&barr);
	}
}





void initializeTables(int numRows,int numCols,int top_temp,int left_temp,int right_temp,int bottom_temp)
{
	float ave = 0.0;
	//initialize memory for Array A
	A = malloc(numRows * sizeof(float *));
	float *j = malloc(numRows * numCols * sizeof(float));
	//Repeat for Array B
	B = malloc(numRows * sizeof(float *));
	float *k = malloc(numRows * numCols * sizeof(float));

	//loop through each array to set memory locations
	for (int i = 0; i < numRows; i++)
	{
		A[i] = j + (i * numCols);
		B[i] = k + (i * numCols);
	}

	//set the top and bottom row values
	for (int l = 0; l < numCols; l++)
	{
		A[0][l] = top_temp;
		B[0][l] = top_temp;
		A[numRows -1][l] = bottom_temp;
		B[numRows -1][l] = bottom_temp;
	}
	//left column
	for (int t = 0; t < numRows - 1; t++)
	{
		A[t][0] = left_temp;
		B[t][0] = left_temp;
		A[t][numCols - 1] = right_temp;
		B[t][numCols - 1] = right_temp;
	}
	//get ave of edges, store into the inside elements
	ave = setAverage(numRows, numCols);
	for (int n = 1; n < numRows -1; n++)
	{
		for (int p = 1; p < numCols - 1; p++)
		{
			A[n][p] = ave;
			B[n][p] = 0.0;
		}
	}	
}

float setAverage(int numRows,int numCols)
{
	float edgeSum;
	int numEdges;
	for (int i = 0; i < numCols; i++)
	{
		edgeSum += (A[0][i] + A[numRows - 1][i]);
	}
	for (int j = 1; j < (numRows - 1);j++)
	{
		edgeSum += A[j][0] + A[j][numCols - 1];
	}
	numEdges = (2 * numRows) + (2 * (numCols-2));
	return edgeSum / numEdges;
}

void initializeVarArray(int numRows,int numCols,int numThreads)
{
	int threadRows, modVal, startRow = 1;
	varArray = malloc(numThreads * sizeof(threadVar *));
	threadRows = (numRows - 2) / numThreads;
	modVal = (numRows - 2) % numThreads;
	
	//Loop through array of structs and set values
	for (int i = 0; i < numThreads; i++)
	{
		varArray[i] = malloc(sizeof(threadVar));
		varArray[i]->startRow = startRow;
		if (modVal > 0)
		{
		//add the number of threadRows to struct indicating how many
		//rows they will need to process. Since each # of rows will likely
		//not divide by the number of threads evenly, we will need to add 
		//remainder to be worked by other threads	
			varArray[i]->endRow = (varArray[i]->startRow + threadRows);
			modVal = modVal - 1;
		}
		else
		{
			varArray[i]->endRow = (varArray[i]->startRow + threadRows - 1);
		}
		//set all other items in struct
		varArray[i]->numCols = numCols - 1;
		varArray[i]->maxDiff = 0.0;
		//finally, update the start row of next struct to the end row + 1 
		//of current struct
		startRow = varArray[i]->endRow + 1;
	}
}

double timer()
{
	struct timeval tv;
	
	gettimeofday(&tv, (struct timezone *) 0);
	return( (double) (tv.tv_sec + (tv.tv_usec / 1000000.0)) );
}		
