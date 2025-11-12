#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>
#include <atomic>
#include <iomanip>

#include <messagebus.h>

class PerformancePublisher
{
public:
    PerformancePublisher(const std::string& app_id)
    {
        MessageBus::Config config;
        config.app_id = "test_app";
        config.host = "localhost";
        config.port = 1883;
        config.auto_reconnect = true;

        _bus = std::make_unique<MessageBus>(config);

        // 设置错误处理
        _bus->set_error_handler([this](const std::string& error) {
            std::cerr << "Publisher error: " << error << std::endl;
        });
        
        // 设置连接状态回调
        _bus->set_connection_handler([this](bool connected) {
            if (connected) {
                std::cout << "Publisher connected to broker" << std::endl;
                _connected = true;
            } else {
                std::cout << "Publisher disconnected from broker" << std::endl;
                _connected = false;
            }
        });
    }

    bool start()
    {
        if (!_bus->connect())
        {
            std::cerr << "[Publisher][Error] failed to connect to broker" << std::endl;
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return true;
    }

    void run_test(int message_size, double interval_seconds, int total_messages)
    {
        if (!_connected)
        {
            std::cerr << "[Publisher][Error] failed to connect to broker" << std::endl;
            return;
        }

        std::vector<char> test_data(message_size, 'A');
        std::string test_payload(test_data.begin(), test_data.end());

        std::cout << "Starting performance test:" << std::endl;
        std::cout << "  Message size: " << message_size << " bytes" << std::endl;
        std::cout << "  Interval: " << interval_seconds * 1000 << " ms" << std::endl;
        std::cout << "  Total messages: " << total_messages << std::endl;
        std::cout << "  Expected throughput: " 
                  << std::fixed << std::setprecision(2) 
                  << (message_size * total_messages) / (interval_seconds * total_messages * 1024.0) 
                  << " KB/s" << std::endl;

        auto start_time = std::chrono::high_resolution_clock::now();
        int messages_sent = 0;

        for (int i = 0; i < total_messages; ++i) 
        {
            auto message_start = std::chrono::high_resolution_clock::now();
            
            // 在消息中包含时间戳和序列号
            std::string payload = std::to_string(i) + "|" + 
                                 std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(
                                     message_start.time_since_epoch()).count()) + "|" + 
                                 test_payload;
            
            if (_bus->publish("test/performance", payload, MessageBus::QoS::AT_LEAST_ONCE)) {
                messages_sent++;
                
                // 计算实际发送时间
                auto message_end = std::chrono::high_resolution_clock::now();
                auto send_duration = std::chrono::duration_cast<std::chrono::microseconds>(
                    message_end - message_start);
                
                _total_send_time += send_duration.count();
                
                // 显示进度
                if (i % 100 == 0) {
                    std::cout << "Sent message " << i << "/" << total_messages << std::endl;
                }
            } else {
                std::cerr << "Failed to send message " << i << std::endl;
            }
            
            // 等待到下一个间隔
            auto next_message_time = message_start + 
                                   std::chrono::microseconds(static_cast<int64_t>(interval_seconds * 1000000));
            std::this_thread::sleep_until(next_message_time);
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        // 输出性能统计
        std::cout << "\n=== Performance Results ===" << std::endl;
        std::cout << "Total messages sent: " << messages_sent << std::endl;
        std::cout << "Total time: " << total_duration.count() / 1000.0 << " ms" << std::endl;
        std::cout << "Average send time: " << _total_send_time / messages_sent << " μs" << std::endl;
        std::cout << "Actual throughput: " 
                  << std::fixed << std::setprecision(2) 
                  << (messages_sent * message_size) / (total_duration.count() / 1000000.0) / 1024.0 
                  << " KB/s" << std::endl;
        std::cout << "Message rate: " << messages_sent / (total_duration.count() / 1000000.0) << " msg/s" << std::endl;
    }

private:
    std::unique_ptr<MessageBus> _bus;
    std::atomic<bool> _connected{false};
    int64_t _total_send_time{0};
};


int main() {
    PerformancePublisher publisher("performance_publisher");
    
    if (publisher.start()) {
        
        publisher.run_test(50, 0.01, 1000);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    
    return 0;
}