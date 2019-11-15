#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<sys/time.h>
#include <unistd.h>



double evalfunc(int nodeid, int *expandid1, int *expandid2);


struct node { 
	int id;
	struct node *next;
};

//pthread mutex's to control locking global variables
pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
//create an array of linked lists to hold our nodes
struct node **threadArray;
//variables to hold the RC and ID of solutions
int solutionCount = 0;
int nodesSearch = 0;
int done  = 0;
int threadsDone = 0;
//int threads
//function prototypes
void setThreadArray(int);
void * searchParty(void *);
void initThreadsWorked(int);
//array to track number of nodes visited by each thread
int *threadsWorked;

int main(int argc, char *argv[])
{
	alarm(360);

	int expandId1, expandId2;
	int targetSolution = atoi(argv[2]);
	int numThreads = atoi(argv[1]);
//	int solutionCount = 0;
//	int threadsSearched = 0;
	int threadsUsed = 0;
	int startIndex;
	double returnCode;
	struct node *currentNode, *toFree, *tailNode, *expandNode1, *expandNode2;
	pthread_t tid[numThreads];
	//initialize the array of Linked Lists to be utilized by our threads
	setThreadArray(numThreads);
	initThreadsWorked(numThreads);		
	//initial thread is going to work through the search space, only until we are at a depth low enough to split work up to all of the threads
	//
	startIndex =  numThreads - 1;
	//once we hit the targeted start index, we will start storing into the array for threads to work
	currentNode = (struct node *) malloc(sizeof(struct node));
	currentNode->id = 0;
	currentNode->next = NULL;
	tailNode = currentNode;
	//if the node we want to start all of our threads on is > 0, then we will have the main thread first scan through the initial nodes until we start up the pthreads
	
	while (currentNode->id  < startIndex  )
	{	
		
		returnCode = evalfunc(currentNode->id, &expandId1, &expandId2);
		threadsWorked[0]++;
		if (returnCode > 1.0)
		{
			//solution found
			printf("Solution->Node: %d   RC: %f\n", currentNode->id, returnCode);
			solutionCount++;
		}	
		if (expandId1 >= 0)
		{
			expandNode1 = (struct node*) malloc(sizeof(struct node));
			expandNode1->id = expandId1;
			expandNode1->next = NULL;
			tailNode->next = expandNode1;
			tailNode = expandNode1;
		}
		else
		{
			expandNode1 = NULL;
		}
		if (expandId2 >= 0)
		{
			expandNode2 = (struct node *) malloc(sizeof(struct node));
			expandNode2->id = expandId2;
			expandNode2->next = NULL;
			tailNode->next = expandNode2;
			tailNode = expandNode2;
		}
		else
		{
			expandNode2 = NULL;
		}
		toFree = currentNode;
		currentNode = currentNode->next;
		
		free(toFree);
	}
	
	//main process has completed running until there is enough work for all threads, now spool up the threads
	for( int i = 0; i < numThreads; i++)
	{
		threadArray[i] ->id = startIndex + i;
		pthread_create(&tid[i], NULL, searchParty, (void *)(long int)i);
	}

	//main thread will now monitor to see progress of our threads
	pthread_mutex_lock(&mutex);
	while(threadsDone < numThreads && solutionCount < targetSolution)
	{
		pthread_cond_wait(&cond, &mutex);			
	}
	for (int p = 0; p < numThreads; p++)
	{
			nodesSearch = nodesSearch + threadsWorked[p];
	}
	printf("Nodes Searched: %d\n", nodesSearch);

	return 0;
} 


void *searchParty(void *arg)
{
	struct node *currentNode, *toFree, *tailNode, *expandNode1, *expandNode2;
	int expandId1, expandId2;
	long int rank = (long int)arg;
	double returnCode;

	//initialize a currentNode
	currentNode =(struct node *)malloc(sizeof(struct node));
	currentNode->id = 0;
	currentNode->next = NULL;
	currentNode = threadArray[rank];
	//set the tailing node to currentNode
	tailNode = currentNode;
	while(done == 0)
	{
		while(currentNode)
		{
			returnCode = evalfunc(currentNode->id, &expandId1, &expandId2);
			//build in mutex to lock threadsWorked to update
			if (returnCode > 1.0) //solution found
			{
					pthread_mutex_lock(&mutex);
					printf("Solution->Node: %d   RC: %f\n", currentNode->id, returnCode);
					solutionCount++;	
					pthread_cond_signal(&cond);
					pthread_mutex_unlock(&mutex);	
			}
			if (expandId1 >= 0)
			{
				//new node will be created for expandId1 as expansion, added to elements list
				expandNode1 = (struct node *) malloc(sizeof(struct node));
				expandNode1->id = expandId1;
				expandNode1->next = NULL;
				tailNode->next = expandNode1;
				tailNode = expandNode1;
			}
			else
			{
				//expandId1 was a value of -1, indicating no node to continue searching
				expandNode1 = NULL;
			}
			if (expandId2 >= 0)
			{
				//new node will be created for expandId2 as expansion, added to elements list
				expandNode2 = (struct node *)malloc(sizeof(struct node));
				expandNode2->id = expandId2;
				expandNode2 -> next = NULL;
				tailNode->next = expandNode2;
				tailNode = expandNode2;
			}
			else
			{
				expandNode2 = NULL;
			}
			toFree 	= currentNode;
			//move to our next node in list
		        currentNode = currentNode->next;
			free(toFree);
			//after testing, build else statement that will ask helper function to get work
	//		free(toFree);
			 threadsWorked[rank]++;
		}
		pthread_mutex_lock(&mutex);
		threadsDone++;
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
	}
	
}



void setThreadArray(int numThreads)
{
	//malloc space for threadarray
	threadArray =  malloc(numThreads * sizeof(struct node **));
	//initialize a headNode for each array item
	for (int i = 0; i < numThreads; i++)
	{
		threadArray[i]  = malloc(sizeof(struct node));
		threadArray[i]->id = i;
		threadArray[i]->next = NULL;
	}
}			


void initThreadsWorked(int numThreads)
{
	threadsWorked = malloc(numThreads * sizeof(int));
}
	

