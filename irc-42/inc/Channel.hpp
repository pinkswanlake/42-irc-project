/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asmafrid <asmafrid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 10:50:52 by asmafrid          #+#    #+#             */
/*   Updated: 2025/10/28 14:40:18 by asmafrid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

class Client;

class Channel {
private:
    // ===== CHANNEL IDENTITY =====
    std::string _name;    
    std::string _topic;
    
    // ===== MEMBERS =====
    std::vector<Client*> _members;
  
    // ===== OPERATORS =====
    std::vector<Client*> _operators;
    
    // ===== INVITE LIST =====
    std::vector<Client*> _inviteList;
    
    // ===== MODES (Channel Rules) =====
    
    bool _inviteOnly;
    bool _topicRestricted;
    
    bool _hasKey;              
    std::string _key;         
    
    bool _hasUserLimit;
    int _userLimit;
    
public:
    // ===== CONSTRUCTOR & DESTRUCTOR =====
    Channel(const std::string& name);
    ~Channel();
    
    // ===== MEMBER MANAGEMENT =====
    void addMember(Client* client);           
    void removeMember(Client* client);       
    bool hasMember(Client* client) const;    
    size_t getMemberCount() const;            
    std::vector<Client*> getMembers() const;
    
    // ===== OPERATOR MANAGEMENT =====
    void addOperator(Client* client);       
    void removeOperator(Client* client);      
    bool isOperator(Client* client) const;  
    
    // ===== INVITE MANAGEMENT =====
    void addToInviteList(Client* client);    
    void removeFromInviteList(Client* client); 
    bool isInvited(Client* client) const;
    
    // ===== BROADCASTING =====
    void broadcast(const std::string& message, Client* exclude = NULL);
    
    // ===== TOPIC =====
    void setTopic(const std::string& topic);
    std::string getTopic() const;
    
    // ===== MODE: INVITE-ONLY (i) =====
    void setInviteOnly(bool val);
    bool isInviteOnly() const;
    
    // ===== MODE: TOPIC RESTRICTED (t) =====
    void setTopicRestricted(bool val);
    bool isTopicRestricted() const;
    
    // ===== MODE: KEY/PASSWORD (k) =====
    void setKey(const std::string& key);
    void removeKey();
    bool hasKey() const;
    std::string getKey() const;
    bool checkKey(const std::string& key) const;  // Is this the right password?
    
    // ===== MODE: USER LIMIT (l) =====
    void setUserLimit(int limit);
    void removeUserLimit();
    bool hasUserLimit() const;
    int getUserLimit() const;
    bool isFull() const;  
    
    // ===== GETTERS =====
    std::string getName() const;
    
    // ===== VALIDATION =====
    bool canJoin(Client* client, const std::string& key) const;
};

#endif