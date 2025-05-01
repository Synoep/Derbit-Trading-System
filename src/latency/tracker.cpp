#include "latency/tracker.h"
#include "utils/utils.h"

using namespace std;

void LatencyTracker::start_measurement(LatencyType type, const string& unique_id) {
    lock_guard<mutex> lock(metrics_mutex);
    
    LatencyMetric metric;
    metric.start_time = chrono::high_resolution_clock::now();
    
    if (unique_id.empty()) {
        latency_metrics[type].push_back(metric);
    } else {
        active_measurements[unique_id] = metric;
    }
}

void LatencyTracker::stop_measurement(LatencyType type, const string& unique_id) {
    lock_guard<mutex> lock(metrics_mutex);
    auto end_time = chrono::high_resolution_clock::now();
    
    if (unique_id.empty()) {
        // Find the most recent uncompleted metric for this type
        for (auto& metric : latency_metrics[type]) {
            if (!metric.completed) {
                metric.end_time = end_time;
                metric.duration = chrono::duration_cast<chrono::nanoseconds>(
                    metric.end_time - metric.start_time
                );
                metric.completed = true;
                break;
            }
        }
    } else {
        // Find the specific measurement by unique ID
        auto it = active_measurements.find(unique_id);
        if (it != active_measurements.end()) {
            it->second.end_time = end_time;
            it->second.duration = chrono::duration_cast<chrono::nanoseconds>(
                it->second.end_time - it->second.start_time
            );
            it->second.completed = true;
            
            latency_metrics[type].push_back(it->second);
            active_measurements.erase(it);
        }
    }
}

string LatencyTracker::generate_report() {
    lock_guard<mutex> lock(metrics_mutex);
    
    int terminal_width = utils::getTerminalWidth();
    
    ostringstream report;
    
    // ANSI escape codes for colors
    const string reset_color = "\033[0m";
    const string header_color = "\033[1;36m"; // Bold Cyan
    const string section_color = "\033[1;32m"; // Bold Green
    const string metric_color = "\033[1;33m"; // Bold Yellow
    const string footer_color = "\033[1;34m"; // Bold Blue

    string header = "Latency Benchmarking Report";
    int padding_length = (terminal_width - header.length()) / 2;
    string padding(padding_length, '=');
    
    report << header_color << padding << header << padding << reset_color << "\n\n";

    const char* type_names[] = {
        "Order Placement",
        "Market Data Processing", 
        "WebSocket Message Propagation", 
        "Trading Loop End-to-End"
    };

    // Define column widths based on terminal width
    int type_col_width = 30;
    int metric_col_width = (terminal_width - type_col_width - 4) / 2;

    for (int type = 0; type < 4; ++type) {
        auto metrics = latency_metrics[static_cast<LatencyType>(type)];
        
        if (metrics.empty()) continue;

        // Calculate statistics
        vector<chrono::nanoseconds> durations;
        for (const auto& metric : metrics) {
            if (metric.completed) {
                durations.push_back(metric.duration);
            }
        }

        if (durations.empty()) {
            report << section_color << type_names[type] << reset_color
                   << " Latency: No completed measurements\n\n";
            continue;
        }

        sort(durations.begin(), durations.end());

        auto total_measurements = durations.size();
        auto mean_duration = accumulate(durations.begin(), durations.end(), 
            chrono::nanoseconds(0)) / total_measurements;

        auto percentile_50 = durations[total_measurements * 0.5];
        auto percentile_90 = durations[total_measurements * 0.9];
        auto percentile_99 = durations[total_measurements * 0.99];
        auto min_duration = durations.front();
        auto max_duration = durations.back();

        report << section_color << left << setw(type_col_width) << type_names[type] 
               << reset_color
               << right 
               << fixed << setprecision(3);
        
        // First column of metrics
        report << "  " << metric_color << "Meas: " << reset_color << setw(6) << total_measurements 
               << "  " << metric_color << "Mean: " << reset_color << setw(8) << mean_duration.count() / 1000.0 << " µs\n";
        
        // Padding for alignment
        report << string(type_col_width, ' ');
        
        // Second column of metrics
        report << "  " << metric_color << "Min:  " << reset_color << setw(8) << min_duration.count() / 1000.0 << " µs"
               << "  " << metric_color << "Max:  " << reset_color << setw(8) << max_duration.count() / 1000.0 << " µs\n";
        
        report << string(type_col_width, ' ')
               << "  " << metric_color << "50th: " << reset_color << setw(8) << percentile_50.count() / 1000.0 << " µs"
               << "  " << metric_color << "90th: " << reset_color << setw(8) << percentile_90.count() / 1000.0 << " µs"
               << "  " << metric_color << "99th: " << reset_color << setw(8) << percentile_99.count() / 1000.0 << " µs\n\n";
    }

    report << footer_color << string(terminal_width, '=') << reset_color << "\n";

    return report.str();
}


map<LatencyTracker::LatencyType, vector<LatencyTracker::LatencyMetric>> LatencyTracker::get_raw_metrics() {
    lock_guard<mutex> lock(metrics_mutex);
    return latency_metrics;
}

void LatencyTracker::reset() {
    lock_guard<mutex> lock(metrics_mutex);
    latency_metrics.clear();
    active_measurements.clear();

    int terminal_width = utils::getTerminalWidth();

    const string message = "Latency metrics have been reset.";
    
    int padding_length = (terminal_width - message.length()) / 2;
    string padding(padding_length, ' ');

    cout << padding << message << endl;
}


LatencyTracker& getLatencyTracker() {
    static LatencyTracker tracker;
    return tracker;
}