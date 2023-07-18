#ifndef ECHO_SERVER_HPP
# define ECHO_SERVER_HPP

# include <iostream>
# include <cstdlib>
# include <cstring>
# include <unistd.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <sys/select.h>

# define BUF_SIZE 100

class EchoServer {
	public:
		EchoServer();
		~EchoServer();
		
		int 				start(int port);
		void				run();
		
	private:
		void				acceptRequest(fd_set *reads);
		void				readMessage(int i, fd_set *reads);

		void 				createSocket();
		void 				bindSocket(int port);
		void				listenForClients();

		void 				handleError(char *buf);

		int					serv_sock_, clnt_sock_;
		struct sockaddr_in	serv_adr_;
};

#endif