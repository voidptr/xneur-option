
#include "worker.h"
#include "worker_fork.h"
#include "connection.h"
#include "connection_select.h"
#include "connection_kqueue.h"
#include "descriptors_transmitter.h"
#include "functions.h"
#include "defines.h"

#include <sys/wait.h>

#include <errno.h>

static WorkerFork *worker_fork;

static void signals_handler(int signal)
{
	switch(signal)
	{
		case SIGTERM:
		{
			worker_fork->OnSigTermReceived();
			break;
		}
	}
}

WorkerFork::WorkerFork(const NetIO *io, int worker_id) :
	Worker(io, worker_id), connection(NULL), child_pid(0)
{
	worker_fork = this;

	descriptors_transmitter = new DescriptorsTransmitter(net_io);

	child_pid = fork();
	if(child_pid == -1)
		error("Error %d occured in fork()", errno);

	if(child_pid == 0)
		StartupWorker();

	descriptors_transmitter->Parent();
}

WorkerFork::~WorkerFork()
{
	send_signal(child_pid, SIGTERM);

	int status;
	for(int i = 0; i < WORKER_WAIT_SECONDS; i++)
	{
		int pid = waitpid(child_pid, &status, WUNTRACED | WNOHANG);
		if(pid == child_pid)
			break;
		sleep(1);
	}

	delete descriptors_transmitter;
}

Worker* WorkerFork::Clone() const
{
	WorkerFork *worker_clone = new WorkerFork(net_io);
	return worker_clone;
}

int WorkerFork::GetPID() const
{
	return child_pid;
}

void WorkerFork::StartupWorker()
{
	child_pid = getpid();

	trap_signal(SIGTERM, signals_handler);
	trap_signal(SIGPIPE, SIG_IGN);

	descriptors_transmitter->Child(child_pid);

#ifdef CONNECTION_SELECT
	connection = new ConnectionSelect(net_io);
#elif defined CONNECTION_KQUEUE
	connection = new ConnectionKqueue(net_io);
#else
	#error Define CONNECTION_SELECT or CONNECTION_KQUEUE
#endif

	connection->SetSharedMemory(shared_memory);

	while(true)
	{
		if(IsWorkerTerminated())
			break;

		if(descriptors_transmitter->IsDescriptorsAvailable())
			ReadSocketDescriptors();

		connection->Process();
	}

	delete connection;
	exit(EXIT_SUCCESS);
}

void WorkerFork::OnSigTermReceived()
{
	SetWorkerTerminated(true);
}

bool WorkerFork::Accept(int conn_fd) // Всегда вызывается в родителе
{
	if(!descriptors_transmitter->Send(conn_fd))
		return false;
	return Worker::Accept(conn_fd);
}

void WorkerFork::ReadSocketDescriptors()
{
	Connection::sockets_collection *readed = descriptors_transmitter->Receive();
	connection->AddNewSocketDescriptors(readed);
	delete readed;
}
