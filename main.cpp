# include "includes/main.hpp"

int main(int __ac, char *__av[])
{
	int 				__socket_fd, __connection, __poll_res = 0, __recv_res;
	unsigned long		__address_len;
	vector<int>			__clients;
	char __buffer[1024] = {0};
	struct sockaddr_in	__server_addr = {};
	struct pollfd	__poll_fds[MAX_FD] = {};
	string __confirmation  = string(GRN) + "the message was successfully sent\n" + string(RESET);

	if (__ac != 3)
		return (cerr << RED << "Bad command : usage : ./ircserv <port> <password>" << RESET << endl, 1);
	string __port(__av[1]), __password(__av[2]);

	if (parse_arguments(__port, __password) == -1)
		return (cerr << RED << "Bad arguments : please enter a valid arguments" << RESET << endl, 1);
	if ((__socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		cerr << RED << "Socket error : could not creat socket" << RESET << endl;
		exit (1);
	}
	__server_addr.sin_family = AF_INET;
	__server_addr.sin_port = htons(stoi(__port));
	__server_addr.sin_addr.s_addr = INADDR_ANY;
	__address_len = sizeof(__server_addr);
	if (fcntl(__socket_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		cerr << "fcntl error : failed to switch socket descriptor to non-blocking mode " << endl;
		close(__socket_fd);
		exit(1);
	}
	if (bind(__socket_fd, (struct sockaddr *)&__server_addr, sizeof(__server_addr)) == -1)
	{
		cerr << RED << "Bind error : failed to bind socket to port " << __port << RESET << endl;
		close(__socket_fd);
		exit (1);
	}
	if (listen(__socket_fd, MAX_FD) == -1) {
		cerr << RED << "listen error : failed to listen on socket " << __socket_fd << RESET << endl;
		close(__socket_fd);
		exit (1);
	}
	// begin
	for (int i  = 0; i < MAX_FD; i++) {
		__poll_fds[i].fd = -1;
		__poll_fds[i].events = 0;
		__poll_fds[i].events |= POLLIN;
	}
	__poll_fds[0].fd = __socket_fd;
	while(true) 
	{
		__poll_res = poll(__poll_fds, MAX_FD, 0);
		if ( __poll_res  == -1)
		{
			cerr << RED << "poll error : failed to poll on socket " << __socket_fd << RESET << endl;
			full_close(__poll_fds);
			exit (1);
		}
		else if ( __poll_res > 0)
		{
			for (int i = 0; i < MAX_FD; i++) 
			{
				if (__poll_fds[i].revents & POLLIN)
				{
					if (__poll_fds[i].fd == __socket_fd)
					{
						cout << GRN << "New client want to connect !" << RESET << endl;
						__connection = accept(__socket_fd, (struct sockaddr *)&__server_addr, (socklen_t *)&__address_len);
						if (__connection == -1)
						{
							cerr << RED << "accept error : failed to acceot connection on socket " << __socket_fd << RESET << endl;
							break;
						}
						add_to_poll(__poll_fds, __connection);
						if (password_autontification(__password, __connection, __poll_fds) == -1)
							continue ;
					}
					else
					{
						//there are an error the process should not enter here
						__recv_res = recv(__poll_fds[i].fd, __buffer, sizeof(__buffer), 0);
						if (__recv_res == -1)
						{
							cerr << RED << "recv error : failed to receiven request from client " << RESET << endl;
							break;
						}
						if (__recv_res == 0) 
						{
							cerr << RED << "The client " << __poll_fds[i].fd <<  " disconnected !" << RESET << endl;
							close(__poll_fds[i].fd);
							remove_from_poll(__poll_fds, __poll_fds[i].fd);
							// break ;
						}
						//parse the message
						cout << GRN << "➜ " << RESET << __buffer << endl;
						memset(__buffer, 0, sizeof(__buffer));
					}
				}
			}
		}
	}
	return (0);
}
