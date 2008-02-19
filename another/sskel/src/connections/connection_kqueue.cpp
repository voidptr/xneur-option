
#include "connection.h"
#include "connection_kqueue.h"
#include "net_io.h"
#include "functions.h"
#include "defines.h"

#include <sys/event.h>

#include <errno.h>

ConnectionKqueue::ConnectionKqueue(const NetIO *io) :
	Connection(io)
{
	queue_descriptor = kqueue();
	if(queue_descriptor == -1)
		error("Error %d occured in kquque()", errno);

	events = new events_collection();
	handled_events = new events_collection();

	events->reserve(1024);
	handled_events->reserve(1024);
}

ConnectionKqueue::~ConnectionKqueue()
{
	delete handled_events;
	delete events;
	close(queue_descriptor);
}

void ConnectionKqueue::Process()
{
	if(IsConnectionTerminated())
		return;

	int events_count = events->size();
	int selected = kevent(queue_descriptor, &events->front(), events_count, &handled_events->front(), events_count, NULL);

	if(IsConnectionTerminated())
		return;

	if(selected == -1)
	{
		if(errno != EINTR)
			error("Error %d occured in kevent()", errno);
		return;
	}

	for(int i = 0; i < selected; i++)
	{
		kevent_struct *handled_event = &handled_events->at(i);
		if(handled_event->flags & EV_ERROR)
		{
			warning("Unhandled error %d occured in socket", errno);
			DestroyEvent((kevent_struct*)handled_event->udata);
			continue;
		}

		if(handled_event->filter != EVFILT_READ)
		{
			warning("Unwaited action %d occured in socket", handled_event->filter);
			DestroyEvent((kevent_struct*)handled_event->udata);
			continue;
		}

		int status = net_io->Read(handled_event->ident);
		if(status < 0)
		{
			warning("Error %d occured while reading data from socket", errno);
			status = 0;
		}

		if(status == 0)
			DestroyEvent((kevent_struct*)handled_event->udata);
	}
}

void ConnectionKqueue::AddNewSocketDescriptors(sockets_collection *readed)
{
	for(sockets_iterator iter = readed->begin(); iter != readed->end(); iter++)
	{
		int conn_fd = *iter;

		kevent_struct *event = CreateEvent();

		EV_SET(event, conn_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, event);
	}
	IncreaseConnectionsCount(readed->size());
}

void ConnectionKqueue::DestroyEvent(kevent_struct *event)
{
	RemoveSocket(event->ident);
	EV_SET(event, event->ident, 0, EV_DELETE, 0, 0, NULL);

	kevent_struct *last_event = &events->back();
	if(event != last_event)
	{
		*event = *last_event;
		event->udata = event;
	}

	events->pop_back();
}

ConnectionKqueue::kevent_struct* ConnectionKqueue::CreateEvent()
{
	kevent_struct event;

	events->push_back(event);

	if(events->size() > handled_events->size())
		handled_events->push_back(event);

	return &events->back();
}
