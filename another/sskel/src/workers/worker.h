
#ifndef _WORKER_H_
#define _WORKER_H_

class NetIO;
class Connection;
struct SharedMemory;

class Worker
{
private:
	int id;

protected:
	const NetIO *net_io;

	SharedMemory *shared_memory;

	void SetWorkerTerminated(bool shutdown);

	bool IsWorkerTerminated() const;

public:
	Worker(const NetIO *net_io, int worker_id = 0);
	virtual ~Worker();

	virtual bool Accept(int conn_fd);

	virtual Worker* Clone() const = 0;
	virtual int GetPID() const = 0;

	int GetActiveConnections() const;
	int GetID() const;
};

#endif /* _WORKER_H_ */
