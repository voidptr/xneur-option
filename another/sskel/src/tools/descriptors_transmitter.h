
#ifndef _DESCRIPTORS_TRANSMITTER_H_
#define _DESCRIPTORS_TRANSMITTER_H_

class NetIO;
class Connection;

struct iovec;
struct msghdr;

class DescriptorsTransmitter
{
private:
	const NetIO *net_io;

	union
	{
		struct
		{
			int read_fd;
			int write_fd;
		} named_fds;
		int fds[2];
	}  handlers;

	struct cmsghdr_fd
	{
		int cmsg_len;
		int cmsg_level;
		int cmsg_type;
		int cmsg_data;
	} *cmsg;

	char *iov_base;
	iovec *iov;
	msghdr *msg;
	bool sigio_occured;

	void Init();
	void DeInit();
	void CloseDescriptor(bool read);
	void SetDescriptorOwner(bool read, int owner_pid);

public:
	DescriptorsTransmitter(const NetIO *io);
	~DescriptorsTransmitter();

	void Parent();
	void Child(int child_pid);

	void OnSigIOReceived();

	bool Send(int descriptor);
	Connection::sockets_collection* Receive();

	int GetReadHandler() const;
	bool IsDescriptorsAvailable() const;
};

#endif /* _DESCRIPTIONS_TRANSMITTER_H_ */
