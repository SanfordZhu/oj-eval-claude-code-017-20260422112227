#ifndef USER_H
#define USER_H

#include "utils.h"

struct User {
    MyString username;
    MyString password;
    MyString name;
    MyString mailAddr;
    int privilege;
    bool logged_in;

    User() : privilege(0), logged_in(false) {}

    User(const MyString& uname, const MyString& pwd, const MyString& nm,
         const MyString& mail, int priv)
        : username(uname), password(pwd), name(nm), mailAddr(mail),
          privilege(priv), logged_in(false) {}
};

class UserManager {
private:
    HashMap<MyString, User> users;
    HashMap<MyString, bool> logged_in_users; // username -> logged_in
    int user_count;

public:
    UserManager() : user_count(0) {}

    // Add a new user
    int add_user(const MyString& cur_user, const MyString& username,
                 const MyString& password, const MyString& name,
                 const MyString& mailAddr, int privilege) {
        // Check if username already exists
        User dummy;
        if (users.find(username, dummy)) {
            return -1; // Username already exists
        }

        if (user_count == 0) {
            // First user, privilege is 10
            User new_user(username, password, name, mailAddr, 10);
            users.insert(username, new_user);
            user_count++;
            return 0;
        }

        // Check current user exists and is logged in
        User cur_user_obj;
        if (!users.find(cur_user, cur_user_obj) || !cur_user_obj.logged_in) {
            return -1;
        }

        // Check privilege: new user's privilege must be lower than current user's
        if (privilege >= cur_user_obj.privilege) {
            return -1;
        }

        User new_user(username, password, name, mailAddr, privilege);
        users.insert(username, new_user);
        user_count++;
        return 0;
    }

    // Login user
    int login(const MyString& username, const MyString& password) {
        User user;
        if (!users.find(username, user)) {
            return -1; // User doesn't exist
        }

        if (user.logged_in) {
            return -1; // Already logged in
        }

        if (user.password != password) {
            return -1; // Wrong password
        }

        // Update logged in status
        user.logged_in = true;
        users.erase(username);
        users.insert(username, user);
        logged_in_users.insert(username, true);

        return 0;
    }

    // Logout user
    int logout(const MyString& username) {
        User user;
        if (!users.find(username, user)) {
            return -1; // User doesn't exist
        }

        if (!user.logged_in) {
            return -1; // Not logged in
        }

        user.logged_in = false;
        users.erase(username);
        users.insert(username, user);
        logged_in_users.erase(username);

        return 0;
    }

    // Query user profile
    int query_profile(const MyString& cur_user, const MyString& username,
                      MyString& result) {
        User cur_user_obj, target_user;
        if (!users.find(cur_user, cur_user_obj) || !cur_user_obj.logged_in) {
            return -1; // Current user doesn't exist or not logged in
        }

        if (!users.find(username, target_user)) {
            return -1; // Target user doesn't exist
        }

        // Check permission: cur_user privilege > target_user privilege, or same user
        if (cur_user_obj.privilege <= target_user.privilege && cur_user != username) {
            return -1;
        }

        char buffer[200];
        sprintf(buffer, "%s %s %s %d", username.c_str(), target_user.name.c_str(),
                target_user.mailAddr.c_str(), target_user.privilege);
        result = MyString(buffer);

        return 0;
    }

    // Modify user profile
    int modify_profile(const MyString& cur_user, const MyString& username,
                       const MyString& password, const MyString& name,
                       const MyString& mailAddr, int privilege,
                       MyString& result) {
        User cur_user_obj, target_user;
        if (!users.find(cur_user, cur_user_obj) || !cur_user_obj.logged_in) {
            return -1; // Current user doesn't exist or not logged in
        }

        if (!users.find(username, target_user)) {
            return -1; // Target user doesn't exist
        }

        // Check permission: cur_user privilege > target_user privilege, or same user
        if (cur_user_obj.privilege <= target_user.privilege && cur_user != username) {
            return -1;
        }

        // If modifying privilege, new privilege must be lower than cur_user's privilege
        if (privilege != -1 && privilege >= cur_user_obj.privilege) {
            return -1;
        }

        // Update fields if provided
        if (!password.empty()) target_user.password = password;
        if (!name.empty()) target_user.name = name;
        if (!mailAddr.empty()) target_user.mailAddr = mailAddr;
        if (privilege != -1) target_user.privilege = privilege;

        // Save changes
        users.erase(username);
        users.insert(username, target_user);

        // Return updated profile
        char buffer[200];
        sprintf(buffer, "%s %s %s %d", username.c_str(), target_user.name.c_str(),
                target_user.mailAddr.c_str(), target_user.privilege);
        result = MyString(buffer);

        return 0;
    }

    // Check if user is logged in
    bool is_logged_in(const MyString& username) {
        bool logged_in;
        if (logged_in_users.find(username, logged_in)) {
            return logged_in;
        }
        return false;
    }

    // Get user by username
    bool get_user(const MyString& username, User& user) {
        return users.find(username, user);
    }

    // Clear all users
    void clear() {
        users.clear();
        logged_in_users.clear();
        user_count = 0;
    }
};

#endif // USER_H