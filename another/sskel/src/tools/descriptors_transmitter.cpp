
#include "connection.h"
#include "descriptors_transmitter.h"
#include "net_io.h"
#include "functions.h"
#include "defines.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <errno.h>
#include <fcntl.h>

static DescriptorsTransmitter *descriptors_transmitter = NULL;

static void signals_handler(int signal)
{
	switch(signal)
	{
		case SIGIO:
		{
			descriptors_transmitter->OnSigIOReceived();
			break;
		}
	}
}

DescriptorsTransmitter::DescriptorsTransmitter(const NetIO *io) :
	net_io(io), sigio_occured(false)
{
	descriptors_transmitter = this;

	Init();
}

DescriptorsTransmitter::~DescriptorsTransmitter()
{
	DeInit();
}

void DescriptorsTransmitter::Init()
{
	if(socketpair(AF_LOCAL, SOCK_STREAM, 0, handlers.fds) == -1)
		error("Error %d occured in socketpair()", errno);

	if(!net_io->AddFcntl(handlers.named_fds.read_fd, O_NONBLOCK))
		error("Error %d occured in AddFcntl(read_fd, O_NONBLOCK)", errno);

#ifdef DESRIPTOR_TRANSMITTER_NONBLOCK
	if(!net_io->AddFcntl(handlers.named_fds.write_fd, O_NONBLOCK))
		error("Error %d occured in AddFcntl(write_fd, O_NONBLOCK)", errno);
#endif

	if(!net_io->SetSocketOption(handlers.named_fds.write_fd, SOL_SOCKET, SO_SNDBUF, MAX_BUFF_SIZE))
		error("Error %d occured in SetSocketOption(write_fd, SO_SNDBUF, %d)", errno, MAX_BUFF_SIZE);

	if(!net_io->SetSocketOption(handlers.named_fds.read_fd, SOL_SOCKET, SO_RCVBUF, MAX_BUFF_SIZE))
		error("Error %d occured in SetSocketOption(read_fd, SO_RCVBUF, %d)", errno, MAX_BUFF_SIZE);

	if(!net_io->SetIoctl(handlers.named_fds.read_fd, FIOASYNC, 1))
		error("Error %d occured in SetIoctl(read_fd, FIOASYNC)", errno);

	cmsg = new cmsghdr_fd();
	cmsg->cmsg_len = sizeof(cmsghdr_fd);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;

	iov_base = new char[1];
	iov_base[0] = 0xFF;

	iov = new iovec();
	iov->iov_len = 1;
	iov->iov_base = (void*)iov_base;

	msg = new msghdr();
	msg->msg_name = 0;
	msg->msg_namelen = 0;
	msg->msg_iov = iov;
	msg->msg_iovlen = 1;
	msg->msg_control = cmsg;
	msg->msg_controllen = cmsg->cmsg_len;
}

void DescriptorsTransmitter::DeInit()
{
	CloseDescriptor(true);
	CloseDescriptor(false);

	delete    msg;
	delete    iov;
	delete [] iov_base;
	delete    cmsg;
}

bool DescriptorsTransmitter::Send(int descriptor)
{
	cmsg->cmsg_data = descriptor;

	while(true)
	{
		int writted = sendmsg(handlers.named_fds.write_fd, msg, 0);
		if(writted < 0)
		{
			if(errno == EINTR)
				continue;
			return false;
		}

		return true;
	}
}

Connection::sockets_collection* DescriptorsTransmitter::Receive()
{
 	Connection::sockets_collection *received = new Connection::sockets_collection();

	while(true)
	{
		int readed = recvmsg(handlers.named_fds.read_fd, msg, 0);
		if(readed < 0)
		{
			if(errno == EWOULDBLOCK)
				break;

			if(errno == EINTR)
				continue;

			delete received;
			error("Error %d occured int recvmsg()", errno);
		}

		if(readed == 0)
			break;

		received->push_back(cmsg->cmsg_data);
	}

	sigio_occured = false;
	return received;
}

int DescriptorsTransmitter::GetReadHandler() const
{
	return handlers.named_fds.read_fd;
}

void DescriptorsTransmitter::CloseDescriptor(bool read)
{
	if(read)
	{
		if(handlers.named_fds.read_fd == 0)
			return;

		if(!net_io->Close(handlers.named_fds.read_fd))
			warning("Can't close write handler");

		handlers.named_fds.read_fd = 0;
	}
	else
	{
		if(handlers.named_fds.write_fd == 0)
			return;

		if(!net_io->Close(handlers.named_fds.write_fd))
			warning("Can't close read handler");

		handlers.named_fds.write_fd = 0;
	}
}

void DescriptorsTransmitter::SetDescriptorOwner(bool read, int owner_pid)
{
	if(read)
	{
		if(handlers.named_fds.read_fd == 0)
			return;

		if(!net_io->SetFcntl(handlers.named_fds.read_fd, F_SETOWN, owner_pid))
			error("Error %d occured in SetFcntl(read_fd, F_SETOWN, %d)", errno, owner_pid);
	}
	else
	{
		if(handlers.named_fds.write_fd == 0)
			return;

		if(!net_io->SetFcntl(handlers.named_fds.write_fd, F_SETOWN, owner_pid))
			error("Error %d occured in SetFcntl(write_fd, F_SETOWN, %d)", errno, owner_pid);
	}
}

void DescriptorsTransmitter::Parent()
{
	CloseDescriptor(true);
}

void DescriptorsTransmitter::Child(int child_pid)
{
	trap_signal(SIGIO, signals_handler);

	CloseDescriptor(false);
	SetDescriptorOwner(true, child_pid);
}

void DescriptorsTransmitter::OnSigIOReceived()
{
	sigio_occured = true;
}

bool DescriptorsTransmitter::IsDescriptorsAvailable() const
{
	return sigio_occured;
}
