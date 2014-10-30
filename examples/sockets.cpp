#include <iostream>
#include <string>
#include <cstring>
#include "sockets.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
#ifdef _ISWINDOWS
static bool inited_socket=false;
static int init_winSocket()
{
	if(!inited_socket)
	{
		//�����׽���
		WORD myVersionRequest;
		WSADATA wsaData;
		myVersionRequest=MAKEWORD(1,1);
		int err;
		err=WSAStartup(myVersionRequest,&wsaData);
		if (!err){
			//cerr<<"�Ѵ��׽���"<<endl;
			inited_socket=true;
			return 0;
		}else{
			cerr<<"ERROR:Ƕ����δ��!"<<endl;
			return 1;
		}
	}
	return 0;
}

static int drop_winSocket()
{
	if(inited_socket)
		return WSACleanup();//�ͷ���Դ�Ĳ���
	else 
		return -1;
}
#else
#define drop_winSocket()
#define init_winSocket()
#endif
static int set_addr_reuse(const SOCKET& fd)
{
	int ret;
#ifndef _ISWINDOWS
	const int opt=1;
#else
	const char opt=1;
#endif
	ret=setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	if(ret<0)
	{
		perror("setsockopt");
	}
	return ret;
}
static int net2hInfo(const SOCKADDR_IN addr,string &ip,uint16 &port)
{
	ip=inet_ntoa(addr.sin_addr);
	port=ntohs(addr.sin_port);
	return 0;
}
udpSender::udpSender(const std::string &ip,const uint16 &dest_port)
{
	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	fd=socket(AF_INET,SOCK_DGRAM,0);//�����˿�ʶ���׽���
	if(fd==INVALID_SOCKET)
	{
		cerr<<"err fd"<<fd<<endl;
		perror("socket");
	}
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr =inet_addr(ip.c_str());//ip��ַ
	serv_addr.sin_port=htons(dest_port);//�󶨶˿�
}
udpSender::udpSender(const std::string &ip,const uint16 &dest_port,const uint16 &local_port) 
{
	
	new (this) udpSender(ip,dest_port);
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);//ip��ַ
	addr.sin_port=htons(local_port);//�󶨶˿�
	set_addr_reuse(fd);
	int ret=bind(fd,(struct sockaddr*)&addr,sizeof(struct sockaddr_in));//�����
	if(ret<0)
	{
		cerr<<"bind err:"<<ret<<endl;
		perror("bind");
	}
}

int udpSender::send(const void *buf,size_t size) const
{
	return send(serv_addr,buf,size);
}

int udpSender::recv(void *buf,size_t size) const
{
	SOCKADDR_IN from_addr;
	socklen_t fromlen=sizeof(SOCKADDR_IN);
	int ret= recv(buf,size,from_addr,(size_t &)fromlen);
	if(ret<0)
	{
		perror("recvform");
	}
	return ret;
}
int udpSender::recv(void *buf,size_t size,struct sockaddr_in& from_addr, size_t &fromlen)const
{
	fromlen=sizeof(SOCKADDR_IN);

	int ret= recvfrom(fd,(char *)buf,size,0,(sockaddr *)&from_addr,(socklen_t *)&fromlen);
	if(ret<0)
	{
		perror("recvform");
	}
	return ret;
}
int udpSender::recv(void *buf,size_t size,std::string & form_addr, uint16 &from_port)const
{
	SOCKADDR_IN from_addr;
	socklen_t fromlen=sizeof(SOCKADDR_IN);
	int ret= recv(buf,size,from_addr,(size_t &)fromlen);
	if(ret<0)
	{
		perror("recvform");
	}
	else
	{
		net2hInfo(from_addr,form_addr,from_port);
	}
	return ret;
}

int udpSender::send(const SOCKADDR_IN &to_addr,const void *buf,size_t size) const
{
	int ret=sendto(fd,(const char *)buf,size,0,(const struct sockaddr*)&to_addr,sizeof(struct sockaddr));
	if(ret<0)
	{
		perror("sendto");
		return -1;
	}
	return 0;
}

int udpSender::send(const string &ip,uint16 port,const void *buf,size_t size) const
{
	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_addr.s_addr=inet_addr(ip.c_str());
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	return send(addr,buf,size);
}

