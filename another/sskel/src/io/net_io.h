
#ifndef _NET_IO_H_
#define _NET_IO_H_

class WorkersPool;

class NetIO
{
private:
	int port;
	int listen_fd;
	char *ip_address;
	bool is_server;
	bool terminated;

public:
	NetIO(int work_port);
	~NetIO();

	void SetIPAddress(const char *ip);
	void SetMode(bool server);

	void Init();
	void DeInit();
	void Accept(WorkersPool *workers);

	int Read(int conn_fd) const;
	bool Shutdown(int conn_fd) const;
	bool Close(int conn_fd) const;

	bool AddFcntl(int conn_fd, int add_fcntl) const;
	bool SetFcntl(int conn_fd, int type, int set_fcntl) const;
	bool SetIoctl(int conn_fd, int type, int set_ioctl) const;
	bool SetSocketOption(int conn_fd, int level, int option, int value) const;
};

#endif /* _NET_IO_H_ */
