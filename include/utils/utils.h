#pragma once

#include <iostream>
#include <fmt/color.h>
#include <fmt/core.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <map>


using namespace std;

namespace utils {

    long long time_now();

    string gen_random(const int len);
    
    string get_signature(long long timestamp, string nonce, string data, string clientsecret);

    string to_hex_string(const unsigned char* data, unsigned int length);

    string hmac_sha256(const string& key, const string& data);

    string pretty(string j);

    string printmap(map<string , string> mpp);

    string getPassword();

    void printcmd(string const &str);
    void printcmd(string const &str, int r, int g, int b);
    void printerr(string const &str);

    int getTerminalWidth();
    void printHeader();
    void printHelp();
    void clear_console();
    bool is_key_pressed(char key);
    bool check_key_pressed(char key);
}