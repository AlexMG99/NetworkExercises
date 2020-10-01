#include <stdio.h>
#include <stdlib.h>

//Thread header
#include <thread>
//Thread protector
#include <mutex>

long long int globalVar = 0;
std::mutex mtx;

// Main thread function
void Thread()
{
	for (long long int i = 0; i < 100000; ++i)
	{
		//Protect global variable
		std::unique_lock<std::mutex> lock(mtx);
		++globalVar;
	}
}

int main() 
{
	std::thread t[2] = { std::thread(Thread), std::thread(Thread) };

	//Sincronization point
	t[0].join();
	t[1].join();

	printf("Global var: %lld\n", globalVar);

	system("pause");
	return 0;
}