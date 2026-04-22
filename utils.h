#ifndef UTILS_H
#define UTILS_H

#include <cstdio>
#include <cstring>
#include <cstdlib>

// Simple string class (since we can only use std::string from STL)
class MyString {
private:
    char* data;
    int length;
    int capacity;

    void resize(int new_capacity) {
        char* new_data = new char[new_capacity];
        if (data) {
            memcpy(new_data, data, length + 1);
            delete[] data;
        }
        data = new_data;
        capacity = new_capacity;
    }

public:
    MyString() : data(nullptr), length(0), capacity(0) {
        resize(16);
        data[0] = '\0';
    }

    MyString(const char* str) : data(nullptr), length(0), capacity(0) {
        length = strlen(str);
        capacity = length + 1;
        data = new char[capacity];
        memcpy(data, str, capacity);
    }

    MyString(const MyString& other) : data(nullptr), length(0), capacity(0) {
        length = other.length;
        capacity = length + 1;
        data = new char[capacity];
        memcpy(data, other.data, capacity);
    }

    ~MyString() {
        if (data) delete[] data;
    }

    MyString& operator=(const MyString& other) {
        if (this != &other) {
            length = other.length;
            if (capacity < length + 1) {
                delete[] data;
                capacity = length + 1;
                data = new char[capacity];
            }
            memcpy(data, other.data, length + 1);
        }
        return *this;
    }

    const char* c_str() const { return data; }
    int size() const { return length; }
    bool empty() const { return length == 0; }

    void clear() {
        length = 0;
        data[0] = '\0';
    }

    void append(const char* str) {
        int str_len = strlen(str);
        if (length + str_len + 1 > capacity) {
            resize((length + str_len + 1) * 2);
        }
        memcpy(data + length, str, str_len + 1);
        length += str_len;
    }

    bool operator==(const MyString& other) const {
        return strcmp(data, other.data) == 0;
    }

    bool operator==(const char* other) const {
        return strcmp(data, other) == 0;
    }

    bool operator!=(const MyString& other) const {
        return strcmp(data, other.data) != 0;
    }

    bool operator!=(const char* other) const {
        return strcmp(data, other) != 0;
    }

    bool operator<(const MyString& other) const {
        return strcmp(data, other.data) < 0;
    }

    char& operator[](int index) { return data[index]; }
    const char& operator[](int index) const { return data[index]; }
};

// Hash function for MyString
struct MyStringHash {
    unsigned int operator()(const MyString& str) const {
        // Simple hash function
        const char* s = str.c_str();
        unsigned int hash = 0;
        while (*s) {
            hash = hash * 131 + *s;
            s++;
        }
        return hash;
    }
};

// Simple vector implementation
template<typename T>
class Vector {
private:
    T* data;
    int _size;
    int _capacity;

    void resize(int new_capacity) {
        T* new_data = new T[new_capacity];
        if (data) {
            for (int i = 0; i < _size; i++) {
                new_data[i] = data[i];
            }
            delete[] data;
        }
        data = new_data;
        _capacity = new_capacity;
    }

public:
    Vector() : data(nullptr), _size(0), _capacity(0) {
        resize(16);
    }

    ~Vector() {
        if (data) delete[] data;
    }

    void push_back(const T& value) {
        if (_size == _capacity) {
            resize(_capacity * 2);
        }
        data[_size++] = value;
    }

    void pop_back() {
        if (_size > 0) _size--;
    }

    void erase(T* position) {
        int index = position - data;
        if (index < 0 || index >= _size) return;

        for (int i = index; i < _size - 1; i++) {
            data[i] = data[i + 1];
        }
        _size--;
    }

    T& operator[](int index) { return data[index]; }
    const T& operator[](int index) const { return data[index]; }

    int size() const { return _size; }
    bool empty() const { return _size == 0; }

    void clear() { _size = 0; }

    T* begin() { return data; }
    T* end() { return data + _size; }
    const T* begin() const { return data; }
    const T* end() const { return data + _size; }
};

// Simple hash map implementation
template<typename K, typename V, typename Hash = MyStringHash>
class HashMap {
private:
    struct Node {
        K key;
        V value;
        Node* next;
        Node(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
    };

    Node** buckets;
    int bucket_count;
    int _size;
    Hash hash_func;

    int get_bucket(const K& key) const {
        return hash_func(key) % bucket_count;
    }

public:
    HashMap(int bc = 10007) : bucket_count(bc), _size(0) {
        buckets = new Node*[bucket_count];
        memset(buckets, 0, sizeof(Node*) * bucket_count);
    }

    ~HashMap() {
        clear();
        delete[] buckets;
    }

    void clear() {
        for (int i = 0; i < bucket_count; i++) {
            Node* node = buckets[i];
            while (node) {
                Node* next = node->next;
                delete node;
                node = next;
            }
            buckets[i] = nullptr;
        }
        _size = 0;
    }

