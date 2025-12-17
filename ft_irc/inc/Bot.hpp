#ifndef BOT_HPP
#define BOT_HPP

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>
#include <cstdlib>
#include <ctime>

class Bot {
private:
	int _sockfd;
	std::string _server;
	int _port;
	std::string _nick;
	std::string _password;
	std::string _buffer;
	std::time_t _startTime;

	bool _connectSocket();
	void _sendMessage(const std::string& message);
	std::string _readMessage();
	void _handlePing(const std::string& message);
	void _handlePrivmsg(const std::string& message);
	void _parseAndRespond(const std::string& channel,
						const std::string& sender,
						const std::string& text);
	std::vector<std::string> _split(const std::string& str, char delimiter);
	std::string _getStatus();
	std::string _getJoke();

public:
	Bot(const std::string& server, int port,
			const std::string& nick, const std::string& password);
	~Bot();

	bool connect();
	void authenticate();
	void joinChannel(const std::string& channel);
	void messageLoop();
	void disconnect();
};


#endif //BOT_HPP