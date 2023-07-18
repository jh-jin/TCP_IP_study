
#include "EchoServer.hpp"

void EchoServer::handleError(char *buf)
{
	std::cerr << buf << "\n";
	exit(1);
}

void EchoServer::createSocket() {
	serv_sock_ = socket(PF_INET, SOCK_STREAM, 0);
}

void EchoServer::bindSocket(int port) {
	struct sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(port);
	
	if (bind(serv_sock_, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
		handleError("bind() error");
}

void EchoServer::listenForClients()
{
	if (listen(serv_sock_, 5) == -1)
		handleError("listen() error");
}

int EchoServer::start(int port)
{
	createSocket();
	bindSocket(port);
	listenForClients();
}

void EchoServer::run()
{
	int clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;

	struct timeval timeout;
	fd_set reads, cpy_reads;

	socklen_t adr_size;
	int fd_max, str_len, fd_num, i;
	char buf[BUF_SIZE];

	FD_ZERO(&reads);
	FD_SET(serv_sock_, &reads);
	fd_max = serv_sock_;

	while (1)
	{
		cpy_reads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout);
		if (fd_num == -1)
			break;
		if (fd_num == 0)
			continue;

		for (i = 0; i < fd_max + 1; i++)
		{
			if (FD_ISSET(i, &cpy_reads))
			{
				if (i == serv_sock_) // connect request
				{
					adr_size = sizeof(clnt_adr);
					clnt_sock = accept(serv_sock_, (struct sockaddr *)&clnt_adr, &adr_size);
					FD_SET(clnt_sock, &reads);
					if (fd_max < clnt_sock)
						fd_max = clnt_sock;
					printf("connected client: %d\n", clnt_sock);
				}
				else // read message
				{
					str_len = read(i, buf, BUF_SIZE);
					if (str_len == 0)
					{
						FD_CLR(i, &reads);
						close(i);
						printf("closed client: %d\n", i);
					}
					else
						std::cout << buf << std::endl;
				}
			}
		}
	}
	close(serv_sock_);
}

int main(int argc, char **argv)
{
	if (argc != 2)
		exit(1);

	EchoServer server;
	int port = atoi(argv[1]);

	if (server.start(port))
		server.run();
}