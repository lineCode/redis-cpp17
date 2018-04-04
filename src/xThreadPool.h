#pragma once
#include "all.h"
#include "xCallback.h"

class xThread;
class xEventLoop;

class xThreadPool : noncopyable
{
public:
	typedef std::function<void(xEventLoop*)> ThreadInitCallback;
	xThreadPool(xEventLoop *baseLoop);
	~xThreadPool();

	void setThreadNum(int numThreads) { this->numThreads = numThreads; }
	void start(const ThreadInitCallback &cb = ThreadInitCallback());

	xEventLoop *getNextLoop();
	xEventLoop *getLoopForHash(size_t hashCode);
	std::vector<xEventLoop*> getAllLoops();
	bool getStarted() const { return started; }

private:
	xEventLoop *baseLoop;
	bool started;
	int	numThreads;
	int	next;

	std::vector<ThreadPtr> threads;
	std::vector<xEventLoop*> loops;


};
