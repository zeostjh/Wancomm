#include "NetworkManager.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>

// Forward declaration
namespace VoiceChat {
    class UserManager;
}

namespace VoiceChat {

NetworkManager::NetworkManager()
    : socket_(-1)
    , isReceiving_(false)
    , isConnected_(false)
    , sequenceNumber_(0)
    , isServer_(false)
    , authenticator_(nullptr) {
    std::memset(&serverAddr_, 0, sizeof(serverAddr_));
    std::memset(&clientAddr_, 0, sizeof(clientAddr_));
}

NetworkManager::~NetworkManager() {
    stopReceiving();
    if (socket_ >= 0) {
        close(socket_);
    }
}

bool NetworkManager::initializeServer(int port, const std::string& bindAddress) {
    isServer_ = true;
    
    // Create UDP socket
    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    // Set socket to non-blocking and reusable
    int reuse = 1;
    setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // Bind to port and address
    serverAddr_.sin_family = AF_INET;
    if (bindAddress == "0.0.0.0" || bindAddress.empty()) {
        serverAddr_.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, bindAddress.c_str(), &serverAddr_.sin_addr) <= 0) {
            std::cerr << "Invalid bind address: " << bindAddress << std::endl;
            close(socket_);
            socket_ = -1;
            return false;
        }
    }
    serverAddr_.sin_port = htons(port);

    if (bind(socket_, (struct sockaddr*)&serverAddr_, sizeof(serverAddr_)) < 0) {
        std::cerr << "Failed to bind socket to port " << port << std::endl;
        close(socket_);
        socket_ = -1;
        return false;
    }

    isConnected_ = true;
    std::cout << "Server initialized on port " << port << std::endl;
    return true;
}

bool NetworkManager::initializeClient(const std::string& serverAddress, int port, const std::string& username, const std::string& password) {
    isServer_ = false;
    myUsername_ = username;
    myPassword_ = password;
    
    // Create UDP socket
    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    // Setup server address
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_port = htons(port);
    
    if (inet_pton(AF_INET, serverAddress.c_str(), &serverAddr_.sin_addr) <= 0) {
        std::cerr << "Invalid server address: " << serverAddress << std::endl;
        close(socket_);
        socket_ = -1;
        return false;
    }

    isConnected_ = true;
    
    // Send authentication request
    AudioPacket authPacket;
    authPacket.packetType = static_cast<uint8_t>(PacketType::AuthRequest);
    authPacket.username = myUsername_;
    authPacket.password = myPassword_;
    authPacket.sequence = sequenceNumber_++;
    authPacket.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    sendPacket(authPacket);
    
    std::cout << "Client initialized as '" << username << "', connecting to " << serverAddress << ":" << port << std::endl;
    return true;
}

void NetworkManager::setAuthenticator(IAuthenticator* authenticator) {
    authenticator_ = authenticator;
}

bool NetworkManager::startReceiving() {
    if (!isConnected_) {
        std::cerr << "Not connected" << std::endl;
        return false;
    }

    isReceiving_ = true;
    receiveThread_ = std::thread(&NetworkManager::receiveLoop, this);
    
    // Start heartbeat thread for both server and client
    heartbeatThread_ = std::thread(&NetworkManager::heartbeatLoop, this);
    
    std::cout << "Started receiving packets" << std::endl;
    return true;
}

void NetworkManager::stopReceiving() {
    if (isReceiving_) {
        isReceiving_ = false;
        
        // Send leave packet if client
        if (!isServer_ && isConnected_) {
            AudioPacket leavePacket;
            leavePacket.packetType = static_cast<uint8_t>(PacketType::UserLeave);
            leavePacket.username = myUsername_;
            leavePacket.sequence = sequenceNumber_++;
            leavePacket.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            sendPacket(leavePacket);
        }
        
        if (receiveThread_.joinable()) {
            receiveThread_.join();
        }
        if (heartbeatThread_.joinable()) {
            heartbeatThread_.join();
        }
        std::cout << "Stopped receiving packets" << std::endl;
    }
}

