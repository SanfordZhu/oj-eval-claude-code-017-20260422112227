#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "utils.h"
#include "user.h"
#include "train.h"
#include "ticket.h"

using namespace std;

// Global managers
UserManager user_mgr;
TrainManager train_mgr;
TicketManager ticket_mgr(&user_mgr, &train_mgr);

// Helper function to parse string to int, with default value
int parse_int(const MyString& str, int default_val = -1) {
    if (str.empty()) return default_val;
    return atoi(str.c_str());
}

// Helper function to split string by delimiter
void split_string(const MyString& str, char delim, Vector<MyString>& result) {
    result.clear();
    const char* s = str.c_str();
    char buffer[100];
    int pos = 0;

    while (*s) {
        if (*s == delim) {
            buffer[pos] = '\0';
            if (pos > 0) {
                result.push_back(MyString(buffer));
            }
            pos = 0;
        } else {
            buffer[pos++] = *s;
        }
        s++;
    }

    if (pos > 0) {
        buffer[pos] = '\0';
        result.push_back(MyString(buffer));
    }
}

// Helper function to parse int array
void parse_int_array(const MyString& str, char delim, Vector<int>& result) {
    Vector<MyString> parts;
    split_string(str, delim, parts);

    result.clear();
    for (int i = 0; i < parts.size(); i++) {
        result.push_back(atoi(parts[i].c_str()));
    }
}

