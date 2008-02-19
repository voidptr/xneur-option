
#include "net_io.h"
#include "workers_pool.h"
#include "functions.h"
#include "defines.h"

int main(int, char**)
{
	NetIO *net_io = new NetIO(WORK_PORT);
	WorkersPool *workers_pool = new WorkersPool(net_io);

	workers_pool->CreateWorkers(WORKERS_COUNT);

	net_io->Accept(workers_pool);

	delete workers_pool;
	delete net_io;

	return EXIT_SUCCESS;
}
