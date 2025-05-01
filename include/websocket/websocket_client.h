#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <map>
#include <string>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>

#include <websocketpp/config/asio_client.hpp> 
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp> 
#include <websocketpp/client.hpp> 

#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

extern bool AUTH_SENT;
extern bool isStreaming;

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef shared_ptr<boost::asio::ssl::context> context_ptr;

class websocket_endpoint;

class connection_metadata {
private:
    int m_id;
    websocketpp::connection_hdl m_hdl;
    string m_status;
    string m_uri;
    string m_server;
    string m_error_reason;
    vector<string> m_summaries;

    websocket_endpoint* m_endpoint;

public:
    typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;

    mutex mtx;
    condition_variable cv;
    vector<string> m_messages;
    bool MSG_PROCESSED;

    connection_metadata(int id, websocketpp::connection_hdl hdl, string uri, websocket_endpoint* endpoint = nullptr);

    int get_id();
    websocketpp::connection_hdl get_hdl();
    string get_status();
    void record_sent_message(string const &message);
    void record_summary(string const &message, string const &sent);

    void on_open(client * c, websocketpp::connection_hdl hdl);
    void on_fail(client * c, websocketpp::connection_hdl hdl);
    void on_close(client * c, websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg);

    friend ostream &operator<< (ostream &out, connection_metadata const &data);
};

context_ptr on_tls_init();

class websocket_endpoint {
private:
    typedef map<int, connection_metadata::ptr> con_list;

    client m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

    con_list m_connection_list;
    int m_next_id;

public:
    websocket_endpoint();
    ~websocket_endpoint();

    int connect(string const &uri);
    connection_metadata::ptr get_metadata(int id) const;
    void close(int id, websocketpp::close::status::value code, string reason);
    int send(int id, string message);
    int streamSubscriptions(const vector<string>& connections);
};

#endif // WEBSOCKET_CLIENT_H