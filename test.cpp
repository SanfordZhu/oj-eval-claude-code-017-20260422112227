#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "utils.h"

using namespace std;

int main() {
    // Test MyString
    MyString s1 = "hello";
    MyString s2 = s1;
    MyString s3;
    s3 = s2;

    cout << "s1: " << s1.c_str() << endl;
    cout << "s2: " << s2.c_str() << endl;
    cout << "s3: " << s3.c_str() << endl;

    // Test Vector
    Vector<int> v;
    for (int i = 0; i < 10; i++) {
        v.push_back(i);
    }

    cout << "Vector: ";
    for (int i = 0; i < v.size(); i++) {
        cout << v[i] << " ";
    }
    cout << endl;

    // Test HashMap
    HashMap<MyString, int> map;
    map.insert("one", 1);
    map.insert("two", 2);
    map.insert("three", 3);

    int value;
    if (map.find("two", value)) {
        cout << "Found two: " << value << endl;
    }

    // Test Time
    Time t1 = Time::parse_date("06-15");
    Time t2 = Time::parse_time("14:30");
    Time t3 = Time::parse_datetime("07-20 08:45");

    char buf[100];
    t1.format_date(buf);
    cout << "Date: " << buf << endl;

    t2.format_time(buf);
    cout << "Time: " << buf << endl;

    t3.format_datetime(buf);
    cout << "DateTime: " << buf << endl;

    // Test command parsing
    cout << "\nTesting command parsing..." << endl;
    // Simulate input
    const char* test_input = "add_user -c admin -u user1 -p pass123 -n 张三 -m test@test.com -g 5\n";
    freopen("test_input.txt", "w", stdout);
    printf("%s", test_input);
    fclose(stdout);

    freopen("test_input.txt", "r", stdin);

    Command cmd;
    if (parse_command(cmd)) {
        cout << "Command type: " << cmd.type.c_str() << endl;
        MyString val;
        if (cmd.params.find(MyString("-c"), val)) {
            cout << "-c: " << val.c_str() << endl;
        }
        if (cmd.params.find(MyString("-u"), val)) {
            cout << "-u: " << val.c_str() << endl;
        }
    }

    return 0;
}