#include "api/api.h"
#include "utils/utils.h"
#include "json/json.hpp"
#include "authentication/password.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <regex>
#include <set>
#include <fmt/color.h>



#include "latency/tracker.h"

using namespace std;

using json = nlohmann::json;
bool AUTH_SENT = false;
vector<string> SUPPORTED_CURRENCIES = {"BTC", "ETH", "SOL", "XRP", "MATIC",
                                        "USDC", "USDT", "JPY", "CAD", "AUD", "GBP", 
                                        "EUR", "USD", "CHF", "BRL", "MXN", "COP", 
                                        "CLP", "PEN", "ECS", "ARS",                              
                                    };

vector<string> subscriptions;

vector<string> api::getSubscription(){
    return subscriptions;
}

void api::addSubscriptions(const string &index_name) {
    string subscription = "deribit_price_index." + index_name;
    if (find(subscriptions.begin(), subscriptions.end(), subscription) == subscriptions.end()) {
        subscriptions.push_back(subscription);
    }
}


bool api::removeSubscriptions(const string &index_name) {
    string subscription_to_remove = "deribit_price_index." + index_name;
    auto it = find(subscriptions.begin(), subscriptions.end(), subscription_to_remove);
    if (it != subscriptions.end()) {
        subscriptions.erase(it);
        return 1;
    }
    return 0;
}

bool api::is_valid_instrument(const string& instrument) {
    // Regex for Deribit instrument format
    // Supports formats like BTC-PERPETUAL, ETH-PERPETUAL, BTC-31DEC24, etc.
    regex instrument_pattern(R"(^[A-Z]{3,4}(-)(PERPETUAL|[0-9]{2}[A-Z]{3}[0-9]{2})$)");
    return regex_match(instrument, instrument_pattern);
}

string api::process(const string &input) {

    map<string, function<string(string)>> action_map = 
    {
        {"authorize", api::authorize},
        {"sell", api::sell},
        {"buy", api::buy},
        {"get_open_orders", api::get_open_orders},
        {"modify", api::modify},
        {"cancel", api::cancel},
        {"cancel_all", api::cancel_all},

        {"positions", api::view_positions},
        {"orderbook", api::get_orderbook},

        {"subscribe", api::subscribe},
        {"unsubscribe", api::unsubscribe},
        {"unsubscribe_all", api::unsubscribe_all}
    };

    istringstream s(input.substr(8));
    int id;
    string cmd;
    s >> id >> cmd;

    auto find = action_map.find(cmd);
    if (find == action_map.end()) {
        utils::printerr("ERROR: Unrecognized command. Please enter 'help' to see available commands.\n");
        return "";
    }
    return find->second(input.substr(8));
}

string api::authorize(const string &input) {

    istringstream s(input);
    string auth;
    string id;
    string flag{""};
    string client_id;
    string secret;
    long long tm = utils::time_now();

    s >> id >> auth >> client_id >> secret >> flag;
    string nonce = utils::gen_random(10);

    jsonrpc j;
    j["method"] = "public/auth";
    j["params"] = {{"grant_type", "client_credentials"}, 
    /* 
    NOTE: Changing 'grant_type' to client_signature may help in improving security,
    but will need to use the already provided function utils::get_signature()
    which might invite complexity and errors.
    */
                   {"client_id", client_id},
                   {"client_secret", secret},
                   {"timestamp", tm},
                   {"nonce", nonce},
                   {"scope", "session:name"}
                   };
    if (flag == "-s" && j.dump() != "") AUTH_SENT = true;
    return j.dump();
}