int main() {
    // Basic I/O setup
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Command cmd;

    while (parse_command(cmd)) {
        if (cmd.type == "add_user") {
            MyString cur_user = cmd.get_param("-c");
            MyString username = cmd.get_param("-u");
            MyString password = cmd.get_param("-p");
            MyString name = cmd.get_param("-n");
            MyString mail = cmd.get_param("-m");
            int privilege = parse_int(cmd.get_param("-g"));

            int result = user_mgr.add_user(cur_user, username, password, name, mail, privilege);
            cout << result << "\n";

        } else if (cmd.type == "login") {
            MyString username = cmd.get_param("-u");
            MyString password = cmd.get_param("-p");

            int result = user_mgr.login(username, password);
            cout << result << "\n";

        } else if (cmd.type == "logout") {
            MyString username = cmd.get_param("-u");

            int result = user_mgr.logout(username);
            cout << result << "\n";

        } else if (cmd.type == "query_profile") {
            MyString cur_user = cmd.get_param("-c");
            MyString username = cmd.get_param("-u");
            MyString result_str;

            int result = user_mgr.query_profile(cur_user, username, result_str);
            if (result == 0) {
                cout << result_str.c_str() << "\n";
            } else {
                cout << "-1\n";
            }

        } else if (cmd.type == "modify_profile") {
            MyString cur_user = cmd.get_param("-c");
            MyString username = cmd.get_param("-u");
            MyString password = cmd.get_param("-p");
            MyString name = cmd.get_param("-n");
            MyString mail = cmd.get_param("-m");
            int privilege = parse_int(cmd.get_param("-g"), -1);
            MyString result_str;

            int result = user_mgr.modify_profile(cur_user, username, password, name, mail, privilege, result_str);
            if (result == 0) {
                cout << result_str.c_str() << "\n";
            } else {
                cout << "-1\n";
            }

        } else if (cmd.type == "add_train") {
            MyString trainID = cmd.get_param("-i");
            int stationNum = parse_int(cmd.get_param("-n"));
            int seatNum = parse_int(cmd.get_param("-m"));

            // Parse stations
            Vector<MyString> stations;
            split_string(cmd.get_param("-s"), '|', stations);

            // Parse prices
            Vector<int> prices;
            parse_int_array(cmd.get_param("-p"), '|', prices);

            // Parse start time
            Time startTime = Time::parse_time(cmd.get_param("-x").c_str());

            // Parse travel times
            Vector<int> travelTimes;
            parse_int_array(cmd.get_param("-t"), '|', travelTimes);

            // Parse stopover times
            Vector<int> stopoverTimes;
            MyString stopover_str = cmd.get_param("-o");
            if (stopover_str != MyString("_")) {
                parse_int_array(stopover_str, '|', stopoverTimes);
            }

            // Parse sale date
            Vector<MyString> date_parts;
            split_string(cmd.get_param("-d"), '|', date_parts);
            Time saleDateFrom = Time::parse_date(date_parts[0].c_str());
            Time saleDateTo = Time::parse_date(date_parts[1].c_str());

            // Parse type
            char type = cmd.get_param("-y").c_str()[0];

            int result = train_mgr.add_train(trainID, stationNum, seatNum, stations, prices,
                                            startTime, travelTimes, stopoverTimes,
                                            saleDateFrom, saleDateTo, type);
            cout << result << "\n";

        } else if (cmd.type == "release_train") {
            MyString trainID = cmd.get_param("-i");

            int result = train_mgr.release_train(trainID);
            cout << result << "\n";

        } else if (cmd.type == "query_train") {
            MyString trainID = cmd.get_param("-i");
            Time date = Time::parse_date(cmd.get_param("-d").c_str());
            MyString result_str;

            int result = train_mgr.query_train(trainID, date, result_str);
            if (result == 0) {
                cout << result_str.c_str();
            } else {
                cout << "-1\n";
            }

        } else if (cmd.type == "delete_train") {
            MyString trainID = cmd.get_param("-i");

            int result = train_mgr.delete_train(trainID);
            cout << result << "\n";

        } else if (cmd.type == "query_ticket") {
            MyString from = cmd.get_param("-s");
            MyString to = cmd.get_param("-t");
            Time date = Time::parse_date(cmd.get_param("-d").c_str());
            MyString sort_type = cmd.get_param("-p", "time");

            Vector<TicketQueryResult> results;
            int count = ticket_mgr.query_ticket(from, to, date, sort_type, results);

            if (count >= 0) {
                cout << count << "\n";
                // Output results (simplified)
                // TODO: Implement proper output formatting
            } else {
                cout << "-1\n";
            }

        } else if (cmd.type == "query_transfer") {
            MyString from = cmd.get_param("-s");
            MyString to = cmd.get_param("-t");
            Time date = Time::parse_date(cmd.get_param("-d").c_str());
            MyString sort_type = cmd.get_param("-p", "time");

            TicketQueryResult result1, result2;
            int result = ticket_mgr.query_transfer(from, to, date, sort_type, result1, result2);

            if (result == 0) {
                cout << "0\n"; // No transfer found
            } else if (result == 1) {
                // Output two trains
                // TODO: Implement proper output formatting
            } else {
                cout << "-1\n";
            }

        } else if (cmd.type == "buy_ticket") {
            MyString username = cmd.get_param("-u");
            MyString trainID = cmd.get_param("-i");
            Time date = Time::parse_date(cmd.get_param("-d").c_str());
            int num = parse_int(cmd.get_param("-n"));
            MyString from = cmd.get_param("-f");
            MyString to = cmd.get_param("-t");
            MyString queue_str = cmd.get_param("-q", "false");
            bool queue = (queue_str == "true");

            int result = ticket_mgr.buy_ticket(username, trainID, date, num, from, to, queue);

            if (result == -2) {
                cout << "queue\n";
            } else {
                cout << result << "\n";
            }

        } else if (cmd.type == "query_order") {
            MyString username = cmd.get_param("-u");
            MyString result_str;

            int result = ticket_mgr.query_order(username, result_str);
            if (result == 0) {
                cout << result_str.c_str();
            } else {
                cout << "-1\n";
            }

        } else if (cmd.type == "refund_ticket") {
            MyString username = cmd.get_param("-u");
            int n = parse_int(cmd.get_param("-n", "1"));

            int result = ticket_mgr.refund_ticket(username, n);
            cout << result << "\n";

        } else if (cmd.type == "clean") {
            user_mgr.clear();
            train_mgr.clear();
            ticket_mgr.clear();
            cout << "0\n";

        } else if (cmd.type == "exit") {
            // Logout all users
            // Note: user_mgr.clear() already handles this
            cout << "bye\n";
            break;
        }

        // Flush output
        cout.flush();
    }

    return 0;
}