
#include "connection.h"
#include "connection_select.h"
#include "net_io.h"
#include "functions.h"

#include <errno.h>

ConnectionSelect::ConnectionSelect(const NetIO *io) :
	Connection(io)
{
	sockets = new sockets_collection();
}

ConnectionSelect::~ConnectionSelect()
{
	for(sockets_iterator iter = sockets->begin(); iter != sockets->end(); iter++)
		net_io->Close(*iter);
	delete sockets;
}

void ConnectionSelect::Process()
{
	int max_fd = -1;
	fd_set rfds;

	FD_ZERO(&rfds);

	for(sockets_iterator iter = sockets->begin(); iter != sockets->end(); iter++)
	{
		int conn_fd = *iter;

		FD_SET(conn_fd, &rfds);
		if(conn_fd > max_fd)
			max_fd = conn_fd;
	}

	if(IsConnectionTerminated())
		return;

	int selected = select(max_fd + 1, &rfds, NULL, NULL, NULL);

	if(IsConnectionTerminated())
		return;

	if(selected == -1)
	{
		if(errno != EINTR)
			error("Error %d occured in select()", errno);
		return;
	}

	for(sockets_iterator iter = sockets->begin(); iter != sockets->end(); iter++)
	{
		int conn_fd = *iter;
		if(!FD_ISSET(conn_fd, &rfds))
			continue;

		int status = net_io->Read(conn_fd);
		if(status < 0)
		{
			warning("Error %d occured while reading data from socket", errno);
			status = 0;
		}

		if(status == 0)
		{
			RemoveSocket(conn_fd);

			iter = sockets->erase(iter);
			if(sockets->size() != 0)
				iter--;
		}

		if(--selected == 0)
			break;
	}
}

void ConnectionSelect::AddNewSocketDescriptors(sockets_collection *readed)
{
	IncreaseConnectionsCount(readed->size());
	for(sockets_iterator iter = readed->begin(); iter != readed->end(); iter++)
	{
		if((unsigned int)*iter >= FD_SETSIZE)
		{
			warning("Can't select more than or equal to %d sockets", FD_SETSIZE);
			RemoveSocket(*iter);
			continue;
		}

		sockets->push_back(*iter);
	}
}
