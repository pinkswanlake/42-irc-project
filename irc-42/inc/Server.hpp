/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smuneer <smuneer@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/28 09:56:28 by smuneer           #+#    #+#             */
/*   Updated: 2025/11/28 09:56:29 by smuneer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <poll.h>
#include <algorithm>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include "Client.hpp"
#include "Channel.hpp"
#include <limits.h>

#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"

class Client;
class Server
{
private:
	int port;
	int server_fd;
	std::string password;
	std::string serverName;
	static bool Serversignal;
	std::vector<struct pollfd> poll_fds;
	std::map<int, Client*> clients;
	std::vector<Channel> channels;
	struct sockaddr_in address;
	void errorExit(const std::string &msg);

public:
//check if cannonical form needed
	Server(int port, const std::string &password);
	~Server();

	void initSocket();
	void non_block(int server_fd);
	void run();

	// signal + cleanup
	static void signalHandler(int signum);
	void closeAll();
	void broadcastToAll(const std::string &message);
	// client mgmt
	void acceptNewClient();
	void receiveClientData(int fd);
	void removeClient(int fd);

	void _sendResponse(std::string response, int fd);
	bool is_validNickname(std::string& nickname);
	bool nickNameInUse(std::string& nickname);
	// command handling
	void parseCommand(std::string &line, int fd);
	Client* GetClient(int fd);
	//Channel *GetChannel(std::string name);
	void handleTopic(std::string &fullLine, int fd);
	void handlePrivMsg(std::vector<std::string> &args, int fd);
	Client* GetClientNick(const std::string& nickname);
	Channel* GetChannel(const std::string& name);
	void AddClient(Client *newClient);
	void AddChannel(Channel newChannel);
	void AddFds(pollfd newFd);

	void handlePass(std::vector<std::string> &args, int fd);
	void handleNick(std::vector<std::string> &args, int fd);
	void handleUser(std::vector<std::string> &args, int fd);
	void handleQuit(std::vector<std::string> &args, int fd);
	void handleJoin(std::vector<std::string> &args, int fd);
	void handleInvite(std::vector<std::string> &args, int fd);
	void handleTopic(std::vector<std::string> &args, int fd);
	void handleKick(std::vector<std::string> &args, int fd);
	void handleMode(std::vector<std::string> &args, int fd);
	void handleNotice(std::vector<std::string> &args, int fd);

	const std::string &getPassword() const;
};

#endif
