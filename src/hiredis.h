//
// Created by zhanghao on 2018/6/17.
//
#pragma once
#include "sds.h"
#include "tcpconnection.h"
#include "object.h"
#include "tcpclient.h"
#include "socket.h"
#include "log.h"
#include "threadpool.h"

/* This is the reply object returned by redisCommand() */

class RedisAsyncContext;
struct RedisReply
{
	RedisReply()
	:str(nullptr)
	{
	
	}

	~RedisReply()
	{
		if (str != nullptr)
		{
			zfree(str);
		}
	}

    int32_t type;	/* REDIS_REPLY_* */
    int64_t integer;	/* The integer when type is REDIS_REPLY_INTEGER */
    int32_t len;	 /* Length of string */
    size_t elements;	/* Used for both REDIS_REPLY_ERROR and REDIS_REPLY_STRING */
    char *str;	 /* Used for both REDIS_REPLY_ERROR and REDIS_REPLY_STRING */
    std::vector<RedisReplyPtr> element;	/* elements vector for REDIS_REPLY_ARRAY */
};

struct RedisReadTask
{
    int32_t type;	
    int32_t elements;
    int32_t idx;
    std::any privdata;
    RedisReplyWeakPtr obj;
    struct RedisReadTask *parent;
};

RedisReplyPtr createReplyObject(int32_t type);
RedisReplyPtr createString(const RedisReadTask *task,const char *str,size_t len);
RedisReplyPtr createArray(const RedisReadTask *task,int32_t elements);
RedisReplyPtr createInteger(const RedisReadTask *task,int64_t value);
RedisReplyPtr createNil(const RedisReadTask *task);

/* Default set of functions to build the reply. Keep in mind that such a
 * function returning NULL is interpreted as OOM. */

struct RedisFunc
{
	RedisFunc()
	{
		createStringFuc = createString;
		createArrayFuc = createArray;
		createIntegerFuc = createInteger;
		createNilFuc = createNil;
	}
	
	std::function<RedisReplyPtr(const RedisReadTask*,const char*,size_t)> createStringFuc;
	std::function<RedisReplyPtr(const RedisReadTask*,int32_t)> createArrayFuc;
	std::function<RedisReplyPtr(const RedisReadTask*,int64_t)> createIntegerFuc;
	std::function<RedisReplyPtr(const RedisReadTask*)> createNilFuc;
};

class RedisReader
{
public:
	RedisReader();
	RedisReader(Buffer *buffer);

	int32_t redisReaderGetReply(RedisReplyPtr &reply);
	void redisReaderSetError(int32_t type,const char *str);
	void redisReaderSetErrorProtocolByte(char byte);
	void redisReaderSetErrorOOM();
	void moveToNextTask();
	int32_t processLineItem();
	int32_t processBulkItem();
	int32_t processMultiBulkItem();
	int32_t processItem();

	int64_t readLongLong(const char *s);
	const char *readBytes(uint32_t bytes);
	const char *readLine(int32_t *_len);

private:
	RedisReader(const EventLoop&);
	void operator=(const RedisReader&);

public:
	RedisReadTask rstack[9];
	char errstr[128];
	int32_t ridx;
	int32_t err;
	size_t pos;
	RedisFunc fn;
	BufferPtr buffer;
	RedisReplyPtr reply;
	std::any privdata;
};

typedef std::function<void(const RedisAsyncContextPtr &context,
		const RedisReplyPtr &,const std::any &)> RedisCallbackFn;
struct RedisCallback
{
	RedisCallbackFn fn;
    std::any privdata;
};

struct RedisAsyncCallback
{
	RedisAsyncCallback()
	:data(nullptr),len(0) { }
	RedisCallback cb;
	int32_t len;
	char *data;
};

class RedisContext
{
public:
	RedisContext();
	RedisContext(Buffer *buffer,int32_t sockfd);
	~RedisContext();

