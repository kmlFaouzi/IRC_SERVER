# include "../includes/main.hpp"

Server::Server()
{
	this->__socket_fd = -1;
	this->__poll_res = 0;
	this->__server_addr.sin_family = AF_INET;
	this->__server_addr.sin_addr.s_addr = INADDR_ANY;
	this->__server_addr.sin_port = htons(this->__port);
	for (int i = 0; i < MAX_FD; i++) {
		__poll_fds[i].fd = -1;
		__poll_fds[i].events = 0;
		__poll_fds[i].events |= POLLIN;
	}
	this->__address_len = sizeof(this->__server_addr);
	this->__port = 3000;
	this->__server_name = "CW9";
}

Server::Server(string password, int port, string name = "CW9")
{
	this->__socket_fd = -1;
	this->__poll_res = 0;
	this->__server_addr.sin_family = AF_INET;
	this->__server_addr.sin_addr.s_addr = INADDR_ANY;
	this->__server_addr.sin_port = htons(port);
	for (int i = 0; i < MAX_FD; i++) {
		__poll_fds[i].fd = -1;
		__poll_fds[i].events = 0;
		__poll_fds[i].events |= POLLIN;
	}
	this->__address_len = sizeof(this->__server_addr);
	this->__password = password;
	this->__port = port;
	this->__server_name = name;
}

void	Server::add_to_poll(struct pollfd *__poll_fds, int __fd)
{
	int i = 0;
	while (i < MAX_FD)
	{
		if (__poll_fds[i].fd == -1)
		{
			__poll_fds[i].fd =  __fd;
			fcntl(__poll_fds[i].fd, F_SETFL, O_NONBLOCK);
			return ;
		}
		i++;
	}
}

void	Server::remove_from_poll(struct pollfd *__poll_fds, int __fd) {
	int i = 0;
	while  (i < MAX_FD)
	{
		if (__poll_fds[i].fd == __fd)
		{
			__poll_fds[i].fd = -1;
			return ;
		}
		i++;
	}
}

void	Server::full_close(struct pollfd *__poll_fd)
{
	for(int i = 0; i < MAX_FD; i++)
		if (__poll_fd[i].fd != -1)
			close(__poll_fd[i].fd);
}
void Server::print(void)
{
	cout << GRN << "	███████████████████████████████████" << endl;
	cout << "	█" <<  RESET << "     Server name ➜ " << GRN<< this->__server_name << "   	  █" <<  endl;
	cout << "	█                                 █" << endl;
	cout << "	█" << RESET <<"     Server port ➜ "<< GRN << this->__port
	<< ( this->__port > 9 && this->__port < 100 ? std::setw(15)
		: this->__port >= 100 && this->__port < 1000 ? std::setw(14)
		: this->__port >= 1000 && this->__port < 10000 ? std::setw(13)
		: this->__port > 10000 ? std::setw(12)
		: std::setw(16)) << "█" << endl;
	cout << "	███████████████████████████████████" << RESET << endl;
}

int	Server::password_authentication(int __client_fd, int index)
{
	int		__bytes = 0, __recv_res = 0;
	char	__buffer[1024] = {0};
	string	__client_password;
	string	__response = string(GRN) + "Connected with the irc server successfully\n" + string(RESET);
	string	__try_password = string(RED) + "Password incorrect please try again.\n" + string(RESET);

	__recv_res = recv(__client_fd, __buffer, sizeof(__buffer), 0);
	if(__recv_res == 0)
		throw Error (" Client " + std::to_string(__client_fd) + " disconnected");
	if (__recv_res > 0)
	{
	__request = __buffer;
		if (__request[__request.size() - 1] == '\n')
		{
			if (!__interpret.empty())
			{
				__interpret += __request;
				__request = __interpret;
			}
			if (__request.substr(0, __request.size() - 1) == this->__password)
			{
				if (send(__client_fd, __response.c_str(), __response.size(), 0) == -1)
					throw Error("send error : could not send response to " + std::to_string(__client_fd));
				if (send(__client_fd, "Create an account\n", 19, 0) == -1)
					throw Error ("send error : could not send response to "+ std::to_string(__client_fd));
				this->__clients[index].set_authentication(true);
				if (write(__client_fd, "Username : ", 12) == -1)
					throw Error ("send error : could not send response");
				__request.clear();
				__interpret.clear();
				return (0);
			}
			else
			{
				if (send(__client_fd, __try_password.c_str(), __try_password.size(), 0) == -1)
					throw Error("send error : could not send response to " + std::to_string(__client_fd));
				if (send(__client_fd, "password ➜ ", 12, 0) == -1)
					throw Error("send error : could not send response to " + std::to_string(__client_fd));
				memset(__buffer, 0, sizeof(__buffer));
				__request.clear();
				__interpret.clear();
			}
		}
		else
			__interpret  = __interpret + __request;
	}
	return (0);
}