tcpListener::tcpListener(const std::string &listen_ip,const uint16 &listen_port,int maxConnect)
{
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd==INVALID_SOCKET)
	{
		cerr<<"err fd"<<fd<<endl;
		perror("socket");
		return;
	}
	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(SOCKADDR_IN));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(listen_port);
	addr.sin_addr.s_addr=inet_addr(listen_ip.c_str());
	set_addr_reuse(fd);
	if(bind(fd,(const sockaddr*)&addr,sizeof(SOCKADDR_IN))==SOCKET_ERROR)
	{
		cerr<<"bind err"<<endl;
		perror("bind");
	}
	if(listen(fd,maxConnect)==SOCKET_ERROR)
	{
		cerr<<"listen err"<<endl;
		perror("listen");
		return;
	}
}

tcpListener::tcpListener(const uint16 &listen_port,int maxConnect)
{
	struct in_addr ip;
	ip.s_addr=htonl(INADDR_ANY);
	string ip_str=inet_ntoa(ip);
	new (this) tcpListener(ip_str.c_str(),listen_port,maxConnect);
}

tcpSender tcpListener::accept(sockaddr* addr,int* addrlen)
{
	SOCKET ret;
#ifdef _ISWINDOWS
	if((ret=::accept(fd,addr,addrlen))==SOCKET_ERROR)
#else
	if((ret=::accept(fd,addr,(size_t*)addrlen))==SOCKET_ERROR)
#endif
	{
		cerr<<"accept err"<<endl;
		perror("accept");
		return tcpSender(INVALID_SOCKET);
	}
	else
	{
		return  tcpSender(ret);
	}
}

tcpSender::tcpSender(const string &ip,uint16 serv_port)
{
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd==INVALID_SOCKET)
	{
		cerr<<"err fd "<<fd<<endl;
		//cerr<<"err id "<<WSAGetLastError()<<endl;
		perror("socket");
		return;
	}
	memset(&serv_addr,0,sizeof(SOCKADDR_IN));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(serv_port);
	serv_addr.sin_addr.s_addr=inet_addr(ip.c_str());
}
tcpSender::tcpSender(const std::string &ip,uint16 serv_port,uint16 local_port)
{
	new (this) tcpSender(ip,serv_port);
	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(SOCKADDR_IN));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(local_port);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	set_addr_reuse(fd);
	if(bind(fd,(const sockaddr*)&addr,sizeof(SOCKADDR_IN))==SOCKET_ERROR)
	{
		cerr<<"bind err"<<endl;
		perror("bind");
	}
}

tcpSender::tcpSender(const SOCKADDR_IN &_serv_addr,uint16 local_port)
{
	new (this) tcpSender(_serv_addr);
	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(SOCKADDR_IN));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(local_port);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	set_addr_reuse(fd);
	if(bind(fd,(const sockaddr*)&addr,sizeof(SOCKADDR_IN))==SOCKET_ERROR)
	{
		cerr<<"bind err"<<endl;
		perror("bind");
	}
}

int tcpSender::send(const void *buf,size_t size) const
{
	int ret=::send(fd,(const char *)buf,(int) size,0);
	if(ret<0)
	{
		cerr<<"send err"<<endl;
		perror("send");
	}
	return ret;
}

int tcpSender::recv( void *buf,size_t size) const
{
	int ret=::recv(fd,(char *)buf,(int) size,0);
	if(ret<0)
	{
		cerr<<"recv err"<<endl;
		perror("recv");
	}
	else if(ret==0)
	{
		cerr<<"the socket is disconnection"<<endl;
	}
	return ret; 
}
int tcpSender::connect()
{
	int ret=::connect(fd,(sockaddr*)&serv_addr,sizeof(SOCKADDR_IN));
	if(ret==SOCKET_ERROR)
	{
		cerr<<"connect err"<<endl;
		perror("connect");
	}
	return 0;
}

tcpSender::tcpSender(const SOCKADDR_IN &_serv_addr):serv_addr(_serv_addr){}
Socketer::Socketer():fd(INVALID_SOCKET){init_winSocket();}
Socketer::Socketer(SOCKET _fd):fd(_fd){

}
Socketer::Socketer(const Socketer &socketer)
{
	init(socketer.getSocket());
}
void Socketer::init(SOCKET _fd)
{
	init_winSocket();
#ifndef _ISWINDOWS
	fd=dup(_fd);
#endif
}
Socketer &Socketer::operator=(const Socketer &socketer)
{
	this->Close();
#ifndef _ISWINDOWS
	this->fd=dup(socketer.getSocket());
#else
	this->fd=socketer.getSocket();
#endif
	return *this;
}
