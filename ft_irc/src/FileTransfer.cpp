#include "FileTransfer.hpp"
#include <cerrno>

FileTransfer::FileTransfer(const std::string& filename,
							const std::string& sender,
							const std::string& receiver)
	: _listen_fd(-1), _transfer_fd(-1), _filename(filename),
		_filesize(0), _bytes_sent(0), _sender_nick(sender),
		_receiver_nick(receiver), _active(false) {

	// Open file and get size
	_file.open(filename.c_str(), std::ios::binary | std::ios::ate);
	if (_file.is_open()) {
		_filesize = static_cast<unsigned long>(_file.tellg());
		_file.seekg(0, std::ios::beg);
	}
}

FileTransfer::~FileTransfer() {
	abort();
}

unsigned long FileTransfer::_ipToLong(const std::string& ip) {
	struct in_addr addr;
	inet_pton(AF_INET, ip.c_str(), &addr);
	return ntohl(addr.s_addr);
}

std::string FileTransfer::_getLocalIP() {
	// Simplified - returns localhost; in production, get actual network interface IP
	return "127.0.0.1";
}

void FileTransfer::_displayProgressBar(int percentage, const std::string& action) {
	const char* bars[] = {
		"⣿⣀⣀⣀⣀⣀⣀⣀⣀⣀",  // 10%
		"⣿⣿⣀⣀⣀⣀⣀⣀⣀⣀",  // 20%
		"⣿⣿⣿⣀⣀⣀⣀⣀⣀⣀",  // 30%
		"⣿⣿⣿⣿⣀⣀⣀⣀⣀⣀",  // 40%
		"⣿⣿⣿⣿⣿⣀⣀⣀⣀⣀",  // 50%
		"⣿⣿⣿⣿⣿⣿⣀⣀⣀⣀",  // 60%
		"⣿⣿⣿⣿⣿⣿⣿⣀⣀⣀",  // 70%
		"⣿⣿⣿⣿⣿⣿⣿⣿⣀⣀",  // 80%
		"⣿⣿⣿⣿⣿⣿⣿⣿⣿⣀",  // 90%
		"⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿"   // 100%
	};

	int barIndex = percentage / 10;
	if (barIndex < 0) barIndex = 0;
	if (barIndex > 9) barIndex = 9;

	std::string filename = _filename;
	if (!filename.empty()) {
		size_t lastSlash = filename.find_last_of("/\\");
		if (lastSlash != std::string::npos && lastSlash +1 < filename.length()) {
			filename = filename.substr(lastSlash + 1);
		}
	}

	// Truncate if too long
	if (filename.length() > 20) {
		filename = filename.substr(0, 17) + "...";
	}

	if (filename.empty()) filename = "(file)";

	std::cout << "\r"
				<< std::setw(12) <<std::left << action
				<< std::setw(20) <<std::left << _filename.substr(0, 20)
				<< "" << bars[barIndex]
				<< "" << percentage << "%"
				<< std::flush;
}

