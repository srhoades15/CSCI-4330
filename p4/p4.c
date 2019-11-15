/*  
	Parallel Processing - Project 4
	Shawn Rhoades
	Due: 11/4/2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mpi.h"

#define MAXHOSTS   128				/*set the max capacity of hosts */
#define HOSTLEN 1028					/* set max capacity of host name Length buff */

int  main(int argc, char *argv[])
{

	int numRanks = atoi(argv[2]);
	int myRank;
	int *rankBuff, *sender;
	long long int *hostId, *hostBuff;
	MPI_Status status;
	char myHost[HOSTLEN];
	char  hostNameList[MAXHOSTS][HOSTLEN];

	//generate MPI World
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	
	alarm(180);
	//all processes will store their rank number, hostname and hostId
	sender = malloc(sizeof(int));
	sender[0] = myRank;
	hostId = malloc(sizeof(long long int));
	hostId[0] = gethostid();
	

	gethostname(myHost, HOSTLEN);
	//Rank0 will not gather all of the info from the other ranks and accumulate our final arrays for printing
	if (myRank == 0)
	{
		//allocate memory for our int and long long int arrays
		rankBuff = (int *)malloc(numRanks * sizeof(int));
		hostBuff = (long long int *)malloc(numRanks * sizeof(long long int));
	}
          //Rank0 will not gather all of the info from the other ranks and accumulate our final arra     ys for printing  
          MPI_Gather(sender, 1, MPI_INT, rankBuff, 1, MPI_INT, 0, MPI_COMM_WORLD);        
	 MPI_Gather(hostId, 1, MPI_LONG_LONG_INT, hostBuff, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD)     ;
	 MPI_Gather(myHost, HOSTLEN, MPI_CHAR, hostNameList[myRank], HOSTLEN,MPI_CHAR, 0, MPI_COMM_WORLD);
	if( myRank == 0)
	{
		//now print
		for (int i = 0; i < numRanks; i++)
			printf("%4d\t%s\t%llx\n", rankBuff[i], hostNameList[i] , hostBuff[i]);
	
	}
	
	MPI_Finalize();

}
