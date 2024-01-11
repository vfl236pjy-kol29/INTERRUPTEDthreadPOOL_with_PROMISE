#include <iostream>
#include<queue>
#include<thread>
#include<condition_variable>
#include<vector>
#include<mutex>
#include<functional>
#include"INTERRUPTABLEthread.h"

thread_local bool thread_interrupt_flag = false;
using namespace std;
mutex m_defender;

InterruptableThread::InterruptableThread(void(*taskFunc)()) :m_pFlag(nullptr), m_thread(&InterruptableThread::startFunc1, this, taskFunc) {}
InterruptableThread(OPTIMYSEDThreadPool * pool, int qindex) : m_pFlag(nullptr), m_thread(&InterruptableThread::startFunc2, this, pool, qindex) {}
InterruptableThread::~InterruptableThread() { m_thread.join(); }

void InterruptableThread::startFunc2(OPTIMYSEDThreadPool* pool, int qindex)
{
	{
		lock_guard<mutex>l(m_defender);
		m_pFlag = &thread_interrupt_flag;
	}
	pool->threadFunc(qindex);
	{
		lock_guard<mutex>l(m_defender);
		m_pFlag = nullptr;
	}
}

void InterruptableThread::startFunc1(void(*taskFunc)())
{
	{
		lock_guard<mutex>l(m_defender);
		m_pFlag = &thread_interrupt_flag;
	}
	taskFunc();
	{
		lock_guard<mutex>l(m_defender);
		m_pFlag = nullptr;
	}
}

void InterruptableThread::interrupt()
{
	lock_guard<mutex>l(m_defender);
	if (m_pFlag)//защита от просака
		*m_pFlag = true;
}

bool InterruptableThread::checkInterrupted()
{
	return thread_interrupt_flag;
}

