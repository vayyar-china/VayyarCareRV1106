#include "messagebus.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <iomanip>
#include <sstream>

class PerformanceSubscriber {
public:
    PerformanceSubscriber(const std::string& app_id) {
        MessageBus::Config config;
        config.app_id = app_id;
        config.host = "localhost";
        config.port = 1883;
        config.auto_reconnect = true;
        
        _bus = std::make_unique<MessageBus>(config);
        
        // 设置错误处理
        _bus->set_error_handler([this](const std::string& error) {
            std::cerr << "Subscriber error: " << error << std::endl;
        });
        
        // 设置连接状态回调
        _bus->set_connection_handler([this](bool connected) {
            if (connected) {
                std::cout << "Subscriber connected to broker" << std::endl;
                _connected = true;
            } else {
                std::cout << "Subscriber disconnected from broker" << std::endl;
                _connected = false;
            }
        });
    }
    
    bool start() {
        if (!_bus->connect()) {
            std::cerr << "Failed to connect to broker" << std::endl;
            return false;
        }
        
        // 等待连接建立
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 订阅测试主题
        _subscription = _bus->subscribe("test/performance", 
            [this](const std::string& topic, const std::string& payload) {
                this->on_message_received(topic, payload);
            }, MessageBus::QoS::AT_LEAST_ONCE);
        
        if (!_subscription) {
            std::cerr << "Failed to subscribe to test topic" << std::endl;
            return false;
        }
        
        std::cout << "Subscribed to test/performance" << std::endl;
        return true;
    }
    
    void on_message_received(const std::string& topic, const std::string& payload) {
        auto receive_time = std::chrono::high_resolution_clock::now();
        _total_messages_received++;
        
        // 解析消息：格式为 "序列号|时间戳|数据"
        std::istringstream ss(payload);
        std::string segment;
        std::vector<std::string> parts;
        
        while (std::getline(ss, segment, '|')) {
            parts.push_back(segment);
        }
        
        if (parts.size() >= 2) {
            int sequence_number = std::stoi(parts[0]);
            int64_t send_timestamp = std::stoll(parts[1]);
            
            // 计算延迟（从发送到接收的时间差）
            auto send_time = std::chrono::time_point<std::chrono::high_resolution_clock>(
                std::chrono::microseconds(send_timestamp));
            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
                receive_time - send_time);
            
            _total_latency += latency.count();
            _total_data_received += payload.size();
            
            // 更新最小/最大延迟
            if (latency.count() < _min_latency || _min_latency == 0) {
                _min_latency = latency.count();
            }
            if (latency.count() > _max_latency) {
                _max_latency = latency.count();
            }
            
            // 显示进度
            if (_total_messages_received % 100 == 0) {
                std::cout << "Received message " << _total_messages_received 
                          << " (seq: " << sequence_number << ")" << std::endl;
            }
        }
    }
    
    void print_statistics() {
        if (_total_messages_received == 0) {
            std::cout << "No messages received" << std::endl;
            return;
        }
        
        auto current_time = std::chrono::high_resolution_clock::now();
        auto test_duration = std::chrono::duration_cast<std::chrono::microseconds>(
            current_time - _test_start_time);
        
        std::cout << "\n=== Performance Statistics ===" << std::endl;
        std::cout << "Total messages received: " << _total_messages_received << std::endl;
        std::cout << "Total data received: " << _total_data_received << " bytes" << std::endl;
        std::cout << "Test duration: " << test_duration.count() / 1000.0 << " ms" << std::endl;
        std::cout << "Average latency: " << _total_latency / _total_messages_received << " μs" << std::endl;
        std::cout << "Min latency: " << _min_latency << " μs" << std::endl;
        std::cout << "Max latency: " << _max_latency << " μs" << std::endl;
        std::cout << "Throughput: " 
                  << std::fixed << std::setprecision(2) 
                  << (_total_data_received / (test_duration.count() / 1000000.0)) / 1024.0 
                  << " KB/s" << std::endl;
        std::cout << "Message rate: " 
                  << _total_messages_received / (test_duration.count() / 1000000.0) 
                  << " msg/s" << std::endl;
    }
    
    void wait_for_completion(int expected_messages, int timeout_seconds = 30) {
        std::cout << "Waiting for " << expected_messages << " messages..." << std::endl;
        
        _test_start_time = std::chrono::high_resolution_clock::now();
        auto start_time = _test_start_time;
        
        while (_total_messages_received < expected_messages) {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
            
            if (elapsed.count() > timeout_seconds) {
                std::cout << "Timeout waiting for messages. Received " 
                          << _total_messages_received << "/" << expected_messages << std::endl;
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            
            // 每5秒显示一次进度
            if (elapsed.count() % 5 == 0 && elapsed.count() > 0) {
                std::cout << "Progress: " << _total_messages_received << "/" 
                          << expected_messages << " messages (" 
                          << std::fixed << std::setprecision(1) 
                          << (static_cast<double>(_total_messages_received) / expected_messages * 100.0) 
                          << "%)" << std::endl;
            }
        }
        
        print_statistics();
    }
    
private:
    std::unique_ptr<MessageBus> _bus;
    MessageBus::SubscriptionPtr _subscription;
    std::atomic<bool> _connected{false};
    
    std::atomic<int> _total_messages_received{0};
    std::atomic<int64_t> _total_latency{0};
    std::atomic<int64_t> _min_latency{0};
    std::atomic<int64_t> _max_latency{0};
    std::atomic<size_t> _total_data_received{0};
    
    std::chrono::high_resolution_clock::time_point _test_start_time;
};

int main() {
    PerformanceSubscriber subscriber("performance_subscriber");
    
    if (subscriber.start()) {
  
        subscriber.wait_for_completion(1000, 120); // 2分钟超时
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    return 0;
}