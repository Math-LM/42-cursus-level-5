#include "Bot.hpp"
#include "FileTransfer.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <signal.h>
#include <unistd.h>

//Global bot pointer for signal handling
Bot* g_bot = NULL;

void signalHandler(int signal) {
	if (signal == SIGINT) {
		std::cout << "\nShutting down..." << std::endl;
		if (g_bot) {
			g_bot->disconnect();
		}
		exit(0);
	}
}

void printUsage(const char* program) {
	std::cerr << "Usage: " << program << "<server> <port> <password> [channel]" << std::endl;
	std::cerr << "Example: " << program << "127.0.0.1 6667 mypass #test" << std::endl;
}

int main(int argc, char **argv) {
	if (argc < 4 || argc > 5) {
		printUsage(argv[0]);
		return 1;
	}

	std::string server = argv[1];
	int port = std::atoi(argv[2]);
	std::string password = argv[3];
	std::string channel = "#test";

	if (argc == 5) {
		channel = argv[4];
		//Ensure channel starts with '#'
		if (channel[0] != '#') {
			channel = "#" + channel;
		}
	}

	//Validate port
	if (port <= 0 || port > 65535) {
		std::cerr << "error: Invalid port number" << std::endl;
		return 1;
	}

	std::cout << "=== IRC BOT ===" << std::endl;
	std::cout << "Server: " << server << ":" << port << std::endl;
	std::cout << "Channel: " << channel << std::endl;
	std::cout << "Bot nick: escraBOTceta" << std::endl;
	std::cout << "===============\n" << std::endl;

	//Setup signal handler
	struct sigaction sa;
	sa.sa_handler = signalHandler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	//Create Bot instance
	Bot bot(server, port, "escraBOTceta", password);
	g_bot = &bot;

	//Connect to server
	std::cout << "Connecting to IRC server..." << std::endl;
	if (!bot.connect()) {
		std::cerr << "Error: Failed to connect to server" << std::endl;
		return 1;
	}

	std::cout << "Connect successfully" << std::endl;

	//Authenticate with server
	std::cout << "authenticating..." << std::endl;
	bot.authenticate();

	//Small delay to authenticate
	sleep(2);

	std::cout << "Joining channel " << channel << "..." << std::endl;
	bot.joinChannel(channel);

	sleep(1);

	std::cout << "\n\x02======= escraBOTceta menu =======\x02\n" << std::endl;
	std::cout << " \x02 !hello \x02 - Greeting message"<< std::endl;
	std::cout << " \x02 !help \x02  - Show help information"<< std::endl;
	std::cout << " \x02 !status \x02- Show bot status"<< std::endl;
	std::cout << " \x02 !time \x02  - Show current day and time"<< std::endl;
	std::cout << " \x02 !joke \x02  - Tell a random joke"<< std::endl;
	std::cout << "\nPress \x02 Ctrl+C \x02 to stop the bot"<< std::endl;
	std::cout << "\x02=================================\x02\n" << std::endl;

	//Loop the message above
	bot.messageLoop();

	std::cout << "Disconnecting..." << std::endl;
	bot.disconnect();

	return 0;
}
