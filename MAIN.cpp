#include"INTERRUPTABLEthread.h"
#include"OPTIMYSEDThreadPool.h"
#include<mutex>
#include<iostream>
#include<random>
#include<chrono>
using namespace std;

mutex coutLocker;
void taskFunc(int id, int delay)
{
	for (int i = 0; i < delay; i++) 
	{
		this_thread::sleep_for(chrono::seconds(1));
		if (InterruptableThread::checkInterrupted())
		{
			lock_guard<mutex>l(coutLocker);
			cout << " ÇÀÄÀ×À "<<id<<" ïðåðâàíà " << endl;
			return;
		}

	}
	lock_guard<mutex>l(coutLocker);
	cout << " ÇÀÄÀ×À " << id << " âûïîëíåíà " <<this_thread::get_id()<< endl;
}


int main()
{
	srand(0);
	OPTIMYSEDThreadPool pool;
	pool.start();
	for (int i = 0; i < 20; i++)
	{
		pool.push_task(taskFunc, i, 1 + rand() % 4);
	}
	this_thread::sleep_for(chrono::seconds(2));
	pool.interruptPool();
	return 0;
}