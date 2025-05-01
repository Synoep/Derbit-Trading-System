#include "websocket/websocket_client.h"
#include "utils/utils.h"
#include "authentication/password.h"
#include <fmt/color.h>
#include "latency/tracker.h"

using namespace std;

bool isStreaming = false;

connection_metadata::connection_metadata(
    int id, 
    websocketpp::connection_hdl hdl, 
    string uri, 
    websocket_endpoint* endpoint
) :
    m_id(id),
    m_hdl(hdl),
    m_status("Connecting"),
    m_uri(uri),
    m_server("N/A"),
    m_messages({}),
    m_summaries({}),
    m_endpoint(endpoint),
    MSG_PROCESSED(false)
{}

int connection_metadata::get_id() { return m_id; }
websocketpp::connection_hdl connection_metadata::get_hdl() { return m_hdl; }
string connection_metadata::get_status() { return m_status; }

void connection_metadata::record_sent_message(string const &message) {
    m_messages.push_back("SENT: " + message);
}

void connection_metadata::record_summary(string const &message, string const &sent) {
    if (message == "") return;
    json parsed_msg = json::parse(message);
    string cmd = parsed_msg.contains("method") ? parsed_msg["method"] : "received";
    map<string, string> summary;
    
    map<string, function<map<string, string>(json)>> action_map = 
    {
        {"public/auth", [](json parsed_msg){ 
            map<string, string> summary;
            summary["method"] = parsed_msg["method"];
            summary["grant_type"] = parsed_msg["params"]["grant_type"];
            summary["client_id"] = parsed_msg["params"]["client_id"];
            summary["timestamp"] = to_string(parsed_msg["params"]["timestamp"].get<long long>());
            summary["nonce"] = parsed_msg["params"]["nonce"];
            summary["scope"] = parsed_msg["params"]["scope"];
            return summary;
        }},
        
        {"private/sell", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["instrument_name"] = parsed_msg["params"]["instrument_name"];
            summary["access_token"] = parsed_msg["params"]["access_token"];
            
            if (parsed_msg["params"].contains("amount"))
                summary["amount"] = to_string(parsed_msg["params"]["amount"].get<double>());
            
            if (parsed_msg["params"].contains("contracts"))
                summary["contracts"] = to_string(parsed_msg["params"]["contracts"].get<int>());
            
            summary["order_type"] = parsed_msg["params"]["type"];
            summary["label"] = parsed_msg["params"]["label"];
            summary["time_in_force"] = parsed_msg["params"]["time_in_force"];
            
            if (parsed_msg["params"].contains("price"))
                summary["price"] = to_string(parsed_msg["params"]["price"].get<double>());
            
            return summary;
        }},
        
        {"private/buy", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["instrument_name"] = parsed_msg["params"]["instrument_name"];
            summary["access_token"] = parsed_msg["params"]["access_token"];
            
            if (parsed_msg["params"].contains("amount"))
                summary["amount"] = to_string(parsed_msg["params"]["amount"].get<double>());
            
            if (parsed_msg["params"].contains("contracts"))
                summary["contracts"] = to_string(parsed_msg["params"]["contracts"].get<int>());
            
            summary["order_type"] = parsed_msg["params"]["type"];
            summary["label"] = parsed_msg["params"]["label"];
            summary["time_in_force"] = parsed_msg["params"]["time_in_force"];
            
            if (parsed_msg["params"].contains("price"))
                summary["price"] = to_string(parsed_msg["params"]["price"].get<double>());
            
            return summary;
        }},
        
        {"private/edit", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["order_id"] = parsed_msg["params"]["order_id"];
            
            if (parsed_msg["params"].contains("amount"))
                summary["new_amount"] = to_string(parsed_msg["params"]["amount"].get<double>());
            
            if (parsed_msg["params"].contains("price"))
                summary["new_price"] = to_string(parsed_msg["params"]["price"].get<double>());
            
            return summary;
        }},
        
        {"private/cancel", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["order_id"] = parsed_msg["params"]["order_id"];
            return summary;
        }},
        
        {"private/cancel_all", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            return summary;
        }},
        
        {"private/cancel_all_by_instrument", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["instrument"] = parsed_msg["params"]["instrument"];
            return summary;
        }},
        
        {"private/cancel_by_label", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["label"] = parsed_msg["params"]["label"];
            return summary;
        }},
        
        {"private/cancel_all_by_currency", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["currency"] = parsed_msg["params"]["currency"];
            return summary;
        }},
        
        {"private/get_open_orders", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            return summary;
        }},
        
        {"private/get_open_orders_by_instrument", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["instrument"] = parsed_msg["params"]["instrument"];
            return summary;
        }},
        
        {"private/get_open_orders_by_currency", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["currency"] = parsed_msg["params"]["currency"];
            return summary;
        }},
        
        {"private/get_open_orders_by_label", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["currency"] = parsed_msg["params"]["currency"];
            summary["label"] = parsed_msg["params"]["label"];
            return summary;
        }},
        
        {"private/get_positions", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            
            if (parsed_msg["params"].contains("currency"))
                summary["currency"] = parsed_msg["params"]["currency"];
            
            if (parsed_msg["params"].contains("kind"))
                summary["kind"] = parsed_msg["params"]["kind"];
            
            return summary;
        }},
        
        {"public/get_order_book", [](json parsed_msg){
            map<string, string> summary = {};
            summary["method"] = parsed_msg["method"];
            summary["instrument_name"] = parsed_msg["params"]["instrument_name"];
            summary["depth"] = to_string(parsed_msg["params"]["depth"].get<int>());
            return summary;
        }},
        
        {"received", [](json parsed_msg){
            map<string, string> summary = {};
            if (parsed_msg.contains("result"))
                summary = {{"result", parsed_msg["result"].dump()}};
            else if (parsed_msg.contains("error"))
                summary = {{"error message", parsed_msg["error"].dump()}};
            return summary;
        }}
    };
    
    auto find = action_map.find(cmd);
    if (find == action_map.end()) {
        summary["id"] = to_string(parsed_msg["id"].get<int>());
        if (sent == "SENT") summary["method"] = parsed_msg["method"];
    }
    else {
        summary = find->second(parsed_msg);
    }
    m_summaries.push_back(sent + " : \n" + utils::printmap(summary));
}