	int32_t redisvAppendCommand(const char *format,va_list ap);
	void redisAppendCommand(const char *cmd,size_t len);
	RedisReplyPtr redisCommand(const char *format,...);
	RedisReplyPtr redisvCommand(const char *format,va_list ap);
	RedisReplyPtr redisCommandArgv(int32_t argc,const char **argv,const size_t *argvlen);
	void redisAppendFormattedCommand(const char *cmd,size_t len);
	int32_t redisAppendCommandArgv(int32_t argc,const char **argv,const size_t *argvlen);
	void redisSetError(int32_t type,const char *str);
	RedisReplyPtr redisBlockForReply();
	int32_t redisContextWaitReady(int32_t msec);
	int32_t redisCheckSocketError();
	int32_t redisBufferRead();
	int32_t redisBufferWrite(int32_t *done);
	int32_t redisGetReply(RedisReplyPtr &reply);
	int32_t redisGetReplyFromReader(RedisReplyPtr &reply);
	int32_t redisContextConnectTcp(const char *ip,int16_t port,const struct timeval *timeout);
	int32_t redisAppendCommand(const char *format, ...);
	int32_t redisContextConnectUnix(const char *path,const struct timeval *timeout);

private:
	RedisContext(const RedisContext&);
	void operator=(const RedisContext&);

public:
	void clear();
	void setBlock();
	void setConnected();
	void setDisConnected();

	char errstr[128];	/* String representation of error when applicable */
	const char *ip;
	int16_t port;
	const char *path;
	int32_t err;	/* Error flags, 0 when there is no error */
	int32_t fd;
	int8_t flags;

	Buffer sender;  /* Write buffer */
	RedisReaderPtr reader;	/* Protocol reader */
};

class RedisAsyncContext
{
public:
	typedef std::list<RedisAsyncCallback> RedisAsyncCallbackList;
	RedisAsyncContext(Buffer *buffer,const TcpConnectionPtr &conn);
	~RedisAsyncContext();

	void  __redisAsyncCommand(const RedisCallbackFn &fn,
			const std::any &privdata,char *cmd,size_t len);
	int redisvAsyncCommand(const RedisCallbackFn &fn,
			const std::any &privdata,const char *format,va_list ap);
	int redisAsyncCommand(const RedisCallbackFn &fn,
			const std::any &privdata,const char *format, ...);

	int32_t redisGetReply(RedisReplyPtr reply);
	RedisContextPtr getRedisContext() { return contextPtr; }
	TcpConnectionPtr getServerConn() { return serverConn; }
	std::mutex &getMutex() { return mtx;}
	RedisAsyncCallbackList &getCb() { return asyncCb; }

private:
	RedisAsyncContext(const RedisAsyncContext&);
	void operator=(const RedisAsyncContext&);

	int32_t err;
	char *errstr;
	std::any data;
	RedisContextPtr contextPtr;
	TcpConnectionPtr serverConn;
	RedisAsyncCallbackList asyncCb;
	std::mutex mtx;
};

class Hiredis
{
public:
	Hiredis(EventLoop *loop,bool clusterMode = false);
	~Hiredis();

	void clusterAskConnCallBack(const TcpConnectionPtr &conn);
	void clusterMoveConnCallBack(const TcpConnectionPtr &conn);
	void redisReadCallBack(const TcpConnectionPtr &conn,Buffer *buffer);
	void redisConnCallBack(const TcpConnectionPtr &conn);

	void eraseRedisMap(int32_t sockfd);
	void insertRedisMap(int32_t sockfd,const RedisAsyncContextPtr &context);

	void pushTcpClient(const TcpClientPtr &client);
	void clearTcpClient();

	void start() { pool.start(); }
	void setThreadNum(int16_t threadNum) { pool.setThreadNum(threadNum); }

	auto &getPool() { return pool; }
	auto &getMutex() { return rtx; }
	auto &getAsyncContext() { return redisAsyncContexts; }
	auto &getTcpClient() { return tcpClients; }

	RedisAsyncContextPtr getIteratorNode();

private:
	Hiredis(const Hiredis&);
	void operator=(const Hiredis&);

	typedef std::unordered_map<int32_t,RedisAsyncContextPtr> RedisAsyncContextMap;
	ThreadPool pool;
	std::vector<TcpClientPtr> tcpClients;
	RedisAsyncContextMap redisAsyncContexts;
	bool clusterMode;
	std::mutex rtx;
	RedisAsyncContextMap::iterator node;
};


int redisFormatSdsCommandArgv(sds *target,int argc,const char **argv,const size_t *argvlen);
int32_t redisFormatCommand(char **target,const char *format,...);
int32_t redisFormatCommandArgv(char **target,int32_t argc,
			const char **argv,const size_t *argvlen);
int32_t redisvFormatCommand(char **target,const char *format,va_list ap);

RedisContextPtr redisConnectWithTimeout(const char *ip,
		int16_t port,const struct timeval tv);
RedisContextPtr redisConnect(const char *ip,int16_t port);
RedisContextPtr redisConnectUnix(const char *path);


