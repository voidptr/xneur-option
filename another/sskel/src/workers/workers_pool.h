
#ifndef _WORKERS_POOL_H_
#define _WORKERS_POOL_H_

#include <map>

class Worker;
class NetIO;

class WorkersPool
{
private:
	const NetIO *net_io;

	typedef std::map<int, Worker*> workers_collection;
	typedef workers_collection::iterator workers_iterator; 

	workers_collection *workers;

	void RemoveWorker(workers_iterator pos);

public:
	WorkersPool(const NetIO *io);
	~WorkersPool();

	void Accept(int conn_fd);

	void AddWorker(Worker *worker);
	void CreateWorkers(int workers_count);
	void RestartWorker(int pid);
	void OnChildsTerminated();

	int GetWorkersCount() const;
};

#endif /* _WORKERS_POOL_H_ */
