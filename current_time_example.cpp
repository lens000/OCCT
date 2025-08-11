#include <iostream>
#include <chrono>
#include <ctime>

int main() {
    std::cout << "=== C++中获取当前时间的整数表示 ===" << std::endl;
    
    // 方法1: 使用 std::time() 获取Unix时间戳 (秒)
    std::time_t unix_timestamp = std::time(nullptr);
    std::cout << "1. Unix时间戳 (秒): " << unix_timestamp << std::endl;
    
    // 方法2: 使用 std::chrono 获取Unix时间戳 (秒)
    auto now = std::chrono::system_clock::now();
    auto unix_time_chrono = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()
    ).count();
    std::cout << "2. std::chrono Unix时间戳 (秒): " << unix_time_chrono << std::endl;
    
    // 方法3: 获取毫秒级时间戳
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    std::cout << "3. 毫秒时间戳: " << milliseconds << std::endl;
    
    // 方法4: 获取微秒级时间戳
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()
    ).count();
    std::cout << "4. 微秒时间戳: " << microseconds << std::endl;
    
    // 方法5: 获取纳秒级时间戳
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()
    ).count();
    std::cout << "5. 纳秒时间戳: " << nanoseconds << std::endl;
    
    // 方法6: 将时间转换为本地时间的各个组件
    std::time_t t = std::time(nullptr);
    std::tm* local_time = std::localtime(&t);
    
    std::cout << "\n=== 本地时间组件 ===" << std::endl;
    std::cout << "年: " << (local_time->tm_year + 1900) << std::endl;
    std::cout << "月: " << (local_time->tm_mon + 1) << std::endl;
    std::cout << "日: " << local_time->tm_mday << std::endl;
    std::cout << "时: " << local_time->tm_hour << std::endl;
    std::cout << "分: " << local_time->tm_min << std::endl;
    std::cout << "秒: " << local_time->tm_sec << std::endl;
    
    // 方法7: 自定义格式的整数时间 (YYYYMMDDHHMMSS)
    long long date_time_int = (long long)(local_time->tm_year + 1900) * 10000000000LL +
                              (long long)(local_time->tm_mon + 1) * 100000000LL +
                              (long long)local_time->tm_mday * 1000000LL +
                              (long long)local_time->tm_hour * 10000LL +
                              (long long)local_time->tm_min * 100LL +
                              (long long)local_time->tm_sec;
    std::cout << "\n7. 自定义格式时间 (YYYYMMDDHHMMSS): " << date_time_int << std::endl;
    
    return 0;
}