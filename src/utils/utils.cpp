#include <iostream>
#include <sstream>
#include <iomanip>
#include "utils/utils.h"

#include <chrono>
#include <time.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include "json/json.hpp"

#ifdef _WIN32
#include <conio.h> 
#else
#include <termios.h>
#include <unistd.h>
#endif
#include <fcntl.h>

using namespace std;

using json = nlohmann::json;

int utils::getTerminalWidth() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    return 80; // Default to 80 columns if width can't be determined
}

void utils::printHeader() {
    int terminal_width = utils::getTerminalWidth();
    string title = "DERIBIT Backend";
    string border(terminal_width, '-');

    fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, "{}\n", border);
    fmt::print(fg(fmt::color::white) | fmt::emphasis::bold, "{:^{}}\n", title, terminal_width);
    fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, "{}\n", border);
    fmt::print("\n");

}

void utils::printHelp() {
    int terminal_width = utils::getTerminalWidth();
    string separator(terminal_width, '=');

    cout << "\n" << separator << "\n"
              << fmt::format("{:^{}}\n", "COMMAND LIST", terminal_width)
              << separator << "\n\n";

    cout << "GENERAL COMMANDS:\n"
              << fmt::format("  {:<30} : {}\n", "> help", "Displays this help text")
              << fmt::format("  {:<30} : {}\n", "> quit / exit", "Exits the program")
              << fmt::format("  {:<30} : {}\n", "> connect <URI>", "Creates a WebSocket connection with the given URI")
              << fmt::format("  {:<30} : {}\n", "> close <id> [code] [reason]",
                              "Closes the WebSocket connection with the specified ID; optionally specify exit code and reason")
              << fmt::format("  {:<30} : {}\n", "> show <id>", "Displays metadata for the specified connection")
              << fmt::format("  {:<30} : {}\n", "> show_messages <id>", "Lists all messages sent and received on the specified connection")
              << fmt::format("  {:<30} : {}\n", "> send <id> <message>", "Sends a message to the specified connection")
              << fmt::format("  {:<30} : {}\n", "> view_subscriptions", "Displays the list of subscribed symbols to stream continuous orderbook updates")
              << fmt::format("  {:<30} : {}\n", "> view_stream", "Displays the stream continuous orderbook updates subscribed symbols")
              << fmt::format("  {:<30} : {}\n", "> latency_report", "Generates a performance latency report for the current session")
              << fmt::format("  {:<30} : {}\n", "> reset_report", "Clears the latency report data for the current session")
              << "\n";

    cout << "DERIBIT API COMMANDS:\n\n"
              << "  Connection and Authentication:\n"
              << fmt::format("  {:<60} : {}\n", "> Deribit connect", 
                              "Establish a new WebSocket connection to Deribit's testnet")
              << fmt::format("  {:<60} : {}\n", "> Deribit <id> authorize <client_id> <client_secret> [-s]", 
                              "Authenticate and retrieve an access token; use -s to persist token in session")
              << "\n"
              
              << "  Order Management:\n"
              << fmt::format("  {:<60} : {}\n", "> Deribit <id> buy <instrument> [comments]", 
                              "Place a buy market or limit order for the specified instrument")
              << fmt::format("  {:<60} : {}\n", "> Deribit <id> sell <instrument> [comments]", 
                              "Place a sell market or limit order for the specified instrument")
              << fmt::format("  {:<60} : {}\n", "> Deribit <id> modify <order_id>", 
                              "Update price or quantity of an active order")
              << fmt::format("  {:<60} : {}\n", "> Deribit <id> cancel <order_id>", 
                              "Cancel a specific order by its order ID")
              << fmt::format("  {:<60} : {}\n", "> Deribit <id> cancel_all", 
                              "Cancel all active orders for the current account")
              << "\n"
              
              << "  Information Retrieval:\n"
              << fmt::format("  {:<60} : {}\n", "> Deribit <id> get_open_orders {options}", 
                              "Retrieve open orders with optional filtering")
              << fmt::format("  {:<60} : {}\n", "> Deribit <id> positions [currency] [kind]", 
                              "Fetch current open positions, optionally filtered by currency or instrument type")
              << fmt::format("  {:<60} : {}\n", "> Deribit <id> orderbook <instrument> [depth]", 
                              "View current buy and sell orders for an instrument, with optional depth limit")
              << "\n"

              << "  Symbol Subscription:\n"
              << fmt::format("  {:<60} : {}\n", "> Deribit <id> subscribe [symbol]", 
                              "Subscribes to that symbol to stream continuous orderbook updates")
              << fmt::format("  {:<60} : {}\n", "> Deribit <id> unsubscribe [symbol]", 
                              "Unsubscribes to that symbol stream continuous orderbook updates")
              << fmt::format("  {:<60} : {}\n", "> Deribit <id> unsubscribe_all", 
                              "Unsubscribes to all symbols that have been subscribed to stream real time data")
              << "\n";

    cout << separator << "\n\n";
}

void utils::printcmd(string const &str){
    fmt::print(fg(fmt::rgb(219, 186, 221)), str);
}

void utils::printcmd(string const &str, int r, int g, int b){
    fmt::print(fg(fmt::rgb(r, g, b)), str);
}

void utils::printerr(string const &str){
    fmt::print(fg(fmt::rgb(255, 83, 29)) | fmt::emphasis::bold, str);
}

long long utils::time_now(){
    auto now = chrono::system_clock::now();
    auto now_ms = chrono::time_point_cast<chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    long long unix_timestamp_ms = epoch.count();

    return unix_timestamp_ms;
}

string utils::gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}

string utils::to_hex_string(const unsigned char* data, unsigned int length) {
    ostringstream hex_stream;
    hex_stream << hex << uppercase << setfill('0');
    for (unsigned int i = 0; i < length; ++i) {
        hex_stream << setw(2) << static_cast<int>(data[i]);
    }
    return hex_stream.str();
}

string utils::hmac_sha256(const string& key, const string& data) {
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int result_length = 0;

    HMAC(EVP_sha256(), key.c_str(), key.length(),
         reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
         result, &result_length);

    return utils::to_hex_string(result, result_length);
}

string utils::get_signature(long long timestamp, string nonce, string data, string clientsecret){
    // Can be used if the 'grant_type' is "client_signature" to get the SHA256 encoded signature
    string string_to_code = to_string(timestamp) + "\n" + nonce + "\n" + data;
    return utils::hmac_sha256(clientsecret, string_to_code);
}

string utils::pretty(string j) {
    json serialised = json::parse(j);
    return serialised.dump(4);
}

string utils::printmap(map<string, string> mpp) {
    ostringstream os;

    for (const auto& pair : mpp) {
        os << pair.first << " : " << pair.second << '\n'; // Format: Key : Value
    }

    return os.str();
}

string utils::getPassword() {
    string password;
    char ch;

    cout << "Enter access key: ";

    #ifdef _WIN32
        while ((ch = _getch()) != '\r') { 
            if (ch == '\b') {  
                if (!password.empty()) {
                    password.pop_back();
                    cout << "\b \b";
                }
            } else {
                password += ch;
                cout << '*'; 
            }
        }
        cout << endl;

    #else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt); 
        newt = oldt;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        cin >> password;

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        cout << endl;
    #endif

    return password;
}

void utils::clear_console() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}


bool utils::is_key_pressed(char key) {
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    // Set stdin to non-blocking
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch == key) {
        while (getchar() != EOF);
        return true;
    }
    return false;
}