void	Server::fill_username(int __client_fd, int index)
{
	int		__recv_res, __parsing_res;
	char	__buffer[1024] = {};

	if (this->__clients[index].__username_filled == false)
	{
		// if (write(__client_fd, "Username : ", 12) == -1)
		// 	throw Error ("send error : could not send response");
		__recv_res = recv(__client_fd, __buffer, sizeof(__buffer), 0);
		if (__recv_res == 0)
			throw Error(" Client " + std::to_string(__client_fd) + " disconnected");
		if (__recv_res > 0)
		{
			__parsing_res = this->parse_input(string(__buffer), 1);
			// std::cout << __parsing_res << std::endl;
			// __parsing_res == 0 	? throw Error ("Invalid username")
			// : __parsing_res == -1 ? throw Error ("Username already exist")
			// : this->__clients[index].set_username(string(__buffer));
			if (__parsing_res == 0){
				if (write(__client_fd, "Username : ", 12) == -1)
					throw Error ("send error : could not send response");
				throw Error("Invalid username");
			}
			this->__clients[index].__username_filled  = true;
			this->__clients[index].set_username(string(__buffer));
			this->__clients[index].set_is_registred(true);
			return ;
		}
	}
}

void    Server::fill_nickname(int __client_fd, int index)
{
	int		__recv_res, __parsing_res;
	char	__buffer[1024] = {};

	if (this->__clients[index].__nickname_filled == false)
	{
		cout << "the code enter to fill nickname function " << endl;
		if (send(__client_fd, "Nickname : ", 12, 0) == -1)
			throw Error ("send error : could not send response");
		__recv_res = recv(__client_fd, __buffer, sizeof(__buffer), 0);
		printf("[%d]\n", __recv_res);
		if (__recv_res == 0)
			throw Error(" Client " + std::to_string(__client_fd) + " disconnected");
		else if (__recv_res > 0)
		{
			__parsing_res = this->parse_input(string(__buffer), 2);
			std::cout << __parsing_res << std::endl;
			__parsing_res == 0 	? throw Error ("Invalid nickname")
			: __parsing_res == -1 ? throw Error ("Nickname already exist") // if nickname failed shoudl I remove the client
			: this->__clients[index].set_nickname(string(__buffer));
		}
	}
}

void	Server::fill_operator(int __client_fd, int index)
{
	int		__recv_res, __parsing_res;
	char	__buffer[1024] = {};

	if (send(__client_fd, "Operator : ", 12, 0) == -1)
		throw Error ("send error : could not send response");
	__recv_res = recv(__client_fd, __buffer, sizeof(__buffer),0);
	if (__recv_res == 0)
		throw Error(" Client " + std::to_string(__client_fd) + " disconnected");
	else if (__recv_res > 0)
	{
		__parsing_res = this->parse_input(string(__buffer), 3);
		__parsing_res == -1	? throw Error ("Invalid input")
		: __parsing_res == 0 ? this->__clients[index].set_is_operator(true)
		: this->__clients[index].set_is_operator(false);
	}
}

