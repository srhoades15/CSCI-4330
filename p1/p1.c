/*
	Shawn Rhoades
	Parallel Processing - CSCI 4330
	Project 1
	Due 9/16/2019 
*/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>

//Function ProtoType
float** generateArray(int, int);




int main(int argc, char *argv[])
{
	alarm(360);

	//Values entered by user on Command Line will be stored into usable variables
	int numRows = atoi(argv[1]);				//number of rows in table
	int numCols = atoi(argv[2]);				//number of columns in table
	int top_temp = atoi(argv[3]);				//temperature to initialize top row
	int left_temp = atoi(argv[4]);				//temperature to initialize far left columns
	int right_temp = atoi(argv[5]);				//temperature to initialize far right columns
	int bottom_temp = atoi(argv[6]);			//temperature to initialize bottom row
	float epsilon = atof(argv[7]);				//Epsilon represents the max difference we are seeking between iterations
								//Epsilon is our sentinal value, that will stop iterations when reached
	float initialAve = 0.0;				//Initial ave and sum are used to sum and average the border elements

	float initialSum = 0.0;				//then initialize the inside of the table using the initialAve

	//initialize two arrays
	float **A = generateArray(numRows, numCols);
	float **B = generateArray(numRows, numCols);

	//initialize temps for top row
	for (int i = 0; i < numCols; i++)
	{
		A[0][i] = top_temp;
		B[0][i] = top_temp;
	}
	//initialize temps for left column
	for (int j = 0; j < numRows; j++)
	{
		A[j][0] = left_temp;
		B[j][0] = left_temp;
	}
	//initialize temps for right column
	for (int k = 0; k < numRows; k++)
	{
		A[k][numCols-1] = right_temp;
		B[k][numCols-1] = right_temp;
	}
	//initialize temps for bottom row
	for (int l = 0; l < numCols; l++)
	{
		A[numRows-1][l] = bottom_temp;
		B[numRows-1][l] = bottom_temp;
	}
/*
	//find the average of all of the external indices
	for (int m = 0; m < numCols; m++)
	{
		initialSum = initialSum + A[0][m];
		initialSum = initialSum + A[numRows-1][m];
	}
	for (int n = 1; n < numRows - 1; n++)
	{
		initialSum = initialSum + A[n][0];
		initialSum = initialSum + A[n][numCols-1];
	}*/
	int edgeSum = 0;
	for (int m = 0; m < numCols; m++)
	{
		initialSum += A[0][m] + A[numRows-1][m];
		edgeSum += 2;
	}
	for (int i = 1; i < numRows-1; i++)
	{
		initialSum += A[i][0] + A[i][numCols-1];
		edgeSum += 2;
	}
	initialAve = initialSum / edgeSum;	
	//initialAve = initialSum / ((2 * numRows) + (2 * (numCols - 2))); 

	//set all of the internal points on grid to the Initial average
	for (int i = 1; i < numRows - 1; i++)
	{
		for (int j = 1; j < numCols - 1; j++)
		{
			A[i][j] = initialAve;
			B[i][j] = 0;
		}
	}
	


	int powerOfTwo = 0;   		//variable to track if the loop count is on a value that is a power of 2
	int loopCount = 0;		//variable to track the number of iterations completed
	int done = 0;			//bool value will be triggered when the difference of a value is below epsilon
	double maxDiff = 0.0;		//maxDiff tracks the ongoing difference between indices between iterations
	float **tempArr;		//tempArr serves to move updated table B to A after each iteration, allows table A to always be
					//calculated into table B
	while (done == 0)
	{
		//in while loop, iteration through each table, updating the value of index B to the average of it's N, W, S and E neighbors
		for(int i = 1; i < numRows - 1; i++)
		{
			for (int j = 1; j < numCols - 1; j++)
			{
				B[i][j] = (A[i-1][j] + A[i][j+1] + A[i+1][j] + A[i][j-1]) / 4.0;
				//find the highest difference between indices in table A and B
				if (fabs(A[i][j] - B[i][j]) > maxDiff)
					maxDiff = fabs(A[i][j] - B[i][j]);
			}
		}
		//if the max difference is now equal to or less than epsilon, stop looping and exit
		if (maxDiff <= epsilon)
			done = 1;
		//output the iteration number if the iteration count is a power of 2, or the maxDiff is <= epsilon
		if (loopCount == pow(2, powerOfTwo) || done == 1)
		{
			printf("Iteration %5d: %8f\n", loopCount, maxDiff);
			//iterate powerOfTwo;
			powerOfTwo++;
		}
		//use tempArr to move the newly calculated table, B, into table A for next iteration
		tempArr = B;
		B = A;
		A = tempArr;
		//reset our maxDiff variable
		maxDiff = 0.0;
		//iterate LoopCount and PowerofTwo;
		loopCount++;
	}	
		
		
	return 0;
}


//generateArray function allows us to input a number of rows and columns and returns a pointer to 
//a 2D array of float variables
float** generateArray(int num_rows, int num_cols)
{
	float **arr, *p;
	
	arr = malloc(num_rows * sizeof(float **));
	p = malloc(num_rows * num_cols * sizeof(float));

	for (int i = 0; i < num_rows; i++)
	{
		arr[i] = p + (i * num_cols);
	}

	return arr;
}