string api::sell(const string &input) {
    string sell;
    string id;
    string instrument;
    string cmd;
    string access_key;
    string order_type;
    string label;
    string frc;
    int contracts{0};
    double amount{0.0};

    istringstream s(input);
    s >> id >> sell >> instrument >> label;

    if (Password::password().getAccessToken() == "") {
        utils::printcmd("Enter the access token: ");
        cin >> access_key;
    }
    else {
        access_key = Password::password().getAccessToken();
    }

    utils::printcmd("\nEnter 1 for contracts or 2 for amount: ");
    int choice;
    cin >> choice;
    
    if (choice == 1) {
        utils::printcmd("Enter the number of contracts: ");
        cin >> contracts;
    } else if (choice == 2) {
        utils::printcmd("Enter the amount: ");
        cin >> amount;
    } else {
        utils::printerr("\nIncorrect syntax; couldn't place order\n");
        return "";
    }

    vector<string> order_types = {
        "limit", 
        "stop_limit", 
        "take_limit", 
        "market", 
        "stop_market", 
        "take_market", 
        "market_limit", 
        "trailing_stop"
    };

    map<string, vector<string>> order_type_tif = {
        {"limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"stop_limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"take_limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"market", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"stop_market", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"take_market", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"market_limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"trailing_stop", {"good_til_cancelled"}}
    };

    // Print available order types
    utils::printcmd("\nAvailable order types:");
    for (size_t i = 0; i < order_types.size(); ++i) {
        utils::printcmd("\n" + to_string(i + 1) + ". " + order_types[i]);
    }
    
    // Prompt for order type selection
    utils::printcmd("\nEnter the number corresponding to the order type: ");
    int order_type_choice;
    cin >> order_type_choice;

    // Validate order type selection
    if (order_type_choice < 1 || order_type_choice > order_types.size()) {
        utils::printerr("\nInvalid order type selection\n");
        return "";
    }
    
    // Get selected order type
    order_type = order_types[order_type_choice - 1];

    // Get permitted time-in-force options for the selected order type
    const vector<string>& permitted_tif = order_type_tif[order_type];
    
    // Print available time-in-force options
    utils::printcmd("\nAvailable time-in-force options for " + order_type + " order:");
    for (size_t i = 0; i < permitted_tif.size(); ++i) {
        utils::printcmd("\n" + to_string(i + 1) + ". " + permitted_tif[i]);
    }
    
    // Prompt for time-in-force selection
    utils::printcmd("\nEnter the number corresponding to the time-in-force value: ");
    int tif_choice;
    cin >> tif_choice;

    // Validate time-in-force selection
    if (tif_choice < 1 || tif_choice > permitted_tif.size()) {
        utils::printerr("\nInvalid time-in-force selection\n");
        return "";
    }
    
    // Get selected time-in-force value
    frc = permitted_tif[tif_choice - 1];

    double price{0.0};
    if (order_type == "limit" || order_type == "stop_limit") {
        utils::printcmd("\nEnter the price at which you want to sell: ");
        cin >> price;
    }

    getLatencyTracker().start_measurement(LatencyTracker::ORDER_PLACEMENT);


    jsonrpc j("private/sell");

    j["params"] = {{"instrument_name", instrument},
                   {"access_token", access_key}};

    // Explicitly choose either amount or contracts based on choice
    if (choice == 2 && amount > 0) { 
        j["params"]["amount"] = amount;
    }
    else if (choice == 1 && contracts > 0) {
        j["params"]["contracts"] = contracts;
    }
    else {
        utils::printerr("\nInvalid quantity specified\n");
        return "";
    }

    if (price > 0) { 
        j["params"]["price"] = price;
    }
    
    j["params"]["type"] = order_type;
    j["params"]["label"] = label;
    j["params"]["time_in_force"] = frc;

    getLatencyTracker().stop_measurement(LatencyTracker::ORDER_PLACEMENT);

    return j.dump();
}

