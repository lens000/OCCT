#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>

int main() {
    std::cout << "=== 获取毫秒精度的当前时间 ===" << std::endl;
    
    // 方法1: 获取毫秒级Unix时间戳
    auto now = std::chrono::system_clock::now();
    auto milliseconds_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    
    std::cout << "1. 毫秒时间戳: " << milliseconds_timestamp << std::endl;
    
    // 方法2: 分别获取秒和毫秒部分
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()
    ).count();
    
    auto milliseconds_part = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count() % 1000;
    
    std::cout << "2. 秒部分: " << seconds << ", 毫秒部分: " << milliseconds_part << std::endl;
    
    // 方法3: 转换为本地时间并显示毫秒
    std::time_t time_t_seconds = std::chrono::system_clock::to_time_t(now);
    std::tm* local_time = std::localtime(&time_t_seconds);
    
    std::cout << "3. 格式化时间 (毫秒): " 
              << std::setfill('0') 
              << std::setw(4) << (local_time->tm_year + 1900) << "-"
              << std::setw(2) << (local_time->tm_mon + 1) << "-"
              << std::setw(2) << local_time->tm_mday << " "
              << std::setw(2) << local_time->tm_hour << ":"
              << std::setw(2) << local_time->tm_min << ":"
              << std::setw(2) << local_time->tm_sec << "."
              << std::setw(3) << milliseconds_part << std::endl;
    
    // 方法4: 自定义毫秒格式 (YYYYMMDDHHMMSSMMM)
    long long custom_format = (long long)(local_time->tm_year + 1900) * 10000000000000LL +
                              (long long)(local_time->tm_mon + 1) * 100000000000LL +
                              (long long)local_time->tm_mday * 1000000000LL +
                              (long long)local_time->tm_hour * 10000000LL +
                              (long long)local_time->tm_min * 100000LL +
                              (long long)local_time->tm_sec * 1000LL +
                              milliseconds_part;
    
    std::cout << "4. 自定义格式 (YYYYMMDDHHMMSSMMM): " << custom_format << std::endl;
    
    // 方法5: 使用高精度时钟
    auto high_res_now = std::chrono::high_resolution_clock::now();
    auto high_res_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        high_res_now.time_since_epoch()
    ).count();
    
    std::cout << "5. 高精度时钟毫秒时间戳: " << high_res_ms << std::endl;
    
    // 演示连续获取几次毫秒时间，显示精度
    std::cout << "\n=== 毫秒精度演示 ===" << std::endl;
    for (int i = 0; i < 3; i++) {
        auto current = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            current.time_since_epoch()
        ).count();
        std::cout << "第" << (i+1) << "次获取: " << ms << std::endl;
        
        // 短暂延迟以显示毫秒差异
        std::chrono::milliseconds delay(10);
        std::this_thread::sleep_for(delay);
    }
    
    return 0;
}