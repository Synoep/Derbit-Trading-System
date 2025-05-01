#pragma once

#include <string>

using namespace std;

class Password {

    private:
        static int num_sets;
        string access_token;
        
        Password() : access_token("") {}

    public:
        static Password &password();

        Password(const Password&) = delete;
        void operator=(const Password&) = delete;

        void setAccessToken(const string& token);
        void setAccessToken(int& token);

        string getAccessToken() const;
};