bool NetworkManager::sendPacket(const AudioPacket& packet) {
    if (!isConnected_ || socket_ < 0) {
        return false;
    }

    PacketType type = static_cast<PacketType>(packet.packetType);
    
    // Special handling for authentication packets
    if (type == PacketType::AuthRequest) {
        // Build auth packet: [packetType|sequence|timestamp|usernameLen|username|passwordLen|password]
        size_t usernameLen = packet.username.length();
        size_t passwordLen = packet.password.length();
        const size_t headerSize = 1 + sizeof(uint32_t) * 2 + 1 + 1;
        std::vector<unsigned char> buffer(headerSize + usernameLen + passwordLen);
        
        size_t offset = 0;
        buffer[offset++] = packet.packetType;
        
        uint32_t seq = htonl(packet.sequence);
        std::memcpy(buffer.data() + offset, &seq, sizeof(seq));
        offset += sizeof(seq);
        
        uint32_t ts = htonl(packet.timestamp);
        std::memcpy(buffer.data() + offset, &ts, sizeof(ts));
        offset += sizeof(ts);
        
        buffer[offset++] = static_cast<uint8_t>(usernameLen);
        if (usernameLen > 0) {
            std::memcpy(buffer.data() + offset, packet.username.c_str(), usernameLen);
            offset += usernameLen;
        }
        
        buffer[offset++] = static_cast<uint8_t>(passwordLen);
        if (passwordLen > 0) {
            std::memcpy(buffer.data() + offset, packet.password.c_str(), passwordLen);
            offset += passwordLen;
        }
        
        ssize_t sent = sendto(socket_, buffer.data(), buffer.size(), 0,
                            (struct sockaddr*)&serverAddr_, sizeof(serverAddr_));
        return sent > 0;
    }

    // Build normal packet buffer: [packetType|sequence|timestamp|dataSize|usernameLen|username|channel|isTalking|isListening|data]
    const size_t headerSize = 1 + sizeof(uint32_t) * 2 + sizeof(uint16_t) + 1 + 1 + 1 + 1; // + username length
    size_t usernameLen = packet.username.length();
    std::vector<unsigned char> buffer(headerSize + usernameLen + packet.data.size());
    
    size_t offset = 0;
    
    // Pack header
    buffer[offset++] = packet.packetType;
    
    uint32_t seq = htonl(packet.sequence);
    std::memcpy(buffer.data() + offset, &seq, sizeof(seq));
    offset += sizeof(seq);
    
    uint32_t ts = htonl(packet.timestamp);
    std::memcpy(buffer.data() + offset, &ts, sizeof(ts));
    offset += sizeof(ts);
    
    uint16_t size = htons(packet.dataSize);
    std::memcpy(buffer.data() + offset, &size, sizeof(size));
    offset += sizeof(size);
    
    buffer[offset++] = static_cast<uint8_t>(usernameLen);
    if (usernameLen > 0) {
        std::memcpy(buffer.data() + offset, packet.username.c_str(), usernameLen);
        offset += usernameLen;
    }
    
    buffer[offset++] = packet.channel;
    buffer[offset++] = packet.isTalking ? 1 : 0;
    buffer[offset++] = packet.isListening ? 1 : 0;
    
    if (packet.data.size() > 0) {
        std::memcpy(buffer.data() + offset, packet.data.data(), packet.data.size());
    }

    if (isServer_) {
        // Server sends to all connected clients except sender
        std::lock_guard<std::mutex> lock(clientsMutex_);
        bool anySent = false;
        for (const auto& [username, client] : connectedClients_) {
            if (username != packet.username && client.channel == packet.channel) {
                ssize_t sent = sendto(socket_, buffer.data(), buffer.size(), 0,
                                    (struct sockaddr*)&client.address, sizeof(client.address));
                if (sent > 0) anySent = true;
            }
        }
        return anySent;
    } else {
        // Client sends to server
        ssize_t sent = sendto(socket_, buffer.data(), buffer.size(), 0,
                            (struct sockaddr*)&serverAddr_, sizeof(serverAddr_));
        return sent > 0;
    }
}

