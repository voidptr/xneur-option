
#ifndef _CONNECTION_KQUEUE_H_
#define _CONNECTION_KQUEUE_H_

#include <vector>

class Connection;
class NetIO;
struct kevent;

class ConnectionKqueue : public Connection
{
private:
	typedef struct kevent kevent_struct;
	typedef std::vector<kevent_struct> events_collection;

	int queue_descriptor;
	events_collection *events;
	events_collection *handled_events;

	kevent_struct* CreateEvent();
	void DestroyEvent(kevent_struct *event);

public:
	ConnectionKqueue(const NetIO *io);
	virtual ~ConnectionKqueue();

	virtual void Process();
	virtual void AddNewSocketDescriptors(sockets_collection *readed);
};

#endif /* _CONNECTION_KQUEUE_H_ */
