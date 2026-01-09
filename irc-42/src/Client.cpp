/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smuneer <smuneer@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 11:24:02 by asmafrid          #+#    #+#             */
/*   Updated: 2025/11/28 09:28:18 by smuneer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Client.hpp"
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <sys/socket.h>

// CONSTRUCTOR
Client::Client(int fd)
    : _fd(fd),
      _nickname(""),
      _username(""),
      _realname(""),
      _hostname("localhost"),
      _hasPassword(false),
      _hasNick(false),
      _hasUser(false),
      _isRegistered(false),  // ← NEW! Start as not registered
       _isAuthenticated(false),
      _inputBuffer("")
{
    std::cout << "Client Created with fd: " << fd << std::endl;
}

// DESTRUCTOR
Client::~Client()
{
    std::cout << "Client destroyed: " << _nickname << std::endl;
}

// AUTHENTICATION
void Client::setPassword(bool val)
{
    _hasPassword = val;
    _isAuthenticated = true;
   checkRegistration();  // ← Check if registration complete
}

void Client::setNickname(const std::string& nick)
{
    _nickname = nick;
    _hasNick = true;
    checkRegistration();  // ← Check if registration complete
}

bool Client::getisAuthenticated() const
{
    return (_isAuthenticated);
}

void Client::setUsername(const std::string& user, const std::string& real)
{
    _username = user;
    _realname = real;
    _hasUser = true;
    checkRegistration();  // ← Check if registration complete
}

// ===== NEW HELPER FUNCTION =====
// Automatically checks and updates registration status
void Client::checkRegistration()
{
    if (_hasPassword && _hasNick && _hasUser && !_isRegistered)
    {
        _isRegistered = true;
        std::cout << "Client " << _nickname << " is now registered!" << std::endl;
    }
}


// ===== NEW FUNCTION =====
// More explicit name for registration check
bool Client::isRegistered() const
{
    return _isRegistered;
}

// BUFFER MANAGEMENT
void Client::appendToBuffer(const std::string& data)
{
    _inputBuffer += data;
}

bool Client::hasCompleteMessage() const
{
    return (_inputBuffer.find("\r\n") != std::string::npos || 
            _inputBuffer.find("\n") != std::string::npos);
}

std::string Client::extractMessage()
{
    size_t pos = _inputBuffer.find("\r\n");
    
    if (pos == std::string::npos)
    {
        pos = _inputBuffer.find("\n");
        if (pos == std::string::npos)
            return "";
    }
    
    // Extract the message
    std::string message = _inputBuffer.substr(0, pos);
    
    // Remove from buffer including the delimiter
    if (_inputBuffer[pos] == '\r')
        _inputBuffer.erase(0, pos + 2);  // Remove \r\n
    else
        _inputBuffer.erase(0, pos + 1);  // Remove \n
    
    return message;
}

// CHANNEL MANAGEMENT
void Client::joinChannel(Channel* channel)
{
    if (!channel) return;  // Safety check
    if (!isInChannel(channel))
        _joinedChannels.push_back(channel);
}

void Client::leaveChannel(Channel* channel)
{
    std::vector<Channel*>::iterator it = 
        std::find(_joinedChannels.begin(), _joinedChannels.end(), channel);
    
    if (it != _joinedChannels.end())
    {
        _joinedChannels.erase(it);
    }
}

bool Client::isInChannel(Channel* channel) const
{
    return std::find(_joinedChannels.begin(),
                     _joinedChannels.end(),
                     channel) != _joinedChannels.end();
}

std::vector<Channel*> Client::getChannels() const
{
    return _joinedChannels;
}

// GETTERS
int Client::getFd() const
{
    return _fd;
}

std::string Client::getNickname() const
{
    return _nickname;
}

std::string Client::getUsername() const
{
    return _username;
}

std::string Client::getRealname() const
{
    
    return _realname;
}

std::string Client::getHostname() const
{
    return _hostname;
}

std::string Client::getPrefix() const {
    return ":" + _nickname + "!" + _username + "@" + getHostname();
}


// MESSAGING
void Client::sendMessage(const std::string& message)
{
    std::string fullMessage = message;
    
    // Ensure message ends with \r\n
    if (fullMessage.size() < 2 || 
        fullMessage.substr(fullMessage.size() - 2) != "\r\n")
    {
        fullMessage += "\r\n";
    }
    
    // Send via socket
    send(_fd, fullMessage.c_str(), fullMessage.length(), 0);
}