void NetworkManager::setReceiveCallback(ReceiveCallback callback) {
    receiveCallback_ = callback;
}

void NetworkManager::setUserListCallback(UserListCallback callback) {
    userListCallback_ = callback;
}

bool NetworkManager::isConnected() const {
    return isConnected_;
}

std::vector<ClientInfo> NetworkManager::getConnectedUsers() const {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    std::vector<ClientInfo> users;
    for (const auto& [username, client] : connectedClients_) {
        users.push_back(client);
    }
    return users;
}

void NetworkManager::disconnectUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    connectedClients_.erase(username);
}

void NetworkManager::receiveLoop() {
    std::vector<unsigned char> buffer(4096);
    struct sockaddr_in fromAddr;
    socklen_t fromLen = sizeof(fromAddr);

    while (isReceiving_) {
        ssize_t received = recvfrom(socket_, buffer.data(), buffer.size(), 0,
                                   (struct sockaddr*)&fromAddr, &fromLen);
        
        if (received < 1) {
            continue;  // Invalid packet
        }

        size_t offset = 0;
        
        // Unpack packet type
        uint8_t packetType = buffer[offset++];
        PacketType type = static_cast<PacketType>(packetType);
        
        // Handle different packet types
        AudioPacket packet;
        packet.packetType = packetType;
        
        // Parse based on packet type
        if (type == PacketType::AuthRequest && isServer_) {
            // Parse auth request: [packetType|sequence|timestamp|usernameLen|username|passwordLen|password]
            if (received < static_cast<ssize_t>(offset + sizeof(uint32_t) * 2 + 2)) {
                continue;
            }
            
            uint32_t seq, ts;
            std::memcpy(&seq, buffer.data() + offset, sizeof(seq));
            offset += sizeof(seq);
            packet.sequence = ntohl(seq);
            
            std::memcpy(&ts, buffer.data() + offset, sizeof(ts));
            offset += sizeof(ts);
            packet.timestamp = ntohl(ts);
            
            uint8_t usernameLen = buffer[offset++];
            if (usernameLen > 0 && received >= static_cast<ssize_t>(offset + usernameLen + 1)) {
                packet.username = std::string(reinterpret_cast<char*>(buffer.data() + offset), usernameLen);
                offset += usernameLen;
            }
            
            uint8_t passwordLen = buffer[offset++];
            if (passwordLen > 0 && received >= static_cast<ssize_t>(offset + passwordLen)) {
                packet.password = std::string(reinterpret_cast<char*>(buffer.data() + offset), passwordLen);
                offset += passwordLen;
            }
            
            // Authenticate client
            bool authSuccess = authenticateClient(packet.username, packet.password);
            sendAuthResponse(fromAddr, authSuccess);
            
            if (authSuccess) {
                // Register new client
                std::lock_guard<std::mutex> lock(clientsMutex_);
                ClientInfo& client = connectedClients_[packet.username];
                client.username = packet.username;
                client.address = fromAddr;
                client.channel = 0; // Default channel
                client.isTalking = false;
                client.isListening = true;
                client.lastHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
                
                if (authenticator_) {
                    authenticator_->updateLastLogin(packet.username);
                }
                
                std::cout << "User authenticated and joined: " << packet.username << std::endl;
                broadcastUserList();
            } else {
                std::cout << "Authentication failed for: " << packet.username << std::endl;
            }
            continue;
        }
        
        if (type == PacketType::AuthResponse && !isServer_) {
            // Parse auth response: [packetType|sequence|timestamp|authSuccess]
            if (received < static_cast<ssize_t>(offset + sizeof(uint32_t) * 2 + 1)) {
                continue;
            }
            
            uint32_t seq, ts;
            std::memcpy(&seq, buffer.data() + offset, sizeof(seq));
            offset += sizeof(seq);
            
            std::memcpy(&ts, buffer.data() + offset, sizeof(ts));
            offset += sizeof(ts);
            
            bool authSuccess = buffer[offset++] != 0;
            
            // Client received authentication response
            if (!authSuccess) {
                std::cerr << "Authentication failed! Disconnecting..." << std::endl;
                isConnected_ = false;
            } else {
                std::cout << "Authentication successful!" << std::endl;
            }
            continue;
        }
        
        // For other packet types, parse standard header
        if (received < static_cast<ssize_t>(offset + sizeof(uint32_t) * 2 + sizeof(uint16_t) + 1)) {
            continue;  // Invalid packet
        }
        
        uint32_t seq, ts;
        uint16_t size;
        
        std::memcpy(&seq, buffer.data() + offset, sizeof(seq));
        offset += sizeof(seq);
        packet.sequence = ntohl(seq);
        
        std::memcpy(&ts, buffer.data() + offset, sizeof(ts));
        offset += sizeof(ts);
        packet.timestamp = ntohl(ts);
        
        std::memcpy(&size, buffer.data() + offset, sizeof(size));
        offset += sizeof(size);
        packet.dataSize = ntohs(size);
        
        // Unpack username
        uint8_t usernameLen = buffer[offset++];
        if (usernameLen > 0 && received >= static_cast<ssize_t>(offset + usernameLen)) {
            packet.username = std::string(reinterpret_cast<char*>(buffer.data() + offset), usernameLen);
            offset += usernameLen;
        }
        
        if (received >= static_cast<ssize_t>(offset + 3)) {
            packet.channel = buffer[offset++];
            packet.isTalking = buffer[offset++] != 0;
            packet.isListening = buffer[offset++] != 0;
        }
        
        // Handle different packet types based on type already parsed above
        
        if (type == PacketType::UserJoin && isServer_) {
            // Register new client
            std::lock_guard<std::mutex> lock(clientsMutex_);
            ClientInfo& client = connectedClients_[packet.username];
            client.username = packet.username;
            client.address = fromAddr;
            client.channel = packet.channel;
            client.isTalking = false;
            client.isListening = true;
            client.lastHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            
            std::cout << "User joined: " << packet.username << std::endl;
            broadcastUserList();
            continue;
        }
        
        if (type == PacketType::UserLeave && isServer_) {
            disconnectUser(packet.username);
            std::cout << "User left: " << packet.username << std::endl;
            broadcastUserList();
            continue;
        }
        
        if (type == PacketType::UserList && !isServer_) {
            // Client received user list from server
            // Parse user list from packet data
            if (userListCallback_) {
                std::vector<ClientInfo> users;
                // The data contains serialized user list
                // For now, just notify that list changed
                userListCallback_(users);
            }
            continue;
        }
        
        if (type == PacketType::Heartbeat) {
            if (isServer_) {
                // Update client heartbeat
                std::lock_guard<std::mutex> lock(clientsMutex_);
                auto it = connectedClients_.find(packet.username);
                if (it != connectedClients_.end()) {
                    it->second.lastHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()).count();
                    it->second.isTalking = packet.isTalking;
                    it->second.isListening = packet.isListening;
                }
            }
            continue;
        }
        
        // Audio packet - copy payload
        if (type == PacketType::Audio) {
            size_t payloadSize = received - offset;
            packet.data.resize(payloadSize);
            if (payloadSize > 0) {
                std::memcpy(packet.data.data(), buffer.data() + offset, payloadSize);
            }
            
            // Update client info if server
            if (isServer_) {
                ClientInfo& client = connectedClients_[packet.username];
                client.address = fromAddr;
                client.isTalking = packet.isTalking;
                client.isListening = packet.isListening;
                client.lastHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
                    
                // Forward packet to other clients on same channel
                std::lock_guard<std::mutex> lock(clientsMutex_);
                for (const auto& [username, destClient] : connectedClients_) {
                    if (username != packet.username && 
                        destClient.channel == packet.channel &&
                        destClient.isListening) {
                        sendto(socket_, buffer.data(), received, 0,
                              (struct sockaddr*)&destClient.address, sizeof(destClient.address));
                    }
                }
            }

            // Call callback
            if (receiveCallback_) {
                receiveCallback_(packet);
            }
        }
    }
}