void connection_metadata::on_open(client * c, websocketpp::connection_hdl hdl) {
    m_status = "Connected";
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
}

void connection_metadata::on_fail(client * c, websocketpp::connection_hdl hdl) {
    m_status = "Failed";
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
    m_error_reason = con->get_ec().message();
}

void connection_metadata::on_close(client * c, websocketpp::connection_hdl hdl) {
    m_status = "Closed";
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    stringstream s;
    s << "Close code: " << con->get_remote_close_code() << "("
      << websocketpp::close::status::get_string(con->get_remote_close_code())
      << "), Close reason: " << con->get_remote_close_reason();
    
    m_error_reason = s.str();
}

void connection_metadata::on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
    // Start latency tracking
    getLatencyTracker().start_measurement(
        LatencyTracker::WEBSOCKET_MESSAGE_PROPAGATION, 
        "websocket_message_" + to_string(m_id)
    );

    try {
        if (!msg) return;

        string payload = msg->get_payload();

        json received_json;
        try {
            received_json = json::parse(payload);
        } catch (const json::parse_error& e) {
            cerr << "JSON parse error: " << e.what() << endl;
            cerr << "Problematic payload: " << payload << endl;
            return;
        }

        if (received_json.contains("method")) {
            string method = received_json.value("method", "");

            if (method == "subscription" && isStreaming) {
                auto params = received_json.value("params", json{});
                auto data = params.value("data", json{});

                if (!data.is_null() && data.is_object()) {
                    utils::clear_console();
                    fmt::print(fmt::fg(fmt::color::blue) | fmt::emphasis::bold,
                        "> (Press q to stop streaming)\n\n");
                    cout << "Subscription Data: " << data.dump(4) << endl;

                    if (data.contains("price") && data["price"].is_number() &&
                        data.contains("timestamp") && data["timestamp"].is_number() &&
                        data.contains("index_name") && data["index_name"].is_string()) {
                        double price = data["price"];
                        int64_t timestamp = data["timestamp"];
                        string index_name = data["index_name"];

                        fmt::print(fmt::fg(fmt::color::green) | fmt::emphasis::bold,
                                   "Price: {} ", price);
                        fmt::print(fmt::fg(fmt::color::yellow),
                                   "Timestamp: {} ", timestamp);
                        fmt::print(fmt::fg(fmt::color::cyan),
                                   "Index: {}\n", index_name);
                    } else {
                        cerr << "Unexpected data format" << endl;
                    }
                } else {
                    cerr << "Invalid or null data received" << endl;
                }
            }
        }
        if(!isStreaming){
            if (msg->get_opcode() == websocketpp::frame::opcode::text) {
                m_messages.push_back("RECEIVED: " + msg->get_payload());
            record_summary(msg->get_payload(), "RECEIVED");
            } else {
                m_messages.push_back("RECEIVED: " + websocketpp::utility::to_hex(msg->get_payload()));
            record_summary(websocketpp::utility::to_hex(msg->get_payload()), "RECEIVED");
            }
            if (msg->get_payload()[0] == '{') {
                cout << "Received message: " << utils::pretty(msg->get_payload()) << endl;
            }
            else{
                cout << "Received message: " << msg->get_payload() << endl;
            }
        }

        if (AUTH_SENT && received_json.contains("result") && 
            received_json["result"].contains("access_token")) {
            Password::password().setAccessToken(received_json["result"]["access_token"]);
            utils::printcmd("Authorization successful!\n");
            AUTH_SENT = false;
        }

        MSG_PROCESSED = true;
        cv.notify_one();
    }
    catch (const exception& e) {
        cerr << "Error processing message: " << e.what() << endl;
        MSG_PROCESSED = true;
        cv.notify_one();
    }

    // Stop latency tracking
    getLatencyTracker().stop_measurement(
        LatencyTracker::WEBSOCKET_MESSAGE_PROPAGATION, 
        "websocket_message_" + to_string(m_id)
    );
}

