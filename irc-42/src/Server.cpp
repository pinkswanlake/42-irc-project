/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smuneer <smuneer@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/28 09:56:55 by smuneer           #+#    #+#             */
/*   Updated: 2025/11/28 10:14:33 by smuneer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/replies.hpp"

bool Server::Serversignal = false;

Server::Server(int port, const std::string &password)
    : port(port), server_fd(-1), password(password), serverName("ircserv") {}

Server::~Server()
{
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        delete it->second;
    clients.clear();

    if (server_fd != -1)
        close(server_fd);

    std::cout << "[Server] Closed successfully" << std::endl;
}


void Server::removeClient(int fd)
{
    std::map<int, Client*>::iterator it = clients.find(fd);
    if (it == clients.end()) return;

    Client* client = it->second;

    std::cout << "[Server] Client is disconnected (fd = " << fd << ")" << std::endl;

    // ✅ FIX: Get copy of channels BEFORE removing
    std::vector<Channel*> userChannels = client->getChannels();
    
    // Remove client from all channels
    for (size_t i = 0; i < userChannels.size(); ++i)
    {
        userChannels[i]->removeMember(client);
    }

    close(fd);

    // Remove from poll_fds
    for (size_t i = 0; i < poll_fds.size(); ++i)
    {
        if (poll_fds[i].fd == fd)
        {
            poll_fds.erase(poll_fds.begin() + i);
            break;
        }
    }

    // Delete client and remove from map
    delete client;
    clients.erase(it);
}

void Server::errorExit(const std::string &msg)
{
    std::cerr << "[Error] " << msg << std::endl;
    if (server_fd != -1)
        close(server_fd);
    std::exit(EXIT_FAILURE);
}

void Server::signalHandler(int signum)
{
    (void)signum;
    std::cout << std::endl << "Signal Received!" << std::endl;
    Server::Serversignal = true;
}


void Server::non_block(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        errorExit("fcntl() failed");
}


void Server::initSocket()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        errorExit("socket() failed");

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        errorExit("setsockopt() failed");

    non_block(server_fd);

    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
        errorExit("bind() failed");

    if (listen(server_fd, 20) == -1)
        errorExit("listen() failed");

    struct pollfd pd;
    pd.fd = server_fd;
    pd.events = POLLIN;
    pd.revents = 0;
    poll_fds.push_back(pd);

    std::cout << "[Server] Listening on port " << port << std::endl;
}

void Server::run()
{
    while (Server::Serversignal == false)
    {
        if ((poll(&poll_fds[0], poll_fds.size(), -1) == -1) && Server::Serversignal == false)
            errorExit("poll() failed");

        for (int i = poll_fds.size() - 1; i >= 0; --i)
        {
            int fd = poll_fds[i].fd;

            if (poll_fds[i].revents & POLLIN)
            {
                if (fd == server_fd)
                    acceptNewClient();
                else
                    receiveClientData(fd);
            }
            else if (poll_fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
            {
                removeClient(fd);
            }
        }
    }
}

void Server::acceptNewClient()
{
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);

    int clfd = accept(server_fd, (sockaddr *)&cliaddr, &len);
    if (clfd == -1)
    {
        std::cout << "accept() failed" << std::endl;
        return;
    }

    non_block(clfd);

    struct pollfd pd;
    pd.fd = clfd;
    pd.events = POLLIN;
    pd.revents = 0;
    poll_fds.push_back(pd);

    Client *c = new Client(clfd);
    AddClient(c);

    std::cout << "[Server] Client <" << clfd << "> Connected from "
              << inet_ntoa(cliaddr.sin_addr) << ":" << ntohs(cliaddr.sin_port)
              << std::endl;
}

void Server::receiveClientData(int fd)
{
    char buff[1024];
    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);

    if (bytes <= 0)
    {
        removeClient(fd);
        return;
    }

    buff[bytes] = '\0';

    // Safe client lookup
    std::map<int, Client*>::iterator it = clients.find(fd);
    if (it == clients.end())
    {
        removeClient(fd);
        return;
    }

    Client* client = it->second;
    client->appendToBuffer(buff);

    while (client->hasCompleteMessage())
    {
        std::string msg = client->extractMessage();
        std::cout << "[Server] Received from fd=" << fd << ": " << msg << std::endl;

        parseCommand(msg, fd);
    }
}

Client* Server::GetClient(int fd)
{
    std::map<int, Client*>::iterator it = clients.find(fd);
    if (it != clients.end())
        return it->second;
    return NULL;
}

void Server::AddClient(Client* newClient)
{
    if (newClient)
        clients[newClient->getFd()] = newClient;
}

void Server::AddChannel(Channel newChannel)
{
    channels.push_back(newChannel);
}

void Server::AddFds(pollfd newFd)
{
    poll_fds.push_back(newFd);
}

Client* Server::GetClientNick(const std::string& nickname)
{
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second && it->second->getNickname() == nickname)
            return it->second;
    }
    return NULL;
}

void Server::broadcastToAll(const std::string &message)
{
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        Client* client = it->second;
        if (client && client->isRegistered())  // only send to registered clients
        {
            client->sendMessage(message);
        }
    }
}

Channel* Server::GetChannel(const std::string& name)
{
    for (size_t i = 0; i < channels.size(); ++i)
    {
        if (channels[i].getName() == name)
            return &channels[i];
    }
    return NULL;
}

void Server::parseCommand(std::string &line, int fd)
{
    Client* cli = GetClient(fd);
    if (!cli)
        return;
    if (line.empty())
        return;

    // Trim space
    size_t start = line.find_first_not_of(" \t\r\n");
    size_t end   = line.find_last_not_of(" \t\r\n");

    if (start == std::string::npos)
        return;

    line = line.substr(start, end - start + 1);

    // Split command + args
    // Split command + args (RFC compliant)
size_t spacePos = line.find(' ');
std::string command;
std::vector<std::string> args;

if (spacePos != std::string::npos)
{
    command = line.substr(0, spacePos);
    std::string rest = line.substr(spacePos + 1);

    while (!rest.empty())
    {
        if (rest[0] == ':')
        {
            args.push_back(rest.substr(1));
            break;
        }

        size_t pos = rest.find(' ');
        if (pos == std::string::npos)
        {
            args.push_back(rest);
            break;
        }

        args.push_back(rest.substr(0, pos));
        rest.erase(0, pos + 1);
        rest.erase(0, rest.find_first_not_of(' '));
    }
}
else
{
    command = line;
}


    // Uppercase command
    for (size_t i = 0; i < command.size(); ++i)
        command[i] = toupper(command[i]);

    // Registration
    if (command == "PASS") handlePass(args, fd);
    else if (command == "NICK") handleNick(args, fd);
    else if (command == "USER") handleUser(args, fd);
    else if (command == "QUIT") handleQuit(args, fd);
    else if (!cli->isRegistered())
    {
        if (command != "PASS" && command != "NICK" && command != "USER")
        {
            _sendResponse(ERR_NOTREGISTERED(serverName, "*"), fd);
        }
    return;
    }
    else
    {
    // Now the client is fully registered.
    // Add all future command handlers here.

        if (command == "JOIN")
            handleJoin(args, fd);
        else if (command == "INVITE")
            handleInvite(args, fd);
        else if (command == "TOPIC")
            handleTopic(args, fd);
        else if (command == "KICK")
            handleKick(args, fd);
        else if (command == "MODE")
            handleMode(args, fd);
        else if (command == "PRIVMSG")
            handlePrivMsg(args, fd);
        else if (command == "NOTICE")
            handleNotice(args, fd);


    // If unknown command:
        else
             _sendResponse(ERR_UNKNOWNCOMMAND(serverName, GetClient(fd)->getNickname(), command), fd);
}
}
