/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asmafrid <asmafrid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 10:51:08 by asmafrid          #+#    #+#             */
/*   Updated: 2025/10/28 15:05:32 by asmafrid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../inc/Channel.hpp"
#include "../inc/Client.hpp"
#include <algorithm>
#include <iostream>

// ===== CONSTRUCTOR =====
Channel::Channel(const std::string& name)
    : _name(name),
      _topic(""),
      _inviteOnly(false),
      _topicRestricted(false),
      _hasKey(false),
      _key(""),
      _hasUserLimit(false),
      _userLimit(0)
{
    std::cout << "Channel created: " << _name << std::endl;
}

// ===== DESTRUCTOR =====
Channel::~Channel()
{
    std::cout << "Channel destroyed: " << _name << std::endl;
}

// ===== ADD MEMBER =====
void Channel::addMember(Client* client)
{
    if (!client || hasMember(client))
        return;

    _members.push_back(client);
    client->joinChannel(this);

    // First member becomes operator
    if (_members.size() == 1)
    {
        addOperator(client);
        std::cout << client->getNickname() << " is operator of " << _name << std::endl;
    }

    // If they were invited, remove from invite list
    removeFromInviteList(client);

    std::cout << client->getNickname() << " joined " << _name << std::endl;
}

// ===== REMOVE MEMBER =====
void Channel::removeMember(Client* client)
{
    if (!client)
        return;

    std::vector<Client*>::iterator it =
        std::find(_members.begin(), _members.end(), client);

    if (it != _members.end())
    {
        _members.erase(it);

        client->leaveChannel(this);

        removeOperator(client);
        removeFromInviteList(client);

        std::cout << client->getNickname() << " left " << _name << std::endl;

        // If last operator leaves, promote next user automatically (RFC bhv)
        if (_operators.empty() && !_members.empty())
        {
            addOperator(_members[0]);
            std::cout << _members[0]->getNickname()
                      << " promoted to operator in " << _name << std::endl;
        }
    }
}

// ===== CHECK IF MEMBER =====
bool Channel::hasMember(Client* client) const
{
    return std::find(_members.begin(), _members.end(), client) != _members.end();
}

// ===== GET MEMBER COUNT =====
size_t Channel::getMemberCount() const
{
    return _members.size();
}

// ===== GET ALL MEMBERS =====
std::vector<Client*> Channel::getMembers() const
{
    return _members;
}

// ===== ADD OPERATOR =====
void Channel::addOperator(Client* client)
{
    if (!client || !hasMember(client))
        return;

    if (!isOperator(client))
        _operators.push_back(client);
}

// ===== REMOVE OPERATOR =====
void Channel::removeOperator(Client* client)
{
    std::vector<Client*>::iterator it =
        std::find(_operators.begin(), _operators.end(), client);

    if (it != _operators.end())
        _operators.erase(it);
}

// ===== CHECK IF OPERATOR =====
bool Channel::isOperator(Client* client) const
{
    return std::find(_operators.begin(), _operators.end(), client) != _operators.end();
}

// ===== BROADCAST =====
void Channel::broadcast(const std::string& message, Client* exclude)
{
    for (size_t i = 0; i < _members.size(); i++)
    {
        if (_members[i] != exclude)
            _members[i]->sendMessage(message);
    }
}

// ===== TOPIC =====
void Channel::setTopic(const std::string& topic)
{
    _topic = topic;
}

std::string Channel::getTopic() const
{
    return _topic;
}

// ===== INVITE-ONLY (i) =====
void Channel::setInviteOnly(bool val)
{
    _inviteOnly = val;
}

bool Channel::isInviteOnly() const
{
    return _inviteOnly;
}

// ===== TOPIC RESTRICTION (t) =====
void Channel::setTopicRestricted(bool val)
{
    _topicRestricted = val;
    std::cout << _topicRestricted << std::endl;
}

bool Channel::isTopicRestricted() const
{
    return _topicRestricted;
}

// ===== KEY (k) =====
void Channel::setKey(const std::string& key)
{
    _key = key;
    _hasKey = true;
}

void Channel::removeKey()
{
    _key = "";
    _hasKey = false;
}

bool Channel::hasKey() const
{
    return _hasKey;
}

std::string Channel::getKey() const
{
    return _key;
}

bool Channel::checkKey(const std::string& key) const
{
    if (!_hasKey)
        return true;

    return (_key == key);
}

// ===== USER LIMIT (l) =====
void Channel::setUserLimit(int limit)
{
    _userLimit = limit;
    _hasUserLimit = true;
}

void Channel::removeUserLimit()
{
    _hasUserLimit = false;
    _userLimit = 0;
}

bool Channel::hasUserLimit() const
{
    return _hasUserLimit;
}

int Channel::getUserLimit() const
{
    return _userLimit;
}

bool Channel::isFull() const
{
    if (!_hasUserLimit)
        return false;

    return ((int)_members.size() >= _userLimit);
}

// ===== INVITE LIST =====
void Channel::addToInviteList(Client* client)
{
    if (client && !isInvited(client))
        _inviteList.push_back(client);
}

void Channel::removeFromInviteList(Client* client)
{
    if (!client)
        return;

    std::vector<Client*>::iterator it =
        std::find(_inviteList.begin(), _inviteList.end(), client);

    if (it != _inviteList.end())
        _inviteList.erase(it);
}

bool Channel::isInvited(Client* client) const
{
    return std::find(_inviteList.begin(), _inviteList.end(), client) != _inviteList.end();
}

// ===== CHECK JOIN PERMISSIONS =====
bool Channel::canJoin(Client* client, const std::string& key) const
{
    if (hasMember(client))
        return false;

    if (_inviteOnly && !isInvited(client))
        return false;

    if (isFull())
        return false;

    if (!checkKey(key))
        return false;

    return true;
}

// ===== GET NAME =====
std::string Channel::getName() const
{
    return _name;
}