string api::buy(const string &input) {
    string buy;
    string id;
    string instrument;
    string cmd;
    string access_key;
    string order_type;
    string label;
    string frc;
    int contracts{0};
    double amount{0.0};

    istringstream s(input);
    s >> id >> buy >> instrument >> label;

    if (Password::password().getAccessToken() == "") {
        utils::printcmd("Enter the access token: ");
        cin >> access_key;
    }
    else {
        access_key = Password::password().getAccessToken();
    }

    utils::printcmd("\nEnter 1 for contracts or 2 for amount: ");
    int choice;
    cin >> choice;
    
    if (choice == 1) {
        utils::printcmd("Enter the number of contracts: ");
        cin >> contracts;
    } else if (choice == 2) {
        utils::printcmd("Enter the amount: ");
        cin >> amount;
    } else {
        utils::printerr("\nIncorrect syntax; couldn't place order\n");
        return "";
    }

    vector<string> order_types = {
        "limit", 
        "stop_limit", 
        "take_limit", 
        "market", 
        "stop_market", 
        "take_market", 
        "market_limit", 
        "trailing_stop"
    };

    map<string, vector<string>> order_type_tif = {
        {"limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"stop_limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"take_limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"market", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"stop_market", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"take_market", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"market_limit", {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"}},
        {"trailing_stop", {"good_til_cancelled"}}
    };

    utils::printcmd("\nAvailable order types:");
    for (size_t i = 0; i < order_types.size(); ++i) {
        utils::printcmd("\n" + to_string(i + 1) + ". " + order_types[i]);
    }
    
    // Prompt for order type selection
    utils::printcmd("\nEnter the number corresponding to the order type: ");
    int order_type_choice;
    cin >> order_type_choice;

    // Validate order type selection
    if (order_type_choice < 1 || order_type_choice > order_types.size()) {
        utils::printerr("\nInvalid order type selection\n");
        return "";
    }
    
    // Get selected order type
    order_type = order_types[order_type_choice - 1];

    // Get permitted time-in-force options for the selected order type
    const vector<string>& permitted_tif = order_type_tif[order_type];
    
    utils::printcmd("\nAvailable time-in-force options for " + order_type + " order:");
    for (size_t i = 0; i < permitted_tif.size(); ++i) {
        utils::printcmd("\n" + to_string(i + 1) + ". " + permitted_tif[i]);
    }
    
    // Prompt for time-in-force selection
    utils::printcmd("\nEnter the number corresponding to the time-in-force value: ");
    int tif_choice;
    cin >> tif_choice;

    // Validate time-in-force selection
    if (tif_choice < 1 || tif_choice > permitted_tif.size()) {
        utils::printerr("\nInvalid time-in-force selection\n");
        return "";
    }
    
    // Get selected time-in-force value
    frc = permitted_tif[tif_choice - 1];

    double price{0.0};
    if (order_type == "limit" || order_type == "stop_limit") {
        utils::printcmd("\nEnter the price at which you want to buy: ");
        cin >> price;
    }

    getLatencyTracker().start_measurement(LatencyTracker::ORDER_PLACEMENT);


    jsonrpc j("private/buy");

    j["params"] = {{"instrument_name", instrument},
                   {"access_token", access_key}};

    // Explicitly choose either amount or contracts based on choice
    if (choice == 2 && amount > 0) { 
        j["params"]["amount"] = amount;
    }
    else if (choice == 1 && contracts > 0) {
        j["params"]["contracts"] = contracts;
    }
    else {
        utils::printerr("\nInvalid quantity specified\n");
        return "";
    }

    if (price > 0) { 
        j["params"]["price"] = price;
    }
    
    j["params"]["type"] = order_type;
    j["params"]["label"] = label;
    j["params"]["time_in_force"] = frc;

    getLatencyTracker().stop_measurement(LatencyTracker::ORDER_PLACEMENT);

    return j.dump();
}

string api::modify(const string &input) {
    istringstream is(input);

    int id;
    string cmd;
    string ord_id;
    
    // Correctly parse the input
    is >> id >> cmd >> ord_id;

    if (ord_id.empty()) {
        utils::printerr("Error: Order ID is required\n");
        return "";
    }

    jsonrpc j("private/edit");

    double amount = -1.0;
    double price = -1.0;

    utils::printcmd("Enter the new price (-1 to keep current): ");
    cin >> price;

    utils::printcmd("Enter the new amount (-1 to keep current): ");
    cin >> amount;

    getLatencyTracker().start_measurement(LatencyTracker::ORDER_PLACEMENT);


    j["params"] = {{"order_id", ord_id}};
    
    if (amount > 0) j["params"]["amount"] = amount;
    if (price > 0) j["params"]["price"] = price;

    getLatencyTracker().stop_measurement(LatencyTracker::ORDER_PLACEMENT);

    return j.dump();
}

string api::cancel(const string &input) {
    istringstream iss(input);
    int id;
    string cmd;
    string ord_id;

    iss >> id >> cmd >> ord_id;
    if (ord_id.empty()) { 
        fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold,
                "> Order ID cannot be blank.\n");
        fmt::print(fmt::fg(fmt::color::yellow) | fmt::emphasis::bold,
                "> If you want to cancel all orders, use cancel_all instead.\n");
        return "";
    }

    getLatencyTracker().start_measurement(LatencyTracker::ORDER_PLACEMENT);

    jsonrpc j;
    j["method"] = "private/cancel";
    j["params"]["order_id"] = ord_id;

    getLatencyTracker().stop_measurement(LatencyTracker::ORDER_PLACEMENT);
    return j.dump();
}

string api::cancel_all(const string &input) {
    istringstream iss(input);
    int id;
    string cmd;
    string option;
    string label;

    getLatencyTracker().start_measurement(LatencyTracker::ORDER_PLACEMENT);

    jsonrpc j;
    j["params"] = {};

    iss >> id >> cmd >> option >> label;
    if (option.empty()) { 
        j["method"] = "private/cancel_all";
    }
    else if (find(SUPPORTED_CURRENCIES.begin(), SUPPORTED_CURRENCIES.end(), option) == SUPPORTED_CURRENCIES.end()) {
        j["method"] = "private/cancel_all_by_instrument";
        j["params"]["instrument"] = option;
    }
    else if (option == "-s") {
        j["method"] = "private/cancel_by_label";
        j["params"]["label"] = label;
    }
    else { 
        j["method"] = "private/cancel_all_by_currency";
        j["params"]["currency"] = option;
    }

    getLatencyTracker().stop_measurement(LatencyTracker::ORDER_PLACEMENT);
    
    return j.dump();
}