void NetworkManager::heartbeatLoop() {
    while (isReceiving_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        if (isServer_) {
            cleanupInactiveClients();
        } else {
            // Client sends heartbeat to server
            AudioPacket heartbeat;
            heartbeat.packetType = static_cast<uint8_t>(PacketType::Heartbeat);
            heartbeat.username = myUsername_;
            heartbeat.sequence = sequenceNumber_++;
            heartbeat.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            sendPacket(heartbeat);
        }
    }
}

void NetworkManager::broadcastUserList() {
    if (!isServer_) return;
    
    AudioPacket listPacket;
    listPacket.packetType = static_cast<uint8_t>(PacketType::UserList);
    listPacket.sequence = sequenceNumber_++;
    listPacket.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    // Serialize user list into packet data
    // Simple format: count|username1|username2|...
    std::lock_guard<std::mutex> lock(clientsMutex_);
    std::string userList;
    for (const auto& [username, client] : connectedClients_) {
        if (!userList.empty()) userList += ",";
        userList += username;
    }
    
    listPacket.data.resize(userList.size());
    if (!userList.empty()) {
        std::memcpy(listPacket.data.data(), userList.c_str(), userList.size());
        listPacket.dataSize = userList.size();
    }
    
    // Send to all clients
    for (const auto& [username, client] : connectedClients_) {
        sendto(socket_, listPacket.data.data(), listPacket.data.size(), 0,
              (struct sockaddr*)&client.address, sizeof(client.address));
    }
}

