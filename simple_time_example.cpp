#include <iostream>
#include <chrono>
#include <ctime>

int main() {
    // 最常用的方法: 获取Unix时间戳 (自1970年1月1日以来的秒数)
    std::time_t timestamp = std::time(nullptr);
    std::cout << "Unix时间戳: " << timestamp << std::endl;
    
    // 使用现代C++方式获取毫秒时间戳
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    std::cout << "毫秒时间戳: " << ms << std::endl;
    
    return 0;
}