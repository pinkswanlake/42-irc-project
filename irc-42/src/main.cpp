/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smuneer <smuneer@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/28 09:56:49 by smuneer           #+#    #+#             */
/*   Updated: 2025/11/28 09:56:50 by smuneer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include <csignal>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Format: " << argv[0] << " <port number> <password>" << std::endl;
        return 1;
    }

    int port;
    try {
        port = std::atoi(argv[1]);
        if (port < 1024 || port > 65535)
            throw std::out_of_range("Port out of range");
    } catch (const std::exception &e) {
        std::cerr << "Invalid port number" << std::endl;
        return 1;
    }

    std::string password = argv[2];

    // Register Ctrl+C handler
    std::signal(SIGINT, Server::signalHandler);

    Server server(port, password);
    server.initSocket();
    server.run();

    return 0;
}