void NetworkManager::cleanupInactiveClients() {
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    std::lock_guard<std::mutex> lock(clientsMutex_);
    auto it = connectedClients_.begin();
    bool anyRemoved = false;
    
    while (it != connectedClients_.end()) {
        if (now - it->second.lastHeartbeat > CLIENT_TIMEOUT_MS) {
            std::cout << "Client timed out: " << it->first << std::endl;
            it = connectedClients_.erase(it);
            anyRemoved = true;
        } else {
            ++it;
        }
    }
    
    if (anyRemoved) {
        broadcastUserList();
    }
}

bool NetworkManager::authenticateClient(const std::string& username, const std::string& password) {
    if (!authenticator_) {
        // No authenticator - allow all (for backward compatibility)
        return true;
    }
    
    return authenticator_->authenticateUser(username, password);
}

void NetworkManager::sendAuthResponse(const struct sockaddr_in& clientAddr, bool success) {
    AudioPacket authResponse;
    authResponse.packetType = static_cast<uint8_t>(PacketType::AuthResponse);
    authResponse.authSuccess = success;
    authResponse.sequence = sequenceNumber_++;
    authResponse.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    // Serialize packet
    std::vector<unsigned char> buffer(16);
    size_t offset = 0;
    
    buffer[offset++] = authResponse.packetType;
    
    uint32_t seq = htonl(authResponse.sequence);
    std::memcpy(buffer.data() + offset, &seq, sizeof(seq));
    offset += sizeof(seq);
    
    uint32_t ts = htonl(authResponse.timestamp);
    std::memcpy(buffer.data() + offset, &ts, sizeof(ts));
    offset += sizeof(ts);
    
    buffer[offset++] = success ? 1 : 0;
    
    sendto(socket_, buffer.data(), offset, 0,
           (struct sockaddr*)&clientAddr, sizeof(clientAddr));
}

} // namespace VoiceChat

