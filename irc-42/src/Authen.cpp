/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Authen.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smuneer <smuneer@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/28 09:56:34 by smuneer           #+#    #+#             */
/*   Updated: 2025/11/28 10:07:11 by smuneer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/replies.hpp"


void Server::_sendResponse(std::string response, int fd)
{
	if(send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() faild" << std::endl;
}

void Server::handlePass(std::vector<std::string> &args, int fd)
{
    Client *cli = GetClient(fd);
    if (!cli)
        return;

    if (args.empty())
    {
        _sendResponse(ERR_NEEDMOREPARAMS(serverName, "PASS"), fd);
        return;
    }

    if (cli->isRegistered())
    {
        _sendResponse(ERR_ALREADYREGISTERED(serverName), fd);
        return;
    }
    std::string pass = args[0];

    if (pass == password)
    {
        cli->setPassword(true);
        std::cout << "Client " << fd << " provided correct PASS\n";
    }
    else
    {
        _sendResponse(ERR_PASSWDMISMATCH(serverName), fd);
    }
    if (cli->getisAuthenticated() && !cli->getNickname().empty() && !cli->getUsername().empty())
    {
        cli->checkRegistration();
        if (cli->isRegistered())
        {
            _sendResponse(RPL_WELCOME(serverName, cli->getNickname()), fd);
        }
    }
}



bool Server::is_validNickname(std::string& nickname)
{
    if (nickname.empty() || nickname.length() > 9)
        return false;

    if (!std::isalpha(nickname[0]))
        return false;

    if (nickname[0] == '#' || nickname[0] == '&' || nickname[0] == ':')
        return false;

    // Remaining chars must be alnum or _
    for (size_t i = 1; i < nickname.size(); i++)
    {
        if (!std::isalnum(nickname[i]) && nickname[i] != '_' && nickname[i] != '-')
            return false;
    }
    return true;
}



bool Server::nickNameInUse(std::string& nickname)
{
    std::string nickLower = nickname;
    std::transform(nickLower.begin(), nickLower.end(), nickLower.begin(), ::tolower);

    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (!it->second)
            continue;

        std::string exist = it->second->getNickname();
        std::transform(exist.begin(), exist.end(), exist.begin(), ::tolower);

        if (exist == nickLower)
            return true;
    }
    return false;
}


void Server::handleNick(std::vector<std::string> &args, int fd)
{
    Client *cli = GetClient(fd);
    if (!cli)
        return;

    if (args.empty())
    {
        _sendResponse(ERR_NONICKNAME(serverName), fd);
        return;
    }

    std::string newNick = args[0];

    if (!is_validNickname(newNick))
    {
        _sendResponse(ERR_ERRONEUSNICK(serverName,newNick), fd);
        return;
    }

    // Case-insensitive collision check
    if (nickNameInUse(newNick))
    {
        std::string old = cli->getNickname();
        std::string lowerOld = old; 
        std::string lowerNew = newNick;
        std::transform(lowerOld.begin(), lowerOld.end(), lowerOld.begin(), ::tolower);
        std::transform(lowerNew.begin(), lowerNew.end(), lowerNew.begin(), ::tolower);

        if (lowerOld != lowerNew)
        {
            _sendResponse(ERR_NICKINUSE(serverName, newNick), fd);
            return;
        }
    }

    std::string oldNick = cli->getNickname();
    cli->setNickname(newNick);
    cli->checkRegistration();


    // --- BROADCAST nickname change ---
    if (!oldNick.empty() && oldNick != newNick)
{
    std::string msg =
        ":" + oldNick + "!" + cli->getUsername() + "@" +
        serverName + " NICK :" + newNick + "\r\n";

    // 1) Send to the client themselves
    _sendResponse(msg, fd);

    // 2) Send to users in the same channels
    std::vector<Channel*> chans = cli->getChannels();
    for (size_t i = 0; i < chans.size(); ++i)
    {
        chans[i]->broadcast(msg, cli);
    }
}
     if (cli->getisAuthenticated() && !cli->getUsername().empty() && oldNick.empty())
    {
        if (cli->isRegistered())
        {
            _sendResponse(RPL_WELCOME(serverName, cli->getNickname()), fd);

        }
    }
}

void Server::handleUser(std::vector<std::string> &args, int fd)
{
    Client *cli = GetClient(fd);
    if (!cli)
        return;

    // USER <username> <mode> <unused> :<realname>
    if (args.size() < 4)
    {
        _sendResponse(ERR_NEEDMOREPARAMS(serverName, "USER"), fd);
        return;
    }

    if (cli->isRegistered())
    {
        _sendResponse(ERR_ALREADYREGISTERED(serverName), fd);
        return;
    }

    std::string username = args[0];
    std::string realname = args[3]; // already parsed correctly

    cli->setUsername(username, realname);

    // Registration complete?
    if (cli->getisAuthenticated() && !cli->getNickname().empty())
    {
        cli->checkRegistration();
        if (cli->isRegistered())
        {
            _sendResponse(RPL_WELCOME(serverName, cli->getNickname()), fd);
        }
    }
}



void Server::handleQuit(std::vector<std::string> &args, int fd)
{
    Client *client = GetClient(fd);
    if (!client)
        return;

    // ---- Build quit reason ----
    std::string reason = "Client Quit";
    if (!args.empty())
    {
        reason.clear();
        for (size_t i = 0; i < args.size(); i++)
        {
            if (i > 0)
                reason += " ";
            reason += args[i];
        }
        if (!reason.empty() && reason[0] == ':')
            reason.erase(0, 1);
    }

    // ---- Correct QUIT message (FROM CLIENT) ----
    std::string quitMsg =
        client->getPrefix() +
        " QUIT :" + reason + "\r\n";

    std::cout << "[QUIT] " << quitMsg;

    // ---- Broadcast to all channels client is in ----
    const std::vector<Channel*> &userChannels = client->getChannels();
    for (size_t i = 0; i < userChannels.size(); i++)
    {
        userChannels[i]->broadcast(quitMsg, client);
        userChannels[i]->removeMember(client);
    }

    // ---- Remove client from server ----
    removeClient(fd);
}
