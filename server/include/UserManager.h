#pragma once

#include <string>
#include <map>
#include <mutex>
#include <vector>
#include "NetworkManager.h"

namespace VoiceChat {

struct User {
    std::string username;
    std::string passwordHash;
    bool isActive;
    uint64_t createdAt;
    uint64_t lastLogin;
};

class UserManager : public IAuthenticator {
public:
    UserManager();
    ~UserManager();

    // Load users from file
    bool loadFromFile(const std::string& filename);
    
    // Save users to file
    bool saveToFile(const std::string& filename);
    
    // Create a new user
    bool createUser(const std::string& username, const std::string& password);
    
    // Delete a user
    bool deleteUser(const std::string& username);
    
    // IAuthenticator interface implementation
    bool authenticateUser(const std::string& username, const std::string& password) override;
    void updateLastLogin(const std::string& username) override;
    
    // Update user password
    bool updatePassword(const std::string& username, const std::string& newPassword);
    
    // Get all users
    std::vector<User> getAllUsers() const;
    
    // Check if user exists
    bool userExists(const std::string& username) const;

private:
    std::string hashPassword(const std::string& password) const;
    bool verifyPassword(const std::string& password, const std::string& hash) const;
    
    mutable std::mutex usersMutex_;
    std::map<std::string, User> users_;
    std::string dataFile_;
};

} // namespace VoiceChat
