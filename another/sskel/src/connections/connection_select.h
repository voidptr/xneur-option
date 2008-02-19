
#ifndef _CONNECTION_SELECT_H_
#define _CONNECTION_SELECT_H_

class Connection;
class NetIO;

class ConnectionSelect : public Connection
{
private:
	sockets_collection *sockets;

public:
	ConnectionSelect(const NetIO *io);
	virtual ~ConnectionSelect();

	virtual void AddNewSocketDescriptors(sockets_collection *readed);
	virtual void Process();
};

#endif /* _CONNECTION_SELECT_H_ */
