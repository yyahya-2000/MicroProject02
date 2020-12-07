#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h> //if you have Windows
//#include <unistd.h> //if you have LINUX/UNIX, and replace Sleep() by sleep()
#include <string>

int smokingTimes;
std::string* components = new std::string[4]{ "tobacco","paper","matches" };

pthread_cond_t component_cond;

sem_t  componentsCompleted;
sem_t  componentsUncompleted;

pthread_mutex_t mutexD;
pthread_mutex_t mutexF;

int thirdComponentNum;// processor number, which has the third component

void* Smoker(void* param) {
	int sNum = *((int*)param);//processor number
	while (1)
		//for (int i = 0; i < smokingTimes; i++)
	{
		// processors haven't the third component, will wait 
		while (sNum != thirdComponentNum) {
			pthread_cond_wait(&component_cond, &mutexD);
		}
		pthread_mutex_lock(&mutexD);
		sem_wait(&componentsCompleted);
		printf("Time %d: Smoker is rolling the cigarette using %d processor\n", int(clock()), sNum);
		//Sleep(500);// 0.5 second forrolling the cigarette
		printf("Time %d: Smoker is smoking using %d processor\n", int(clock()), sNum);
		//Sleep(1000);// 1 second for smoking (*_*)
		printf("Time %d: Smoker finish smoking using %d processor\n\n", int(clock()), sNum);
		//if (i != smokingTimes - 1)
		sem_post(&componentsUncompleted);
		pthread_mutex_unlock(&mutexD);
		Sleep(6);//important, it let's Mediator func complete editing thirdComponentNum
	}
	return nullptr;
}

void* Mediator(void* param) {
	for (int i = 0; i <= smokingTimes; i++)
	{
		/*
		* here we don't need mutex, cause only one processor(Mediator) will implement this method
		*/
		// pthread_mutex_lock(&mutexF);
		sem_wait(&componentsUncompleted);
		if (i == smokingTimes)
			return nullptr;
		printf("Time %d: Mediator is going to put two different random components on the table\n", int(clock()));
		/*
		* instead of making two random choices for these two components,
		* I only took one random value (which is the component number not on the table)
		*/
		thirdComponentNum = rand() % 3 + 1;

		/* look for the two components which should be on the table*/
		std::string onTheTable = "";
		for (int i = 1; i < 4; i++)
		{
			if (i == thirdComponentNum)
				continue;
			onTheTable += *(components + i - 1) + " ";
		}
		printf("Time %d: Mediator put (%s) on the table\n\n", int(clock()), onTheTable.c_str());
		sem_post(&componentsCompleted);
		pthread_cond_signal(&component_cond);
		//pthread_mutex_unlock(&mutexF);		
	}
	return nullptr;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Sorry, incorrect input!\n");
		printf("Please make sure the entry is only one arg, which is a positive integer!");
		exit(1);
	}

	if (!(smokingTimes = std::stoi(argv[1])) || smokingTimes <= 0) {
		printf("Sorry, incorrect input!\nPlease make sure the entry is a positive integer!");
		exit(1);
	}

	srand(time(NULL));
	pthread_mutex_init(&mutexD, nullptr);
	pthread_mutex_init(&mutexF, nullptr);
	sem_init(&componentsCompleted, 0, 0);
	sem_init(&componentsUncompleted, 0, 1);

	/*
	* run three threads for smoker
	*/
	pthread_t threadS[3];
	int smokers[3];
	for (int i = 0; i < 3; i++) {
		smokers[i] = i + 1;
		pthread_create(&threadS[i], nullptr, Smoker, (void*)(smokers + i));
	}
	//current thread run Mediator func
	Mediator(nullptr);
	//canceling all threads when smoker somkes "smokingTimes" times
	for (int i = 0; i < 3; i++)
		pthread_cancel(threadS[i]);
	return 0;
}