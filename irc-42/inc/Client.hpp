/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smuneer <smuneer@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 11:01:28 by asmafrid          #+#    #+#             */
/*   Updated: 2025/11/28 09:28:31 by smuneer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Channel;

class Client { 
private: 
    // THE CLIENT'S IDENTITY
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    std::string _hostname;

            bool _hasPassword;                 
            bool _hasNick;
            bool _hasUser;
            bool _isRegistered;
            bool _isAuthenticated;

               
            std::string _inputBuffer;
            std::vector<Channel*> _joinedChannels;
            std::string _modes;  // stores user modes, e.g., "i"


public:
    // CONSTRUCTOR & DESTRUCTOR
    Client(int fd);
    ~Client();

    // AUTHENTICATION
    void setPassword(bool val);
    void setNickname(const std::string& nick);
    void setUsername(const std::string& user, const std::string& real);
    bool isRegistered() const;
    bool getisAuthenticated() const;
    void checkRegistration();
    
    // BUFFER MANAGEMENT
    void appendToBuffer(const std::string& data);
    std::string extractMessage();
    bool hasCompleteMessage() const;

    // CHANNEL MANAGEMENT
    void joinChannel(Channel* channel);
    void leaveChannel(Channel* channel);
    bool isInChannel(Channel* channel) const;
    std::vector<Channel*> getChannels() const;

    // GETTERS
    int getFd() const;
    std::string getNickname() const;
    std::string getUsername() const;
    std::string getRealname() const;
    std::string getHostname() const;
    std::string getPrefix() const;

    // MESSAGING
    void sendMessage(const std::string& message);
};

#endif
