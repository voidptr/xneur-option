
#ifndef _WORKER_FORK_H_
#define _WORKER_FORK_H_

class NetIO;
class Worker;
class Connection;
class DescriptorsTransmitter;

class WorkerFork : public Worker
{
private:
	Connection *connection;
	DescriptorsTransmitter *descriptors_transmitter;
	int child_pid;

	void StartupWorker();
	void ReadSocketDescriptors();

public:
	WorkerFork(const NetIO *io, int worker_id = 0);
	virtual ~WorkerFork();

	virtual bool Accept(int conn_fd);

	virtual Worker* Clone() const;
	virtual int GetPID() const;

	void OnSigTermReceived();
	void OnSigIOReceived();
};

#endif /* _WORKER_FORK_H_ */
