#pragma once

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <map>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>

namespace VoiceChat {

// Forward declaration
class UserManager;

// Authentication interface for decoupling
class IAuthenticator {
public:
    virtual ~IAuthenticator() = default;
    virtual bool authenticateUser(const std::string& username, const std::string& password) = 0;
    virtual void updateLastLogin(const std::string& username) = 0;
};

enum class PacketType : uint8_t {
    Audio = 0,
    UserJoin = 1,
    UserLeave = 2,
    UserList = 3,
    Heartbeat = 4,
    AuthRequest = 5,
    AuthResponse = 6
};

struct ClientInfo {
    std::string username;
    struct sockaddr_in address;
    uint8_t channel;
    bool isTalking;
    bool isListening;
    uint64_t lastHeartbeat;
};

struct AudioPacket {
    uint32_t sequence;
    uint32_t timestamp;
    uint16_t dataSize;
    uint8_t channel;  // Channel number (0-3)
    uint8_t packetType; // PacketType
    std::string username;
    std::string password; // For authentication
    bool isTalking;
    bool isListening;
    bool authSuccess; // For auth response
    std::vector<unsigned char> data;
};

class NetworkManager {
public:
    using ReceiveCallback = std::function<void(const AudioPacket& packet)>;
    using UserListCallback = std::function<void(const std::vector<ClientInfo>& users)>;

    NetworkManager();
    ~NetworkManager();

    // Initialize as server (listening mode)
    bool initializeServer(int port, const std::string& bindAddress = "0.0.0.0");
    
    // Initialize as client (connect to server)
    bool initializeClient(const std::string& serverAddress, int port, const std::string& username, const std::string& password = "");
    
    // Set authenticator (for server-side authentication)
    void setAuthenticator(IAuthenticator* authenticator);
    
    // Start receiving packets
    bool startReceiving();
    
    // Stop receiving packets
    void stopReceiving();
    
    // Send an audio packet
    bool sendPacket(const AudioPacket& packet);
    
    // Set callback for received packets
    void setReceiveCallback(ReceiveCallback callback);
    
    // Set callback for user list updates
    void setUserListCallback(UserListCallback callback);
    
    // Check if connected
    bool isConnected() const;
    
    // Get connected users (for server)
    std::vector<ClientInfo> getConnectedUsers() const;
    
    // Disconnect a specific user (for server)
    void disconnectUser(const std::string& username);

private:
    void receiveLoop();
    void heartbeatLoop();
    void broadcastUserList();
    void cleanupInactiveClients();
    
    int socket_;
    struct sockaddr_in serverAddr_;
    struct sockaddr_in clientAddr_;
    std::thread receiveThread_;
    std::thread heartbeatThread_;
    std::atomic<bool> isReceiving_;
    std::atomic<bool> isConnected_;
    ReceiveCallback receiveCallback_;
    UserListCallback userListCallback_;
    uint32_t sequenceNumber_;
    bool isServer_;
    std::string myUsername_;
    std::string myPassword_;
    IAuthenticator* authenticator_;
    
    // Multi-client support
    mutable std::mutex clientsMutex_;
    std::map<std::string, ClientInfo> connectedClients_;
    static constexpr uint64_t CLIENT_TIMEOUT_MS = 5000;
    
    // Authentication helpers
    bool authenticateClient(const std::string& username, const std::string& password);
    void sendAuthResponse(const struct sockaddr_in& clientAddr, bool success);
};

} // namespace VoiceChat
