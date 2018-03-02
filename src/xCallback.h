#pragma once
#include "all.h"


class xBuffer;
class xRedisAsyncContext;
class xTcpConnection;
class xConnector;
class xRedisContext;
class xRedisReader;
class xHiredisAsync;
class xTcpClient;
class xSession;
class xItem;

typedef std::shared_ptr<xRedisReader>  RedisReaderPtr;
typedef std::shared_ptr<xRedisContext> RedisContextPtr;
typedef std::shared_ptr<xHiredisAsync> HiredisAsyncPtr;
typedef std::shared_ptr<xRedisAsyncContext> RedisAsyncContextPtr;
typedef std::shared_ptr<xBuffer> xBufferPtr;
typedef std::shared_ptr<xTcpConnection> TcpConnectionPtr;
typedef std::shared_ptr<xConnector> ConnectorPtr;
typedef std::shared_ptr<xTcpClient> TcpClientPtr;
typedef std::shared_ptr<xSession>  SessionPtr;
typedef std::shared_ptr<xItem> ItemPtr;
typedef std::shared_ptr<const xItem> ConstItemPtr;

typedef std::function<void (const std::any &)> xTimerCallback;
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const std::any &)> ConnectionErrorCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;
typedef std::function<void (const TcpConnectionPtr&,xBuffer*)> MessageCallback;