string api::get_open_orders(const string &input) {

    getLatencyTracker().start_measurement(LatencyTracker::MARKET_DATA_PROCESSING);

    istringstream is(input);

    int id;
    string cmd;
    string opt1;
    string opt2;
    is >> id >> cmd >> opt1 >> opt2;

    jsonrpc j;

    if (opt1 == "") {
        j["method"] = "private/get_open_orders";
    }
    else if (find(SUPPORTED_CURRENCIES.begin(), SUPPORTED_CURRENCIES.end(), opt1) == SUPPORTED_CURRENCIES.end()) {
        j["method"] = "private/get_open_orders_by_instrument";
        j["params"] = {{"instrument", opt1}};
    }
    else if ( opt2 == "" ) {
        j["method"] = "private/get_open_orders_by_currency";
        j["params"] = {{"currency", opt1}};
    }
    else {
        j["method"] = "private/get_open_orders_by_label";
        j["params"] = {{"currency", opt1},
                        {"label", opt2}};
    }

    getLatencyTracker().stop_measurement(LatencyTracker::MARKET_DATA_PROCESSING);

    return j.dump();
}

string api::view_positions(const string &input) {
    getLatencyTracker().start_measurement(LatencyTracker::MARKET_DATA_PROCESSING);
    istringstream is(input);
    int id;
    string cmd;
    string currency;
    string kind;    
    is >> id >> cmd >> currency >> kind;
    
    if (!currency.empty()) {
        static const set<string> valid_currencies = {
            "BTC", "ETH", "USDC", "USDT", "EURR"
        };
        if (valid_currencies.find(currency) == valid_currencies.end()) {
            fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold,
                "> Error: Invalid currency format\n");
            return "";
        }
    }
    
    // Inline validation for kind
    if (!kind.empty()) {
        static const set<string> valid_kinds = {
            "future", "option", "spot", 
            "future_combo", "option_combo"
        };
        if (valid_kinds.find(kind) == valid_kinds.end()) {
            fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold,
                "> Error: Invalid instrument kind\n");
            return "";
        }
    }
    
    jsonrpc j;
    j["method"] = "private/get_positions";
    
    if (!currency.empty()) {
        j["params"]["currency"] = currency;
    }
    
    if (!kind.empty()) {
        j["params"]["kind"] = kind;
    }
    
    getLatencyTracker().stop_measurement(LatencyTracker::MARKET_DATA_PROCESSING);
    return j.dump();
}

string api::get_orderbook(const string &input) {
    getLatencyTracker().start_measurement(LatencyTracker::MARKET_DATA_PROCESSING);
    istringstream is(input);
    int id;
    string cmd;
    string instrument;
    int depth = 10;

    is >> id >> cmd >> instrument;
    
    if (instrument.empty()) {
        fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold,
           "> Error: Instrument name is required\n");
        return "";
    }

    jsonrpc j;
    j["method"] = "public/get_order_book";
    j["params"] = {
        {"instrument_name", instrument},
        {"depth", depth}
    };
    getLatencyTracker().stop_measurement(LatencyTracker::MARKET_DATA_PROCESSING);
    return j.dump();
}

string api::subscribe(const string &input) {
    istringstream is(input);
    int id;
    string cmd;
    string index_name;

    is >> id >> cmd >> index_name;

    addSubscriptions(index_name);
    fmt::print(fmt::fg(fmt::color::green) | fmt::emphasis::bold,
           "> Subscribed to {}\n", index_name);
    return "";
}

string api::unsubscribe(const string &input) {
    istringstream is(input);
    int id;
    string cmd;
    string index_name;

    is >> id >> cmd >> index_name;

    if(!removeSubscriptions(index_name)){
        fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold,
           "> Error: Incorrect index name or not subscribed to the specified index.\n");
    }else{
        fmt::print(fmt::fg(fmt::color::yellow) | fmt::emphasis::bold,
            "> Unsubscribed to {}\n", index_name);
    }
    return "";
}

string api::unsubscribe_all(const string &input) {
    istringstream is(input);
    int id;
    string cmd;
    subscriptions = {};
    fmt::print(fmt::fg(fmt::color::yellow) | fmt::emphasis::bold,
        "> Unsubscribed to all symbols\n");
    return "";
}