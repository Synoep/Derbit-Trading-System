#ifndef LATENCY_TRACKER_H
#define LATENCY_TRACKER_H

#include <chrono>
#include <map>
#include <vector>
#include <mutex>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <iomanip>

using namespace std;

class LatencyTracker {
public:
    enum LatencyType {
        ORDER_PLACEMENT,
        MARKET_DATA_PROCESSING,
        WEBSOCKET_MESSAGE_PROPAGATION,
        TRADING_LOOP_END_TO_END
    };

    struct LatencyMetric {
        chrono::high_resolution_clock::time_point start_time;
        chrono::high_resolution_clock::time_point end_time;
        chrono::nanoseconds duration{0};
        bool completed{false};
    };

    void start_measurement(LatencyType type, const string& unique_id = "");

    void stop_measurement(LatencyType type, const string& unique_id = "");

    string generate_report();

    map<LatencyType, vector<LatencyMetric>> get_raw_metrics();

    void reset();

private:
    // Thread-safe collections for storing latency metrics
    mutex metrics_mutex;
    map<LatencyType, vector<LatencyMetric>> latency_metrics;
    map<string, LatencyMetric> active_measurements;
};

// Global singleton accessor
LatencyTracker& getLatencyTracker();

#endif // LATENCY_TRACKER_H