#include "Bot.hpp"

Bot::Bot(const std::string& server, int port,
				const std::string& nick, const std::string& password)
	:  _sockfd(-1), _server(server), _port(port),
		_nick(nick), _password(password) {
			_startTime = std::time(NULL);
		}

Bot::~Bot() {
	if (_sockfd != -1)
		close(_sockfd);
}

bool Bot::_connectSocket() {
	struct sockaddr_in serv_addr;

	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd < 0) {
		std::cerr << "Error: cannot create socket" << std::endl;
		return false;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(_port);

	struct hostent* host = gethostbyname(_server.c_str());
	if (!host) {
		std::cerr << "Error: unknown host" << std::endl;
		return false;
	}
	memcpy(&serv_addr.sin_addr, host->h_addr, host->h_length);

	if (::connect(_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		std::cerr << "Error: connection failed" << std::endl;
		return false;
	}

	std::cout << "Connected to " << _server << ":" << _port << std::endl;
	return true;
}

void Bot::_sendMessage(const std::string& message) {
	if (send(_sockfd, message.c_str(), message.length(), 0) < 0) {
		std::cerr << "Error: failed to send message" << std::endl;
	}
}

std::string Bot::_readMessage() {
	// First, check if there's already a complete message in the buffer
	size_t pos = _buffer.find("\n");
	if (pos != std::string::npos) {
		std::string message = _buffer.substr(0, pos);
		_buffer = _buffer.substr(pos + 1);
		// Remove trailing \r if present
		if (!message.empty() && message[message.length() - 1] == '\r') {
			message = message.substr(0, message.length() - 1);
		}
		return message;
	}

	// No complete message in buffer, read more data from socket
	char buffer[512];
	int n = recv(_sockfd, buffer, sizeof(buffer) - 1, 0);

	if (n <= 0)
		return "";

	buffer[n] = '\0';
	_buffer += std::string(buffer, n);

	// Try to extract a complete message again
	pos = _buffer.find("\n");
	if (pos != std::string::npos) {
		std::string message = _buffer.substr(0, pos);
		_buffer = _buffer.substr(pos + 1);
		// Remove trailing \r if present
		if (!message.empty() && message[message.length() - 1] == '\r') {
			message = message.substr(0, message.length() - 1);
		}
		return message;
	}

	return ""; //No complete message yet
}

std::vector<std::string> Bot::_split(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;

	while (std::getline(ss, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

void Bot::_handlePing(const std::string& message) {
	size_t pos = message.find("PING");
	if (pos != std::string::npos) {
		std::string response = "PONG " + message.substr(pos + 4);
		_sendMessage(response);
		std::cout << "Respond to PING" << std::endl;
	}
}

void Bot::_handlePrivmsg(const std::string& message) {
	std::cout << 1;
	size_t senderEnd = message.find('!');
	if (senderEnd == std::string::npos || senderEnd == 0)
		return;
	std::cout << 2;

	std::string sender = message.substr(1, senderEnd - 1);

	size_t privmsgPos = message.find("PRIVMSG");
	if (privmsgPos == std::string::npos)
		return;
	std::cout << 3;

	size_t channelStart = privmsgPos + 8;
	if (channelStart >= message.length())
		return;
	std::cout << 4;


	size_t channelEnd = message.find(' ', channelStart);
	if (channelEnd == std::string::npos)
		return;

	std::cout << 5;
	std::string channel = message.substr(channelStart, channelEnd - channelStart);

	size_t textStart = message.find(':', channelEnd);
	if (textStart == std::string::npos)
		return;

	std::cout << 6;
	std::string text = message.substr(textStart + 1);

	std::cout << "textStart: " << text << std::endl;

	// Remove trailing whitespace characters (\r, \n, spaces, etc.)
	while (!text.empty() && (text[text.length() - 1] == '\r' || 
							  text[text.length() - 1] == '\n' ||
							  text[text.length() - 1] == ' ' ||
							  text[text.length() - 1] == '\t')) {
		text = text.substr(0, text.length() - 1);
	}
	
	_parseAndRespond(channel, sender, text);
}

void Bot::_parseAndRespond(const std::string& channel,
							const std::string& sender,
							const std::string& text) {
	// Check if message is a command (starting with '!')
	if (text.empty() || text[0] != '!') return;

	std::string cmd = text.substr(1);

	// If it's a direct message (channel is bot's nick), respond to the sender, otherwise, respond to the channel
	std::string target = (channel == _nick) ? sender : channel;

	std::cout << "CMD being received: " << cmd << std::endl;
	//Handle different commands
	if (cmd == "hello") {
		std::string response = "PRIVMSG " + target + " :Hello " + sender + "!\r\n";
		_sendMessage(response);
	} else if (cmd == "help") {
		std::string response = "PRIVMSG " + target + " :Available commands: \x02!hello, !help, !joke, !status, !time\x02\r\n";
		_sendMessage(response);
	} else if (cmd == "joke") {
		std::string joke = _getJoke();
		std::string response = "PRIVMSG " + target + " :" + joke + "\r\n";
		_sendMessage(response);
	} else if (cmd == "time") {
		std::time_t now = std::time(NULL);
		char buffer[64];
		std::tm* tm_info = std::localtime(&now);
		std::strftime(buffer, sizeof(buffer), "\x02%A, %B %d %Y\x02 - \x02%H:%M:%S\x02", tm_info);
		std::string timeStr(buffer);
		std::string response = "PRIVMSG " + target + " :Current day and time: " + timeStr + "\r\n";
		_sendMessage(response);
	} else if (cmd == "status") {
		std::string uptimeStr = _getStatus();
		std::string response = "PRIVMSG " + target + " :Bot is\x02 ACTIVE\x02 and has been running for " + uptimeStr + "\r\n";
		_sendMessage(response);
	}
}

std::string Bot::_getStatus() {
	std::time_t now = std::time(NULL);
	int sec = static_cast<int>(difftime(now, _startTime));
	
	int day = sec / 86400;
	sec %= 86400;
	int hr = sec / 3600;
	sec %= 3600;
	int min = sec / 60;
	sec %= 60;

	std::stringstream ss;
	if (day > 0)
		ss << day << "d ";
	if (hr > 0)
		ss << hr << "h ";
	if (min > 0)
		ss << min << "m ";
	ss << sec << "s ";
	return ss.str();
}

std::string Bot::_getJoke() {
	static const std::string hardcodedJokes[] = {
		"Why do programmers prefer dark mode?...\x02 Because light attracts bugs.\x02",
		"Why did the programmer quit his job?...\x02 Because he didn't get arrays.\x02",
		"How many programmers does it take to change a light bulb?...\x02 None, that's a hardware problem.\x02",
		"Why do Java developers wear glasses?...\x02 Because they don't C#.\x02",
		"What's a programmer's favorite hangout place?...\x02 Foo Bar.\x02",
		"Why did the developer go broke?...\x02 Because he used up all his cache.\x02",
		"What do you call 8 hobbits?...\x02 A hobbyte.\x02",
		"There are 10 types of people in the world:\x02 those who understand binary and those who don't.\x02",
		"A SQL query walks into a bar, walks up to two tables and asks:\x02 Can I join you?\x02",
		"Why was the JavaScript developer sad?...\x02 Because he didn't know how to 'null' his feelings.\x02",
		"I have a joke about UDP,\x02 but you might not get it.\x02",
		"How do you comfort a JavaScript bug?...\x02 You console it.\x02",
		"Why do programmers always mix up Halloween and Christmas?...\x02 Because Oct 31 == Dec 25.\x02",
		"What's the object-oriented way to become wealthy?...\x02 Inheritance.\x02",
		"Why did the computer keep sneezing?...\x02 It had a virus.\x02",
		"A programmer's wife tells him: Go to the store and pick up a loaf of bread. If they have eggs, get a dozen.\x02 The programmer returns home with 12 loaves of bread.\x02",
		"Why don't programmers like nature?...\x02 It has too many bugs.\x02",
		"What do you call a programmer from Finland?...\x02 Nerdic.\x02",
		"Why was the function sad?...\x02 Because it didn't get a callback.\x02",
		"How many programmers does it take to change a lightbulb?...\x02 Only one. But then the whole house falls down.\x02",
		"What did the Java code say to the C code?...\x02 You've got no class.\x02",
		"Why did the programmer get stuck in the shower?...\x02 The shampoo bottle said: Lather, Rinse, Repeat.\x02",
		"What's a programmer's favorite snack?...\x02 Microchips.\x02",
		"Why do programmers hate the outdoors?...\x02 The sunlight causes too much glare on their screens.\x02",
		"What's the best thing about a Boolean?...\x02 Even if you're wrong, you're only off by a bit.\x02",
		"To understand recursion,\x02 you must first understand recursion.\x02",
		"Why do programmers prefer iOS development?...\x02 Because they don't like Java.\x02",
		"What did the router say to the doctor?...\x02 It hurts when IP.\x02",
		"Why did the developer stay calm?...\x02 Because he had exception handling.\x02",
		"What's a programmer's least favorite day?...\x02 Array of Sundays.\x02"
	};
	static const size_t jokeCount = sizeof(hardcodedJokes) / sizeof(hardcodedJokes[0]);
	
	static bool seeded = false;
	if (!seeded) {
		std::srand(std::time(NULL));
		seeded = true;
	}

	size_t randomIndex = std::rand() % jokeCount;
	return hardcodedJokes[randomIndex];
}

bool Bot::connect() { return _connectSocket(); }

void Bot::authenticate() {
	// Send password if required
	if (!_password.empty()) {
		std::string passMsg = "PASS " + _password + "\r\n";
		_sendMessage(passMsg);
	}

	// Send NICK and USER commands
	std::string nickMsg = "NICK " + _nick + "\r\n";
	_sendMessage(nickMsg);

	std::string userMsg = "USER " + _nick + " 0 * : " + _nick + "\r\n";
	_sendMessage(userMsg);

	std::cout << "Sent authentication commands" << std::endl;
}

void Bot::joinChannel(const std::string& channel) {
	std::string joinMsg = "JOIN " + channel + "\r\n";
	_sendMessage(joinMsg);
	std::cout << "Joining channel " << channel << std::endl;
}

void Bot::messageLoop() {
	std::cout << "Bot message loop started" << std::endl;
	while (true) {
		std::string message = _readMessage();

		if (message.empty()) {
			// Check if connection is still alive
			char buffer[1];
			int n = recv(_sockfd, buffer, 1, MSG_PEEK | MSG_DONTWAIT);
			if (n == 0) {
				std::cerr << "Connection closed by server" << std::endl;
				break;
			}
			continue;
		}

		if (message.find("PING") != std::string::npos) {
			_handlePing(message);
		} else if (message.find("PRIVMSG") != std::string::npos) {
			_handlePrivmsg(message);
		}
	}
}

void Bot::disconnect() {
	if (_sockfd != -1) {
		close(_sockfd);
		_sockfd = -1;
	}
}