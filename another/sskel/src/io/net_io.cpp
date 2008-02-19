
#include "net_io.h"
#include "workers_pool.h"
#include "functions.h"
#include "defines.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <errno.h>
#include <fcntl.h>

static NetIO *net_io;

static void signals_handler(int signal)
{
	switch(signal)
	{
		case SIGTERM:
		{
			net_io->DeInit();
			break;
		}
	}
}

NetIO::NetIO(int work_port) :
	port(work_port), listen_fd(0), ip_address(NULL), is_server(true),
	terminated(false)
{
	net_io = this;

	trap_signal(SIGTERM, signals_handler);
}

NetIO::~NetIO()
{
	DeInit();
	SetIPAddress(NULL);
}

void NetIO::SetIPAddress(const char *ip)
{
	if(ip_address != NULL)
		delete [] ip_address;
	ip_address = copy_pchar(ip);
}

void NetIO::SetMode(bool server)
{
	is_server = server;
}

void NetIO::Init()
{
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd == -1)
		error("Error %d occured in socket()", errno);

	if(is_server)
	{
		if(!SetSocketOption(listen_fd, SOL_SOCKET, SO_REUSEADDR, 1))
			error("Error %d occured in SetSocketOption(listen_fd, SO_REUSEADDR)", errno);
	}

	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	if(ip_address != NULL)
	{
		if(inet_pton(AF_INET, ip_address, &servaddr.sin_addr) == -1)
			error("Error %d occured in inet_pton(%s)", errno, ip_address);
	}
	else
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listen_fd, (sockaddr*) &servaddr, sizeof(servaddr)) == -1)
		error("Error %d occured in bind()", errno);
}

void NetIO::DeInit()
{
	terminated = true;
	if(listen_fd != 0)
		close(listen_fd);
	listen_fd = 0;
	is_server = true;
}

void NetIO::Accept(WorkersPool *workers)
{
	if(listen_fd == 0)
		Init();

	if(!SetSocketOption(listen_fd, SOL_SOCKET, SO_RCVBUF, MAX_BUFF_SIZE))
		error("Error %d occured in SetSocketOption(listen_fd, SO_RCVBUF, %d)", errno, MAX_BUFF_SIZE);

	if(!SetSocketOption(listen_fd, SOL_SOCKET, SO_SNDBUF, MAX_BUFF_SIZE))
		error("Error %d occured in SetSocketOption(listen_fd, SO_SNDBUF, %d)", errno, MAX_BUFF_SIZE);

	if(listen(listen_fd, -1) == -1)
		error("Error %d occured in listen(listen_fd)", errno);

	while(true)
	{
		int conn_fd = accept(listen_fd, (sockaddr*) NULL, NULL);
		if(terminated)
			return;

		if(conn_fd == -1)
		{
			if(errno == EINTR || errno == ECONNABORTED)
				continue;
			error("Error %d occured in accept()", errno);
		}

		if(!AddFcntl(conn_fd, O_NONBLOCK))
			error("Error %d occured in AddFcntl(conn_fd, O_NONBLOCK)", errno);

		if(!SetSocketOption(conn_fd, SOL_SOCKET, SO_KEEPALIVE, 1))
			error("Error %d occured in SetSocketOption(conn_fd, SO_KEEPALIVE, 1)", errno);

		if(!SetSocketOption(conn_fd, IPPROTO_TCP, TCP_NODELAY, 1))
			error("Error %d occured in SetSocketOption(conn_fd, TCP_NODELAY, 1)", errno);

		workers->Accept(conn_fd);
	}
}
//-----------------------------------------------------------------------------
int NetIO::Read(int conn_fd) const
{
	char str[1024 + 1];

	while(true)
	{
		int readed = read(conn_fd, str, 1024);
		if(readed == -1)
		{
			if(errno == EWOULDBLOCK)
				return 1;

			if(errno == EINTR)
				continue;

			return -1;
		}
		
		if(readed == 0)
			return 0;

		str[readed] = '\0';
		printf("%s", str);
	}
}

bool NetIO::Shutdown(int conn_fd) const
{
	return (shutdown(conn_fd, SHUT_RDWR) != -1);
}

bool NetIO::Close(int conn_fd) const
{
	return (close(conn_fd) != -1);
}

bool NetIO::AddFcntl(int conn_fd, int add_fcntl) const
{
	int fcntls = fcntl(conn_fd, F_GETFL, 0);
	if(fcntls == -1)
		return false;

	return (fcntl(conn_fd, F_SETFL, fcntls | add_fcntl) != -1);
}

bool NetIO::SetFcntl(int conn_fd, int type, int set_fcntl) const
{
	return (fcntl(conn_fd, type, set_fcntl) != -1);
}

bool NetIO::SetIoctl(int conn_fd, int type, int set_ioctl) const
{
	return (ioctl(conn_fd, type, &set_ioctl) != -1);
}

bool NetIO::SetSocketOption(int conn_fd, int level, int option, int value) const
{
	return (setsockopt(conn_fd, level, option, &value, sizeof(value)) != -1);
}