bool FileTransfer::setupListenSocket(int& port) {
	_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listen_fd < 0) {
		return false;
	}

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = 0; // Let system assign port

	if (bind(_listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(_listen_fd);
		_listen_fd = -1;
		return false;
	}

	if (listen(_listen_fd, 1) < 0) {
		close(_listen_fd);
		_listen_fd = -1;
		return false;
	}

	// Get assigned port
	socklen_t len = sizeof(addr);
	if (getsockname(_listen_fd, (struct sockaddr*)&addr, &len) < 0) {
		close(_listen_fd);
		_listen_fd = -1;
		return false;
	}
	port = ntohs(addr.sin_port);

	// Set non-blocking
	int flags = fcntl(_listen_fd, F_GETFL, 0);
	if (flags < 0) {
		close(_listen_fd);
		_listen_fd = -1;
		return false;
	}
	if (fcntl(_listen_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		close(_listen_fd);
		_listen_fd = -1;
		return false;
	}

	return true;
}

std::string FileTransfer::generateDccSendMessage(int port, const std::string& senderPrefix) {
	if (!_file.is_open())
		return "";

	std::string ip = _getLocalIP();
	unsigned long ip_long = _ipToLong(ip);
	std::ostringstream oss;
	oss << ":" << senderPrefix
		<< " PRIVMSG " << _receiver_nick
		<< " :\001DCC SEND " << _filename << " "
		<< ip_long << " " << port << " "
		<< _filesize << "\001\r\n";

	_active = true;
	return oss.str();
}

bool FileTransfer::acceptConnection() {
	if (_listen_fd < 0) {
		return false;
	}

	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	_transfer_fd = accept(_listen_fd, (struct sockaddr*)&client_addr, &len);
	if (_transfer_fd < 0) {
		// If accept would block, caller should retry after poll indicates readiness
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return false;
		}
		// Real error - cleanup
		close(_listen_fd);
		_listen_fd = -1;
		return false;
	}

	// Set transfer socket to non-blocking
	int flags = fcntl(_transfer_fd, F_GETFL, 0);
	if (flags >= 0) {
		fcntl(_transfer_fd, F_SETFL, flags | O_NONBLOCK);
	}

	// Close listening socket
	close(_listen_fd);
	_listen_fd = -1;

	_active = true;
	std::cout << "DCC connection accepted for " << _filename << std::endl;
	return true;
}

bool FileTransfer::sendFileData() {
	if (_transfer_fd < 0 || !_file.is_open()) {
		return false;
	}

	char buffer[4096];
	_file.read(buffer, sizeof(buffer));

	std::streamsize bytes_read_stream = _file.gcount();
	size_t bytes_read = static_cast<size_t>(bytes_read_stream);

	if (bytes_read > 0) {
		ssize_t sent = send(_transfer_fd, buffer, bytes_read, 0);
		if (sent > 0) {
			_bytes_sent += static_cast<unsigned long>(sent);
			displayTransferProgress();
			
			// Check if transfer is complete
			if (_file.eof() && _bytes_sent >= _filesize) {
				shutdown(_transfer_fd, SHUT_WR);
				close(_transfer_fd);
				_transfer_fd = -1;
				_file.close();
				_active = false;
				return false;
			}
			return true; // More data to send
		} else if (sent == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// Would block - seek back and retry later after poll
				_file.seekg(-static_cast<std::streamoff>(bytes_read), std::ios::cur);
				return true; // Still active, retry later
			}
			// Real send error
			std::perror("send");
			abort();
			return false;
		}
	}

	// EOF reached
	if (_file.eof()) {
		displayTransferProgress();
		if (_transfer_fd >= 0) {
			shutdown(_transfer_fd, SHUT_WR);
			close(_transfer_fd);
			_transfer_fd = -1;
		}
		if (_file.is_open()) {
			_file.close();
		}
		_active = false;
		return false;
	}
	return true;
}

void FileTransfer::displayTransferProgress() {
	if (_filesize == 0) return;

	double ratio = (double)_bytes_sent / (double)_filesize;
	int percentage = static_cast<int>(ratio * 100);

	if (percentage < 0) percentage = 0;
	if (percentage > 100) percentage = 100;
	
	_displayProgressBar(percentage, "Sending");

	if (percentage >= 100) {
		std::cout << " - Complete!" << std::endl;
	}
}

bool FileTransfer::isComplete() const {
	return _bytes_sent >= _filesize;
	}

bool FileTransfer::isActive() const {
	return _active;
	}
	
unsigned long FileTransfer::getFilesize() const {
	return _filesize;
	}

unsigned long FileTransfer::getProgress() const {
	if (_filesize == 0) {
		return 0;
	}
	return static_cast<unsigned long>((static_cast<unsigned long long>(_bytes_sent) * 100ULL) / _filesize);
}

unsigned long FileTransfer::getBytesSent() const {
	return _bytes_sent;
}

void FileTransfer::abort() {
	if (_listen_fd >= 0) {
		close(_listen_fd);
		_listen_fd = -1;
	}
	if (_transfer_fd >= 0) {
		shutdown(_transfer_fd, SHUT_WR); // Signal end of data
		close(_transfer_fd);
		_transfer_fd = -1;
	}
	if (_file.is_open()) { 
		_file.close(); 
	}
	_active = false;
}