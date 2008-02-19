
#include "workers_pool.h"
#include "worker.h"
#include "worker_fork.h"
#include "net_io.h"
#include "functions.h"

#include <sys/wait.h>

static WorkersPool *workers_pool;
static void signals_handler(int signal)
{
	switch(signal)
	{
		case SIGCHLD:
		{
			workers_pool->OnChildsTerminated();
			break;
		}
	}
}

WorkersPool::WorkersPool(const NetIO *io) :
	net_io(io)
{
	workers_pool = this;

	workers = new workers_collection();

	srandomdev();

	trap_signal(SIGCHLD, signals_handler);
}

WorkersPool::~WorkersPool()
{
	trap_signal(SIGCHLD, SIG_IGN);

	for(workers_iterator iter = workers->begin(); iter != workers->end(); iter++)
		delete iter->second;
	delete workers;
}

void WorkersPool::AddWorker(Worker *worker)
{
	int pid = worker->GetPID();
	workers->insert(std::make_pair(pid, worker));
}

void WorkersPool::CreateWorkers(int workers_count)
{
	for(int i = 0; i < workers_count; i++)
	{
		Worker *worker = new WorkerFork(net_io);
		AddWorker(worker);
	}
}

void WorkersPool::RestartWorker(int pid)
{
	workers_iterator pos = workers->find(pid);
	if(pos == workers->end())
		return;

	Worker *worker = pos->second;

	AddWorker(worker->Clone());
	RemoveWorker(pos);
}

void WorkersPool::RemoveWorker(workers_iterator pos)
{
	Worker *worker = pos->second;
	workers->erase(pos);
	delete worker;
}

void WorkersPool::Accept(int conn_fd)
{
#ifdef BALANCE_LOAD
	Worker *selected_worker = NULL;
	int min_use = -1;

	for(workers_iterator iter = workers->begin(); iter != workers->end(); iter++)
	{
		Worker *worker = iter->second;
		int use = worker->GetActiveConnections();
		if(min_use == -1 || use < min_use)
		{
			min_use = use;
			selected_worker = worker;
		}
		if(min_use == 0)
			break;
	}

	if(selected_worker == NULL)
		error("There is no available workers");

	if(!selected_worker->Accept(conn_fd))
		net_io->Close(conn_fd);
#else
	
	workers_iterator last_worker = workers->begin();

	while(last_worker != workers->end())
	{
		if(last_worker->second->Accept(conn_fd))
			return;
		last_worker++;
	}

	error("There is no available workers");
#endif
}

int WorkersPool::GetWorkersCount() const
{
	return workers->size();
}

void WorkersPool::OnChildsTerminated()
{
	while(true)
	{
		int stat;
		int pid = waitpid(-1, &stat, WNOHANG | WUNTRACED);
		if(pid <= 0)
			return;

		RestartWorker(pid);
	}
}
