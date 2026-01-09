/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smuneer <smuneer@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/28 09:56:52 by smuneer           #+#    #+#             */
/*   Updated: 2025/11/28 09:56:53 by smuneer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/replies.hpp"
#include "../inc/Channel.hpp"
#include <climits>

void Server::handleMode(std::vector<std::string> &args, int fd)
{
    Client* cli = GetClient(fd);
    if (!cli)
        return;

    if (args.empty())
    {
        _sendResponse(ERR_NEEDMOREPARAMS(serverName, "MODE"), fd);
        return;
    }

    std::string target = args[0];

    // ===== CHANNEL MODE ONLY =====
    if (target[0] != '#')
        return;

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
        _sendResponse((":"+serverName+" 403 "+ cli->getNickname() + " " + target +  " :No such channel!\r\n"), fd);
        return;
    }

    if (!ch->hasMember(cli))
    {
        _sendResponse(":" + serverName + " 442 " +
                      cli->getNickname() + " " + target +
                      " :You're not on that channel\r\n", fd);
        return;
    }

    // ===== QUERY MODES =====
    if (args.size() == 1)
    {
        std::string modes;
        if (ch->isInviteOnly()) modes += "i";
        if (ch->isTopicRestricted()) modes += "t";
        if (ch->hasKey()) modes += "k";
        if (ch->hasUserLimit()) modes += "l";

        _sendResponse(":" + serverName + " 324 " +
                      cli->getNickname() + " " +
                      ch->getName() + " +" + modes + "\r\n", fd);
        return;
    }

    // ===== OPERATOR CHECK =====
    if (!ch->isOperator(cli))
    {
        _sendResponse(":" + serverName + " 482 " +
                      cli->getNickname() + " " + target +
                      " :You're not channel operator\r\n", fd);
        return;
    }

    std::string modes = args[1];
    std::vector<std::string> params(args.begin() + 2, args.end());

    bool add = true;
    char sign = '+';
    size_t paramIndex = 0;

    std::string appliedModes;
    std::vector<std::string> appliedParams;

    for (size_t i = 0; i < modes.size(); ++i)
    {
        char c = modes[i];

        if (c == '+') { add = true; sign = '+'; continue; }
        if (c == '-') { add = false; sign = '-'; continue; }

        switch (c)
        {
            case 'i':
                ch->setInviteOnly(add);
                appliedModes += c;
                break;

            case 't':
                ch->setTopicRestricted(add);
                appliedModes += c;
                break;

            case 'k':
                if (add)
                {
                    if (paramIndex >= params.size())
                    {
                        _sendResponse(ERR_NEEDMOREPARAMS(serverName, "MODE"), fd);
                        return;
                    }
                    ch->setKey(params[paramIndex]);
                    appliedParams.push_back(params[paramIndex++]);
                }
                else
                    ch->removeKey();

                appliedModes += c;
                break;

            case 'l':
                    {
                        if (add)
                            {
                                // +l requires a parameter
                                if (paramIndex >= params.size())
                                {
                                    _sendResponse(ERR_NEEDMOREPARAMS(serverName, "MODE"), fd);
                                    return;
                                }

                            // Parse limit
                            const std::string &limitStr = params[paramIndex];
                            char *end;
                            long limit = std::strtol(limitStr.c_str(), &end, 10);

        // Validation
                            if (*end != '\0' || limit <= 0 || limit > INT_MAX)
                            {
                                _sendResponse(ERR_UNKNOWNMODE(serverName, cli->getNickname(),
                                              ch->getName(),
                                              std::string(1, c)), fd);
                                    return;
                            }

        // Apply limit
                            ch->setUserLimit(static_cast<int>(limit));

                            appliedModes += 'l';
                            appliedParams.push_back(limitStr);
                            paramIndex++;
                                }
                            else
                                {
        // -l removes user limit
                                    ch->removeUserLimit();
                                    appliedModes += 'l';
                                }
                            break;
                        }


            case 'o':
            {
                if (paramIndex >= params.size())
                {
                    _sendResponse(ERR_NEEDMOREPARAMS(serverName, "MODE"), fd);
                    return;
                }

                Client* targetUser = GetClientNick(params[paramIndex]);

                if (!targetUser)
                {
                    _sendResponse(":" + serverName + " 401 " +
                                  cli->getNickname() + " " + params[paramIndex] +
                                  " :No such nick\r\n", fd);
                    paramIndex++;
                    break;
                }

                if (!ch->hasMember(targetUser))
                {
                    _sendResponse(":" + serverName + " 441 " +
                                  cli->getNickname() + " " +
                                  targetUser->getNickname() + " " + ch->getName() +
                                  " :They aren't on that channel\r\n", fd);
                    paramIndex++;
                    break;
                }

                // ✅ YOUR REQUEST: already operator check
                if (add)
                {
                    if (!ch->isOperator(targetUser))
                    {
                        ch->addOperator(targetUser);
                        appliedModes += c;
                        appliedParams.push_back(params[paramIndex]);
                    }
                }
                else
                {
                    if (ch->isOperator(targetUser))
                    {
                        ch->removeOperator(targetUser);
                        appliedModes += c;
                        appliedParams.push_back(params[paramIndex]);
                    }
                }

                paramIndex++;
                break;
            }

            default:
                _sendResponse(ERR_UNKNOWNMODE(serverName, cli->getNickname(),
                                              ch->getName(),
                                              std::string(1, c)), fd);
        }
    }

    // ===== BROADCAST MODE CHANGE =====
    if (!appliedModes.empty())
    {
        std::string msg = ":" + cli->getPrefix() + " MODE " +
                          ch->getName() + " " + sign + appliedModes;

        for (size_t i = 0; i < appliedParams.size(); ++i)
            msg += " " + appliedParams[i];

        msg += "\r\n";
        ch->broadcast(msg, NULL);
    }
}
