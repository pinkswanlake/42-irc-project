/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smuneer <smuneer@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/28 09:56:46 by smuneer           #+#    #+#             */
/*   Updated: 2025/11/28 09:56:47 by smuneer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/replies.hpp"
#include "../inc/Channel.hpp"

void Server::handleKick(std::vector<std::string> &args, int fd)
{
    Client* kicker = GetClient(fd);
    if (!kicker || !kicker->isRegistered())
        return;

    // --- Check argument count ---
    if (args.size() < 2)
    {
        _sendResponse(ERR_NEEDMOREPARAMS(serverName, "KICK"), fd);
        return;
    }

    std::string channelName = args[0];
    std::string targetNick = args[1];

    // --- Construct reason string (multi-word support) ---
    std::string reason = "Kicked";
    if (args.size() >= 3)
    {
        reason = args[2];
        if (reason[0] == ':')
            reason = reason.substr(1);
        for (size_t i = 3; i < args.size(); i++)
            reason += " " + args[i];
    }
    

    // --- Find channel ---
    Channel* ch = GetChannel(channelName);
    if (!ch)
    {
        _sendResponse(ERR_NOSUCHCHANNEL(serverName, kicker->getNickname(), channelName), fd);
        return;
    }

    if (!ch->hasMember(kicker))
    {
        _sendResponse(ERR_NOTONCHANNEL(serverName, kicker->getNickname(), channelName), fd);
        return;
    }


    // --- Check if kicker is operator ---
    if (!ch->isOperator(kicker))
    {
        _sendResponse((":" + serverName + " 482 " + kicker->getNickname() + " " + channelName + " :You're not channel operator\r\n"),fd);
        return;
    }

    // --- Find target client ---
    Client* target = GetClientNick(targetNick);
    if (!target)
    {
        _sendResponse((":" + serverName + " 401 " + kicker->getNickname() + " " + targetNick + " :No such nick\r\n"), fd);
        return;
    }

    // --- Check if target is in the channel ---
    if (!ch->hasMember(target))
    {
        _sendResponse((":" + serverName + " 441 " + kicker->getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n"), fd);
        return;
    }

    // --- Construct KICK message ---how may i domnstrate my part of irc channel and clinet to the evaluator, give me a sensible understandtable , or even make digital, i have 
    std::string msg = ":" + kicker->getPrefix() +
                      " KICK " + channelName + " " + target->getNickname() + " :" + reason + "\r\n";

    // --- Broadcast to all members (including target) ---
    ch->broadcast(msg, NULL);

    // --- Remove target from channel AFTER broadcast ---
    ch->removeMember(target);
}


void Server::handlePrivMsg(std::vector<std::string> &args, int fd)
{
    Client* cli = GetClient(fd);
    if (!cli)
        return;

    // === ERR_NORECIPIENT ===
    if (args.size() < 1)
    {
        _sendResponse(":" + serverName + " 411 " +
                      cli->getNickname() +
                      " :No recipient given (PRIVMSG)\r\n", fd);
        return;
    }

    // === ERR_NOTEXTTOSEND ===
    if (args.size() < 2)
    {
        _sendResponse(":" + serverName + " 412 " +
                      cli->getNickname() +
                      " :No text to send\r\n", fd);
        return;
    }

    std::string target = args[0];

    // ---- rebuild message (everything after :) ----
    std::string message = args[1];
    if (message[0] == ':')
        message = message.substr(1);

    for (size_t i = 2; i < args.size(); i++)
        message += " " + args[i];

    // ================= CHANNEL MESSAGE =================
    if (target[0] == '#' || target[0] == '&')
    {
        Channel* ch = NULL;
        for (size_t i = 0; i < channels.size(); ++i)
        {
            if (channels[i].getName() == target)
            {
                ch = &channels[i];
                break;
            }
        }

        if (!ch)
        {
            _sendResponse((":" + serverName + " 403 " + cli->getNickname() + " " + target + " :No such channel\r\n"), fd);
            return;
        }

        if (!ch->hasMember(cli))
        {
            _sendResponse(":" + serverName + " 442 " +
                          cli->getNickname() + " " + target +
                          " :You're not on that channel\r\n", fd);
            return;
        }

        std::string msg =
            ":" + cli->getPrefix() +
            " PRIVMSG " + target +
            " :" + message + "\r\n";

        // Broadcast to everyone except sender
        ch->broadcast(msg, cli);
    }

    // ================= USER MESSAGE =================
    else
    {
        Client* tar = GetClientNick(target);
        if (!tar)
        {
            _sendResponse(":" + serverName + " 401 " +
                          cli->getNickname() + " " + target +
                          " :No such nick\r\n", fd);
            return;
        }

        std::string msg =
            ":" + cli->getPrefix() +
            " PRIVMSG " + target +
            " :" + message + "\r\n";

        _sendResponse(msg, tar->getFd());
    }
}

void Server::handleNotice(std::vector<std::string> &args, int fd)
{
    Client* cli = GetClient(fd);
    if (!cli || args.size() < 2)
        return; // NO ERRORS for NOTICE

    std::string target = args[0];

    std::string message = args[1];
    if (!message.empty() && message[0] == ':')
        message.erase(0, 1);

    for (size_t i = 2; i < args.size(); i++)
        message += " " + args[i];

    // ===== CHANNEL NOTICE =====
    if (target[0] == '#' || target[0] == '&')
    {
        Channel* ch = NULL;
        for (size_t i = 0; i < channels.size(); ++i)
        {
            if (channels[i].getName() == target)
            {
                ch = &channels[i];
                break;
            }
        }

        if (!ch || !ch->hasMember(cli))
            return; // SILENT

        std::string msg = ":" + cli->getPrefix() +
                          " NOTICE " + target +
                          " :" + message + "\r\n";

        ch->broadcast(msg, cli);
    }
    // ===== USER NOTICE =====
    else
    {
        Client* targetClient = GetClientNick(target);
        if (!targetClient)
            return; // SILENT

        std::string msg = ":" + cli->getPrefix() +
                          " NOTICE " + target +
                          " :" + message + "\r\n";

        targetClient->sendMessage(msg);
    }
}