    bool insert(const K& key, const V& value) {
        int bucket = get_bucket(key);
        Node* node = buckets[bucket];

        // Check if key already exists
        while (node) {
            if (node->key == key) {
                return false; // Key already exists
            }
            node = node->next;
        }

        // Insert new node
        node = new Node(key, value);
        node->next = buckets[bucket];
        buckets[bucket] = node;
        _size++;
        return true;
    }

    bool find(const K& key, V& value) const {
        int bucket = get_bucket(key);
        Node* node = buckets[bucket];

        while (node) {
            if (node->key == key) {
                value = node->value;
                return true;
            }
            node = node->next;
        }
        return false;
    }

    bool erase(const K& key) {
        int bucket = get_bucket(key);
        Node* node = buckets[bucket];
        Node* prev = nullptr;

        while (node) {
            if (node->key == key) {
                if (prev) {
                    prev->next = node->next;
                } else {
                    buckets[bucket] = node->next;
                }
                delete node;
                _size--;
                return true;
            }
            prev = node;
            node = node->next;
        }
        return false;
    }

    int size() const { return _size; }
    bool empty() const { return _size == 0; }
};

// Command parser utilities
struct Command {
    MyString type;
    HashMap<MyString, MyString> params;

    Command() {}

    void clear() {
        type.clear();
        params.clear();
    }

    bool has_param(const char* key) const {
        MyString dummy;
        return params.find(MyString(key), dummy);
    }

    MyString get_param(const char* key, const char* default_val = "") const {
        MyString result;
        if (params.find(MyString(key), result)) {
            return result;
        }
        return MyString(default_val);
    }
};

// Parse a command from input
bool parse_command(Command& cmd) {
    cmd.clear();

    // Read command type
    char buffer[100];
    if (scanf("%s", buffer) != 1) return false;
    cmd.type = MyString(buffer);

    // Read parameters
    while (true) {
        char key[10];
        if (scanf("%s", key) != 1) break;
        if (key[0] != '-') {
            // Not a parameter key, push back
            ungetc(' ', stdin);
            for (int i = strlen(key) - 1; i >= 0; i--) {
                ungetc(key[i], stdin);
            }
            break;
        }

        char value[100];
        if (scanf("%s", value) != 1) return false;

        cmd.params.insert(MyString(key), MyString(value));
    }

    return true;
}

// Time utilities
struct Time {
    int month;  // 6-8
    int day;    // 1-31
    int hour;   // 0-23
    int minute; // 0-59

    Time() : month(0), day(0), hour(0), minute(0) {}
    Time(int m, int d, int h, int mi) : month(m), day(d), hour(h), minute(mi) {}

    // Parse from "mm-dd" format
    static Time parse_date(const char* str) {
        Time t;
        sscanf(str, "%d-%d", &t.month, &t.day);
        return t;
    }

    // Parse from "hr:mi" format
    static Time parse_time(const char* str) {
        Time t;
        sscanf(str, "%d:%d", &t.hour, &t.minute);
        return t;
    }

    // Parse from "mm-dd hr:mi" format
    static Time parse_datetime(const char* str) {
        Time t;
        sscanf(str, "%d-%d %d:%d", &t.month, &t.day, &t.hour, &t.minute);
        return t;
    }

    // Convert to minutes from some reference
    int to_minutes() const {
        // Convert to minutes from June 1, 00:00
        int days = 0;
        if (month == 6) days = day - 1;
        else if (month == 7) days = 30 + day - 1;
        else if (month == 8) days = 61 + day - 1;
        return days * 24 * 60 + hour * 60 + minute;
    }

    // Add minutes
    Time add_minutes(int minutes) const {
        Time result = *this;
        result.minute += minutes;
        result.hour += result.minute / 60;
        result.minute %= 60;
        result.day += result.hour / 24;
        result.hour %= 24;

        // Handle month boundaries (June-August)
        while (result.day > 31) {
            result.day -= 31;
            result.month++;
        }
        while (result.day < 1) {
            result.month--;
            result.day += 31;
        }

        return result;
    }

    // Format as "mm-dd hr:mi"
    void format_datetime(char* buffer) const {
        sprintf(buffer, "%02d-%02d %02d:%02d", month, day, hour, minute);
    }

    // Format as "mm-dd"
    void format_date(char* buffer) const {
        sprintf(buffer, "%02d-%02d", month, day);
    }

    // Format as "hr:mi"
    void format_time(char* buffer) const {
        sprintf(buffer, "%02d:%02d", hour, minute);
    }

    bool operator<(const Time& other) const {
        return to_minutes() < other.to_minutes();
    }

    bool operator==(const Time& other) const {
        return to_minutes() == other.to_minutes();
    }

    int operator-(const Time& other) const {
        return to_minutes() - other.to_minutes();
    }
};

#endif // UTILS_H