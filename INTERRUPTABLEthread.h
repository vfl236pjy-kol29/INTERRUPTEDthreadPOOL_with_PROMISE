#pragma once
#include<queue>
#include<thread>
#include<condition_variable>
#include<vector>
#include<functional>
#include<mutex>
using namespace std;

typedef function<void()>task_type;
typedef void(*FuncType)(int, int);
thread_local bool thread_interrupt_flag = false;
class OPTIMYSEDThreadPool;

class InterruptableThread
{
public:
	InterruptableThread(void(*tf)()) :m_pFlag(NULL), m_thread(tf) {}
	InterruptableThread(OPTIMYSEDThreadPool * pool, int qindex) {}
	~InterruptableThread() {}
	void interrupt();
	void startFunc1(void(*tf)());
	void startFunc2(OPTIMYSEDThreadPool*, int);
	bool checkInterrupted();
	thread m_thread;

private:
	mutex m_defender;
	bool* m_pFlag;

};


