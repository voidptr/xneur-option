
#include "connection.h"
#include "net_io.h"

Connection::Connection(const NetIO *io) :
	net_io(io)
{}

Connection::~Connection()
{}

void Connection::SetSharedMemory(SharedMemory *memory)
{
	shared_memory = memory;
}

void Connection::RemoveSocket(int conn_fd)
{
	net_io->Close(conn_fd);
	shared_memory->ActiveConnections--;
}

void Connection::IncreaseConnectionsCount(int count)
{
	shared_memory->ActiveConnections += count;
}

bool Connection::IsConnectionTerminated() const
{
	return shared_memory->Terminated;
}
