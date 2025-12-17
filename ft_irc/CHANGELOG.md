# ft_irc Project Changelog

## Changes Made for Project Compliance

---

## Summary

This document details all changes made to ensure the ft_irc project complies with the 42 project requirements, specifically:

- **Single poll()**: Only one `poll()` call handles all I/O operations
- **Non-blocking I/O**: All file descriptors are non-blocking
- **Poll-guarded I/O**: All `recv()`/`send()`/`accept()` calls only occur after `poll()` indicates readiness

---

## Files Modified

### 1. `src/server/Server.cpp`

#### 1.1 Output Buffering System

**Problem**: Direct `send()` calls in `_sendToClient()` violated the poll-guarded I/O requirement.

**Solution**: Implemented output buffering with poll-driven flushing.

```cpp
// OLD (non-compliant):
void Server::_sendToClient(int client_fd, const std::string& message) {
    send(client_fd, message.c_str(), message.length(), 0);
}

// NEW (compliant):
void Server::_sendToClient(int client_fd, const std::string& message) {
    _output_buffers[client_fd] += message;
    // Update poll events to include POLLOUT
    for (size_t i = 0; i < _poll_fds.size(); i++) {
        if (_poll_fds[i].fd == client_fd) {
            _poll_fds[i].events |= POLLOUT;
            break;
        }
    }
}
```

**Details**:
- Messages are queued in `_output_buffers` map (keyed by client FD)
- `POLLOUT` event is enabled for the client socket
- Actual `send()` only occurs when `poll()` returns `POLLOUT` for that FD

#### 1.2 Buffer Flushing Function

**New function**: `_flushClientBuffer(int client_fd)`

```cpp
void Server::_flushClientBuffer(int client_fd) {
    std::string& buffer = _output_buffers[client_fd];
    if (buffer.empty()) return;

    ssize_t sent = send(client_fd, buffer.c_str(), buffer.length(), 0);
    if (sent > 0) {
        buffer.erase(0, static_cast<size_t>(sent));
    } else if (sent == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
        return; // Error handled by POLLERR/POLLHUP
    }

    // Remove POLLOUT when buffer is empty
    if (buffer.empty()) {
        for (size_t i = 0; i < _poll_fds.size(); i++) {
            if (_poll_fds[i].fd == client_fd) {
                _poll_fds[i].events = POLLIN;
                break;
            }
        }
    }
}
```

**Details**:
- Only called when `poll()` indicates `POLLOUT` for the FD
- Handles partial sends (only erases sent bytes)
- Handles `EAGAIN`/`EWOULDBLOCK` gracefully
- Removes `POLLOUT` from poll events when buffer is empty

#### 1.3 DCC Transfer FD Registration

**New function**: `_registerTransferFds()`

```cpp
void Server::_registerTransferFds() {
    for (std::map<std::string, FileTransfer*>::iterator it = _active_transfers.begin();
         it != _active_transfers.end(); ++it) {
        FileTransfer* transfer = it->second;
        int listen_fd = transfer->getListenFd();
        int transfer_fd = transfer->getTransferFd();

        // Add listen_fd for POLLIN (accept)
        if (listen_fd >= 0) {
            // ... add to _poll_fds with POLLIN
        }

        // Add transfer_fd for POLLOUT (send)
        if (transfer_fd >= 0) {
            // ... add to _poll_fds with POLLOUT
        }
    }
    // Remove stale FDs from poll array
}
```

**Details**:
- Called at the start of each `poll()` iteration
- Registers DCC listen sockets with `POLLIN` (for accept)
- Registers DCC transfer sockets with `POLLOUT` (for send)
- Removes closed/completed transfer FDs from poll array

#### 1.4 Main Event Loop Modifications

**Changes to `run()`**:

```cpp
void Server::run() {
    while (true) {
        _registerTransferFds();  // NEW: Register DCC FDs before polling

        int poll_count = poll(_poll_fds.data(), _poll_fds.size(), 100);
        // ... error handling ...

        for (size_t i = 0; i < _poll_fds.size(); i++) {
            // Handle POLLIN
            if (_poll_fds[i].revents & POLLIN) {
                if (_poll_fds[i].fd == _server_fd) {
                    _acceptNewClient();
                } else {
                    // NEW: Check for DCC listen FD
                    bool handled = false;
                    for (/* each transfer */) {
                        if (transfer->getListenFd() == _poll_fds[i].fd) {
                            transfer->acceptConnection();
                            handled = true;
                            break;
                        }
                    }
                    if (!handled) {
                        _handleClientData(_poll_fds[i].fd);
                    }
                }
            }

            // NEW: Handle POLLOUT
            if (_poll_fds[i].revents & POLLOUT) {
                // Flush client output buffers
                if (/* has pending output */) {
                    _flushClientBuffer(_poll_fds[i].fd);
                }
                // Handle DCC transfer sends
                for (/* each transfer */) {
                    if (transfer->getTransferFd() == _poll_fds[i].fd) {
                        transfer->sendFileData();
                    }
                }
            }
        }

        _cleanupCompletedTransfers();
    }
}
```

