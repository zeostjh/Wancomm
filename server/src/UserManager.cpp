#include "UserManager.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <openssl/sha.h>
#include <cstring>

namespace VoiceChat {

UserManager::UserManager() 
    : dataFile_("users.dat") {
}

UserManager::~UserManager() {
    saveToFile(dataFile_);
}

std::string UserManager::hashPassword(const std::string& password) const {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), 
           password.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') 
           << static_cast<int>(hash[i]);
    }
    return ss.str();
}

bool UserManager::verifyPassword(const std::string& password, const std::string& hash) const {
    return hashPassword(password) == hash;
}

bool UserManager::loadFromFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(usersMutex_);
    dataFile_ = filename;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        return true; // File doesn't exist yet, that's okay
    }
    
    users_.clear();
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        User user;
        char delimiter;
        
        if (std::getline(iss, user.username, '|') &&
            std::getline(iss, user.passwordHash, '|') &&
            iss >> user.isActive >> delimiter &&
            iss >> user.createdAt >> delimiter &&
            iss >> user.lastLogin) {
            users_[user.username] = user;
        }
    }
    
    return true;
}

bool UserManager::saveToFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(usersMutex_);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& [username, user] : users_) {
        file << user.username << "|"
             << user.passwordHash << "|"
             << user.isActive << "|"
             << user.createdAt << "|"
             << user.lastLogin << "\n";
    }
    
    return true;
}

bool UserManager::createUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(usersMutex_);
    
    if (users_.find(username) != users_.end()) {
        return false; // User already exists
    }
    
    User user;
    user.username = username;
    user.passwordHash = hashPassword(password);
    user.isActive = true;
    user.createdAt = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    user.lastLogin = 0;
    
    users_[username] = user;
    return saveToFile(dataFile_);
}

bool UserManager::deleteUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(usersMutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }
    
    users_.erase(it);
    return saveToFile(dataFile_);
}

bool UserManager::authenticateUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(usersMutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }
    
    return it->second.isActive && verifyPassword(password, it->second.passwordHash);
}

bool UserManager::updatePassword(const std::string& username, const std::string& newPassword) {
    if (newPassword.empty()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(usersMutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }
    
    it->second.passwordHash = hashPassword(newPassword);
    return saveToFile(dataFile_);
}

std::vector<User> UserManager::getAllUsers() const {
    std::lock_guard<std::mutex> lock(usersMutex_);
    
    std::vector<User> userList;
    userList.reserve(users_.size());
    
    for (const auto& [username, user] : users_) {
        userList.push_back(user);
    }
    
    return userList;
}

bool UserManager::userExists(const std::string& username) const {
    std::lock_guard<std::mutex> lock(usersMutex_);
    return users_.find(username) != users_.end();
}

void UserManager::updateLastLogin(const std::string& username) {
    std::lock_guard<std::mutex> lock(usersMutex_);
    
    auto it = users_.find(username);
    if (it != users_.end()) {
        it->second.lastLogin = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        saveToFile(dataFile_);
    }
}

} // namespace VoiceChat