int websocket_endpoint::streamSubscriptions(const vector<string>& connections) {
    if (connections.empty()) {
        cout << "No subscriptions to stream." << endl;
        return -1;
    }

    json subscribe = {
        {"jsonrpc", "2.0"},
        {"id", 4235},
        {"method", "private/subscribe"},
        {"params", {
            {"channels", connections}
        }}
    };
    
    isStreaming = true;
    
    if (!m_connection_list.empty()) {
        int connectionId = m_connection_list.begin()->first;
        
        send(connectionId, subscribe.dump());
        
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        
        int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        
        fmt::print(fmt::fg(fmt::color::blue) | fmt::emphasis::bold,
                "> Streaming... Press 'q' to quit.\n");
        while(isStreaming) {
            // Check for 'q' key press
            char ch;
            if (read(STDIN_FILENO, &ch, 1) > 0) {
                if (ch == 'q' || ch == 'Q') {
                    isStreaming = false;
                    
                    // Unsubscribe
                    json unsubscribe = {
                        {"jsonrpc", "2.0"},
                        {"id", 154},
                        {"method", "private/unsubscribe_all"},
                        {"params", {}}
                    };
                    
                    send(connectionId, unsubscribe.dump());
                    break;
                }
            }
            
            // Prevent busy waiting
            this_thread::sleep_for(chrono::milliseconds(1));
        }
        
        // Restore terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        
        fmt::print(fmt::fg(fmt::color::cyan) | fmt::emphasis::bold,
                "> Streaming stopped.\n");
    } else {
        fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold,
                "> No active connections to send subscribe message.\n");
        return -1;
    }
    
    return 0;
}

ostream &operator<< (ostream &out, connection_metadata const &data) {
    out << "> URI: " << data.m_uri << "\n"
        << "> Status: " << data.m_status << "\n"
        << "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
        << "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason) << "\n"
        << "> Messages Processed: (" << data.m_messages.size() << ") \n";
 
    vector<string>::const_iterator it;
    for (it = data.m_summaries.begin(); it != data.m_summaries.end(); ++it) {
        out << *it << "\n";
    }
    return out;
}

