
#ifndef _CONNECTIONS_H_
#define _CONNECTIONS_H_

#include <list>

class NetIO;

struct SharedMemory
{
	int Terminated;
	int ActiveConnections;
};

class Connection
{
public:
	typedef std::list<int> sockets_collection;
	typedef sockets_collection::iterator sockets_iterator;

protected:
	const NetIO *net_io;

	SharedMemory *shared_memory;

	void RemoveSocket(int conn_fd);
	void IncreaseConnectionsCount(int count);

	bool IsConnectionTerminated() const;

public:
	Connection(const NetIO *io);
	virtual ~Connection();

	virtual void Process() = 0;
	virtual void AddNewSocketDescriptors(sockets_collection *readed) = 0;

	void SetSharedMemory(SharedMemory *memory);
};

#endif /* _CONNECTIONS_H_ */