#### 1.5 Client Removal Cleanup

**Modified**: `_removeClient(int client_fd)`

```cpp
void Server::_removeClient(int client_fd) {
    // ... existing channel removal code ...
    // ... existing poll_fds removal code ...
    // ... existing clients map removal code ...

    _output_buffers.erase(client_fd);  // NEW: Remove output buffer

    close(client_fd);
}
```

---

### 2. `inc/Server.hpp`

#### 2.1 New Member Variables

```cpp
private:
    std::map<int, std::string> _output_buffers;  // NEW: Output buffers per client FD
```

#### 2.2 New Member Functions

```cpp
private:
    void _flushClientBuffer(int client_fd);  // NEW: Flush output on POLLOUT
    void _registerTransferFds();             // NEW: Register DCC FDs with poll
    void _cleanupCompletedTransfers();       // NEW: Cleanup finished transfers
```

---

### 3. `src/FileTransfer.cpp`

#### 3.1 Non-blocking Accept

**Modified**: `acceptConnection()`

```cpp
bool FileTransfer::acceptConnection() {
    // ...
    _transfer_fd = accept(_listen_fd, ...);
    if (_transfer_fd < 0) {
        // NEW: Handle EAGAIN/EWOULDBLOCK
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return false;  // Retry later after poll
        }
        // Real error - cleanup
        close(_listen_fd);
        _listen_fd = -1;
        return false;
    }

    // NEW: Set transfer socket to non-blocking
    int flags = fcntl(_transfer_fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(_transfer_fd, F_SETFL, flags | O_NONBLOCK);
    }
    // ...
}
```

#### 3.2 Non-blocking Send

**Modified**: `sendFileData()`

```cpp
bool FileTransfer::sendFileData() {
    // ...
    ssize_t sent = send(_transfer_fd, buffer, bytes_read, 0);
    if (sent > 0) {
        // ... update progress ...
    } else if (sent == -1) {
        // NEW: Handle EAGAIN/EWOULDBLOCK
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Seek back in file to retry this chunk later
            _file.seekg(-static_cast<std::streamoff>(bytes_read), std::ios::cur);
            return true;  // Still active, retry after poll
        }
        // Real error
        abort();
        return false;
    }
    // ...
}
```

---

### 4. `inc/FileTransfer.hpp`

#### 4.1 New Getter Functions

```cpp
public:
    int getListenFd() const { return _listen_fd; }
    int getTransferFd() const { return _transfer_fd; }
```

**Purpose**: Allow Server to access FDs for poll registration.

---

## Compliance Summary

| Requirement | Implementation |
|-------------|----------------|
| Single poll() | One `poll()` call in `Server::run()` handles server socket, client sockets, and DCC transfer sockets |
| Non-blocking FDs | `_setNonBlocking()` called on server, client, and transfer sockets; `O_NONBLOCK` flag set via `fcntl()` |
| Poll-guarded recv() | `recv()` in `_handleClientData()` only called when `POLLIN` is set |
| Poll-guarded send() | `send()` in `_flushClientBuffer()` only called when `POLLOUT` is set |
| Poll-guarded accept() | `accept()` only called when `POLLIN` is set on listen FDs |
| EAGAIN handling | All I/O functions handle `EAGAIN`/`EWOULDBLOCK` and retry on next poll cycle |
| No forking | No `fork()` calls anywhere in codebase |

---

## Additional Changes

### Portuguese to English Comment Translation

**File**: `src/server/Server.cpp`

| Original (Portuguese) | Translated (English) |
|-----------------------|----------------------|
| "Client está iniciando file transfer" | "Client is initiating a file transfer" |
| "Apenas encaminhar para o destinatário" | "Just forward it to the recipient" |
| "Verificar se o alvo existe" | "Check if the target exists" |
| "Criar transferencia" | "Create transfer" |
| "Enviar ao destinatarios via PRIVMSG" | "Send to recipient via PRIVMSG" |
| "Confirmar ao remetente" | "Confirm to sender" |

---

## Testing Checklist

- [ ] Server accepts multiple simultaneous connections
- [ ] No blocking on any I/O operation
- [ ] DCC file transfers work correctly
- [ ] All IRC commands function properly (NICK, USER, JOIN, PRIVMSG, etc.)
- [ ] Channel operator commands work (KICK, INVITE, TOPIC, MODE)
- [ ] Server handles client disconnections gracefully
- [ ] No memory leaks (run with valgrind)

---

## Build Instructions

```bash
# Build server
make

# Build bot (bonus)
make bonus

# Clean and rebuild
make re
```
