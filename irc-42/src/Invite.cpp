/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smuneer <smuneer@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/28 09:56:42 by smuneer           #+#    #+#             */
/*   Updated: 2025/11/28 10:17:10 by smuneer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/replies.hpp"


void Server::handleJoin(std::vector<std::string> &args, int fd)
{
    Client* cli = GetClient(fd);
    if (!cli)
        return;

    if (args.empty())
    {
        _sendResponse(ERR_NEEDMOREPARAMS(serverName, cli->getNickname()), fd);
        return;
    }

    // Split channels and keys
    std::vector<std::string> channelNames;
    std::vector<std::string> keys;

    // Channels list
    size_t commaPos = 0, start = 0;
    std::string channelsArg = args[0];
    while ((commaPos = channelsArg.find(',', start)) != std::string::npos)
    {
        channelNames.push_back(channelsArg.substr(start, commaPos - start));
        start = commaPos + 1;
    }
    channelNames.push_back(channelsArg.substr(start));

    // Keys list (optional)
    if (args.size() > 1)
    {
        std::string keysArg = args[1];
        start = 0;
        while ((commaPos = keysArg.find(',', start)) != std::string::npos)
        {
            keys.push_back(keysArg.substr(start, commaPos - start));
            start = commaPos + 1;
        }
        keys.push_back(keysArg.substr(start));
    }

    for (size_t i = 0; i < channelNames.size(); i++)
    {
        std::string channelName = channelNames[i];
        std::string key = (i < keys.size()) ? keys[i] : "";

        // Validate channel name
        if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&'))
        {
            _sendResponse(ERR_NOSUCHCHANNEL(serverName, cli->getNickname(), channelName), fd);
            continue;
        }

        // Find or create channel
        Channel* channel = GetChannel(channelName);
        if (!channel)
        {
            AddChannel(Channel(channelName));
            channel = &channels.back();
        }

        // Already in channel
        if (channel->hasMember(cli))
        {
            _sendResponse(ERR_USERNOTINCHANNEL(serverName, cli->getNickname(), channelName), fd);
            continue;
        }

        // Invite-only
        if (channel->isInviteOnly() && !channel->isInvited(cli))
        {
            _sendResponse(ERR_INVITEONLYCHAN(cli->getNickname(), channelName), fd);
            continue;
        }

        // Channel full
        if (channel->isFull())
        {
            _sendResponse(ERR_CHANNELISFULL(cli->getNickname(), channelName), fd);
            continue;
        }

        // Bad key
        if (!channel->checkKey(key))
        {
            _sendResponse(ERR_BADCHANNELKEY(cli->getNickname(), channelName), fd);
            continue;
        }

        // ---- JOIN SUCCESS ----
        channel->addMember(cli);
        std::string joinMsg = ":" + cli->getPrefix() + " JOIN :" + channelName + CRLF;

        // Send to self
        _sendResponse(joinMsg, fd);

        // Broadcast to others
        channel->broadcast(joinMsg, cli);

        // ---- TOPIC ----
        if (channel->getTopic().empty())
            _sendResponse(":" + serverName + " 331 " + cli->getNickname() +
                          " " + channelName + " :No topic is set\r\n", fd);
        else
            _sendResponse(":" + serverName + " 332 " + cli->getNickname() +
                          " " + channelName + " :" + channel->getTopic() + "\r\n", fd);

        // ---- NAMES (353) ----
        std::string names;
        std::vector<Client*> members = channel->getMembers();
        for (size_t j = 0; j < members.size(); j++)
        {
            if (channel->isOperator(members[j]))
                names += "@";
            names += members[j]->getNickname() + " ";
        }
        _sendResponse(":" + serverName + " 353 " + cli->getNickname() +
                      " = " + channelName + " :" + names + "\r\n", fd);

        _sendResponse(":" + serverName + " 366 " + cli->getNickname() +
                      " " + channelName + " :End of /NAMES list\r\n", fd);
    }
}


void Server::handleInvite(std::vector<std::string> &args, int fd)
{
    Client *inviter = GetClient(fd);
    if (!inviter)
        return;

    // === ERR_NEEDMOREPARAMS (461) ===
    if (args.size() < 2)
    {
        _sendResponse(":" + serverName +" 461 " + inviter->getNickname() + " INVITE :Not enough parameters\r\n", fd);
        return;
    }

    std::string targetNick = args[0];
    if (!targetNick.empty() && targetNick[0] == ':')
        targetNick.erase(0, 1);
    std::string channelName = args[1];

       // === Find target user ===
    Client *target = GetClientNick(targetNick);
    if (!target)
    {
        _sendResponse(":" + serverName +" 401 " + inviter->getNickname() + " " + targetNick + " :No such nick\r\n", fd);
        return;
    }

    // === Validate channel format ===
    if (channelName.empty() || channelName[0] != '#')
    { 
         _sendResponse(":" + serverName + " 403 " + inviter->getNickname() + " " + channelName + " :No such channel\r\n", fd);
         return ;

    }

    // === Find channel ===
    Channel *channel = GetChannel(channelName);
    if (!channel)
    {
         _sendResponse((":" + serverName + " 403"+inviter->getNickname() + " " + channelName + " :No such channel\r\n"), fd);
        return;
    }

    // === Check inviter is in channel ===
    if (!channel->hasMember(inviter))
    {
        _sendResponse(":" + serverName +" 442 " + inviter->getNickname() + " " + channelName + " :You're not on that channel\r\n", fd);
        return;
    }

 

    // === Check if already in channel ===
    if (channel->hasMember(target))
    {
        _sendResponse(":" + serverName +" 443 " + target->getNickname() + " " + channelName + " :is already on channel\r\n", fd);
        return;
    }

    // === Check invite-only mode ===
    if (channel->isInviteOnly() && !channel->isOperator(inviter))
    {
        _sendResponse(":" + serverName +" 482 " + inviter->getNickname() + " " + channelName + " :You're not channel operator\r\n", fd);
        return;
    }

    // === Check user limit ===
    if (channel->isFull())
    {
        _sendResponse(":" + serverName + " 471 " + channelName + " :Cannot join channel (+l)\r\n", fd);
        return;
    }

    // === Add target to invite list ===
    channel->addToInviteList(target);

    // === Reply to inviter (RPL_INVITING = 341) ===
    std::string rep1 = ":" + serverName +" 341 " + inviter->getNickname() + " " +
                       target->getNickname() + " " + channelName + "\r\n";
    _sendResponse(rep1, fd);

    // === Notify the invited user ===
   std::string rep2 = inviter->getPrefix() + " INVITE " + target->getNickname() + " :" + channelName + "\r\n";


    _sendResponse(rep2, target->getFd());
}

