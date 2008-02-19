
#include "worker.h"
#include "connection.h"
#include "net_io.h"

#include <sys/mman.h>

Worker::Worker(const NetIO *io, int worker_id) :
	net_io(io)
{
	if (worker_id == 0)
		worker_id = random();
	id = worker_id;

	shared_memory = (SharedMemory*) mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (shared_memory == MAP_FAILED)
		throw "Can't mmap in worker";

	SetWorkerTerminated(false);
}

Worker::~Worker()
{
	munmap(shared_memory, sizeof(SharedMemory));
}

bool Worker::Accept(int conn_fd)
{
	if (!net_io->Close(conn_fd))
		throw "Can't close socket descriptor";
	return true;
}

int Worker::GetActiveConnections() const
{
	return shared_memory->ActiveConnections;
}

void Worker::SetWorkerTerminated(bool shutdown)
{
	shared_memory->Terminated = shutdown;
}

bool Worker::IsWorkerTerminated() const
{
	return shared_memory->Terminated;
}

int Worker::GetID() const
{
	return id;
}