context_ptr on_tls_init() {
    context_ptr context = make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

    try {
        context->set_options(boost::asio::ssl::context::default_workarounds |
                        boost::asio::ssl::context::no_sslv2 |
                        boost::asio::ssl::context::no_sslv3 |
                        boost::asio::ssl::context::single_dh_use);
    } catch (exception &e) {
        cout << "Error in context pointer: " << e.what() << endl;
    }
    return context;
}

websocket_endpoint::websocket_endpoint(): m_next_id(0) {
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

    m_endpoint.init_asio();
    m_endpoint.start_perpetual();

    m_thread.reset(new websocketpp::lib::thread(&client::run, &m_endpoint));
}

websocket_endpoint::~websocket_endpoint() {
    m_endpoint.stop_perpetual();

    for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
        if (it->second->get_status() != "Open") {
            continue;
        }
        
        cout << "> Closing connection " << it->second->get_id() << endl;
        
        websocketpp::lib::error_code ec;
        m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
        if (ec) {
            cout << "> Error closing connection " << it->second->get_id() << ": "  
                    << ec.message() << endl;
        }
    }
    
    m_thread->join();
}

int websocket_endpoint::connect(string const &uri) {
    int new_id = m_next_id++;

    m_endpoint.set_tls_init_handler(websocketpp::lib::bind(
                                    &on_tls_init
                                    ));

    websocketpp::lib::error_code ec;
    client::connection_ptr con = m_endpoint.get_connection(uri, ec);

    if(ec){
        cout << "Connection initialization error: " << ec.message() << endl;
        return -1;
    }

    connection_metadata::ptr metadata_ptr(new connection_metadata(new_id, con->get_handle(), uri, this));
    m_connection_list[new_id] = metadata_ptr;

    con->set_open_handler(websocketpp::lib::bind(
                          &connection_metadata::on_open,
                          metadata_ptr,
                          &m_endpoint,
                          websocketpp::lib::placeholders::_1
                          ));

    con->set_fail_handler(websocketpp::lib::bind(
                          &connection_metadata::on_fail,
                          metadata_ptr,
                          &m_endpoint,
                          websocketpp::lib::placeholders::_1
                          ));
    con->set_close_handler(websocketpp::lib::bind(
                           &connection_metadata::on_close,
                           metadata_ptr,
                           &m_endpoint,
                           websocketpp::lib::placeholders::_1
                          ));
    con->set_message_handler(websocketpp::lib::bind(
                             &connection_metadata::on_message,
                             metadata_ptr,
                             websocketpp::lib::placeholders::_1,
                             websocketpp::lib::placeholders::_2
                            ));

    m_endpoint.connect(con);

    return new_id;
}

connection_metadata::ptr websocket_endpoint::get_metadata(int id) const {
    con_list::const_iterator it = m_connection_list.find(id);
    if (it == m_connection_list.end()) {
        return connection_metadata::ptr(); // Return null/empty pointer if not found
    }
    return it->second;
}

void websocket_endpoint::close(int id, websocketpp::close::status::value code, string reason) {
    websocketpp::lib::error_code ec;
    
    con_list::iterator it = m_connection_list.find(id);
    if (it == m_connection_list.end()) {
        cout << "> No connection found with id " << id << endl;
        return;
    }
    
    m_endpoint.close(it->second->get_hdl(), code, reason, ec);
    if (ec) {
        cout << "> Error closing connection " << id << ": "  
                  << ec.message() << endl;
    }
}

int websocket_endpoint::send(int id, string message) {
    websocketpp::lib::error_code ec;
    
    con_list::iterator it = m_connection_list.find(id);
    if (it == m_connection_list.end()) {
        cout << "> No connection found with id " << id << endl;
        return -1;
    }
    
    m_endpoint.send(it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);
    
    if (ec) {
        cout << "> Error sending message to connection " << id << ": "  
                  << ec.message() << endl;
        return -1;
    }
    
    it->second->record_sent_message(message);
    return 0;
}