void	Server::create_server(void)
{
	 int n = 1;
	if ((this->__poll_fds[0].fd = this->__socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		throw Error("Socket error : could not open socket");
	if (setsockopt(this->__socket_fd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n)) == -1) // to bind multiple times to the same port.
		throw Error("setsockopt error : could not set SO_REUSEADDR");
	if (fcntl(this->__socket_fd, F_SETFL, O_NONBLOCK) == -1)
		throw Error("fcntl error : could not set socket in non-blocking mode");
	if (bind(this->__socket_fd, (struct sockaddr *)&this->__server_addr, sizeof(this->__server_addr)) == -1)
		throw Error("bind error : could not bind socket");
	if (listen(this->__socket_fd, MAX_FD) == -1)
		throw Error("listen error : could not listen on socket");
}

void	Server::run()
{
	int				__connection;
	int				__recv_res;
	char			__buffer[MAX_FD];
	string	__response = string(GRN) + "Connected with the irc server successfully\n" + string(RESET);
	string	__try_password = string(RED) + "Password incorrect please try again.\n" + string(RESET);
	string	__request;
	string	__interpret;

	this->print();
	while (true)
	{
		this->__poll_res = poll(this->__poll_fds, MAX_FD, 0);
		if (__poll_res == -1)
			throw Error("poll error : failed to poll on socket " + std::to_string(this->__socket_fd));
		else if (this->__poll_res > 0)
		{
			for (int i = 0; i < MAX_FD; i++)
			{
				if (this->__poll_fds[i].revents & POLLIN)
				{
					if (this->__poll_fds[i].fd == this->__socket_fd)
					{
						cout << GRN << "New request from " << this->__poll_fds[i].fd << RESET << endl;
						__connection = accept(this->__socket_fd, (struct sockaddr *)&this->__server_addr, (socklen_t *)&this->__address_len);
						if (__connection == -1)
                        {
							cerr << RED << "accept error : could not accept clinet at socket " << this->__socket_fd <<  RESET << endl;
							break ;
						}
						add_to_poll(this->__poll_fds, __connection);
						this->__clients.push_back(Client(__connection));
						if (send(__connection, "password ➜ ", 12, 0) == -1)
								throw Error("send error : could not send response to " + std::to_string(__connection));
					}
					else
					{
						size_t j = 0;
						for (; j < this->__clients.size(); j++)
						{
							if (this->__clients[j].get_fd() == this->__poll_fds[i].fd)
								break ;
						}
						if (!this->__clients[j].is_authenticate())
						{
							try
							{
								this->password_authentication(this->__clients[j].get_fd(), j);

							}
							catch(const std::exception& e)
							{
								std::cerr << e.what() << '\n';
								cout << RED << "Clinet " << this->__clients[j].get_fd() << " Disconnected" << RESET << std::endl;
								remove_from_poll(__poll_fds, __poll_fds[i].fd);
								this->__clients.erase(this->__clients.begin() + j);
								close(__poll_fds[i].fd);
								close(this->__clients[j].get_fd());
								break ;
							}
						}
						if (!this->__clients[j].is_registred())
						{
							try
							{
								cout << "Im here " << endl;
								cout << (this->__clients[j].__username_filled ? "treu" : "false") << endl;
								if (this->__clients[j].__username_filled == false)
								{
									this->fill_username(this->__clients[j].get_fd(), j);
								}
								// else if (this->__clients[j].__nickname_filled == false)
								// 	this->fill_nickname(this->__clients[j].get_fd(), j);
								// if (this->__clients[j].__username_filled == true && this->__clients[j].__nickname_filled == true)
								// 	this->__clients[j].set_is_registred(true);
								// this->fill_operator(this->__clients[j].get_fd(), j);
								// this->__clients[j].set_is_registred(true);
							}
							catch(const std::exception& e)
							{
								std::cerr << e.what() << '\n';
							}
							cout << "The usernme is " << this->__clients[j].get_username() << endl;
 						}
						// else if (this->__clients[j].is_registred())
						// {
						// 	__recv_res = recv(__poll_fds[i].fd, __buffer, sizeof(__buffer), 0);
						// 	if (__recv_res == 0)
						// 	{
						// 		cerr << RED << "The client " << __poll_fds[i].fd <<  " disconnected !" << RESET << endl;
						// 		close(__poll_fds[i].fd);
						// 		remove_from_poll(__poll_fds, __poll_fds[i].fd);
						// 		this->__clients.erase(this->__clients.begin() + j);
						// 		break ;
						// 	}
						// 	cout << GRN << "➜ " << RESET << __buffer << endl;
						// 	memset(__buffer, 0, sizeof(__buffer));
						// }
					}
				}
			}
		}
	}
}
