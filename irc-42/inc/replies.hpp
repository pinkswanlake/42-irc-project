/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   replies.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smuneer <smuneer@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/28 09:56:24 by smuneer           #+#    #+#             */
/*   Updated: 2025/11/28 10:15:58 by smuneer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define once

#define CRLF "\r\n"

#define ERR_NOTREGISTERED(server, nick) \
(":" + server + " 451 " + nick + " :You have not registered\r\n")


 #define ERR_NOSUCHCHANNEL(server, nick, chan) \
(":" + server + " 403 " + nick + " " + chan + " :No such channel\r\n")

#define ERR_NOTONCHANNEL(server, nick, chan) \
(":" + server + " 442 " + nick + " " + chan + " :You're not on that channel\r\n")

#define ERR_USERNOTINCHANNEL(server, nick, chan) \
(":" + server + " 441 " + nick + " " + chan + " :They aren't on that channel\r\n")

#define ERR_CHANOPRIVSNEEDED(server, nick, chan) \
(":" + server + " 482 " + nick + " " + chan + " :You're not channel operator\r\n")

 #define ERR_UNKNOWNCOMMAND(server, nick, cmd) \
(":" + server + " 421 " + nick + " " + cmd + " :Unknown command\r\n")

#define ERR_NEEDMOREPARAMS(server, command) \
(":" + server + " 461 " + " " + command + " :Not enough parameters" + CRLF)

#define ERR_ALREADYREGISTERED(server) \
(":" + server + " 462 "  + " :You may not reregister" + CRLF)

#define ERR_PASSWDMISMATCH(server) \
(":" + server + " 464 "  + " :Password incorrect" + CRLF)

#define RPL_WELCOME(server, nick) \
    (":" + server + " 001 " + nick + " :Welcome to the Internet Relay Network " + nick + CRLF)

#define ERR_INVITEONLYCHAN(nick, chan) \
(":" + serverName + " 473 " + nick + " " + chan + " :Cannot join channel (+i)" + CRLF)

#define ERR_CHANNELISFULL(nick, chan) \
(":" + serverName + " 471 " + nick + " " + chan + " :Cannot join channel (+l)" + CRLF)

#define ERR_ALREADYONCHANNEL(nick, chan) \
(":" + serverName + " 443 " + nick + " " + chan + " :is already on channel" + CRLF)

#define ERR_BADCHANNELKEY(nick, chan) \
(":" + serverName + " 475 " + nick + " " + chan + " :Cannot join channel (+k)" + CRLF)

#define ERR_NONICKNAME(serverName)  \
 (":" + serverName +" 431" + " :No nickname given" + CRLF )

#define ERR_NICKINUSE(server, nickname) \
 (":" + serverName + " 433 " + nickname + " " +  nickname + " :Nickname is already in use" + CRLF)

#define ERR_ERRONEUSNICK(server, nickname) \
 (":" + server + "432 " + nickname + " " +  nickname + " :Erroneus nickname" + CRLF)

 #define ERR_NOTENOUGHPARAM(server, nickname) \
  (":" + server +"461 " + nickname + " :Not enough parameters." + CRLF)


#define ERR_NOSUCHNICK(channelname, name) (": 401 #" + channelname + " " + name + " :No such nick" + CRLF )

#define ERR_UNKNOWNMODE(server, nickname, channelname, mode) ":" + server + " 472 " + nickname+ " " + channelname + " " + mode + " :is not a recognised channel mode" + CRLF


#define ERR_NOTOPERATOR(channelname) (": 482 #" + channelname + " :You're not a channel operator" + CRLF)

#define ERR_INCORPASS(nickname) (": 464 " + nickname + " :Password incorrect !" + CRLF )

