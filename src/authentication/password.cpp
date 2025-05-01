#include <iostream>
#include <string>
#include "authentication/password.h"

using namespace std;

int Password::num_sets = 0;  

Password &Password::password() {
    static Password pwd;
    return pwd;
}

void Password::setAccessToken(const string& token) {
    if(num_sets > 1) {
        cout << "WARNING: Access token can only be set once per session" << endl;
        return;
    }
    access_token = token;
    num_sets++;
}

void Password::setAccessToken(int& token) {
    if(num_sets > 1) {
        cout << "WARNING: Access token can only be set once per session" << endl;
        return;
    }
    access_token = to_string(token);
    num_sets++;
}

string Password::getAccessToken() const {
    return access_token;
}


