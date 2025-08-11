#include <iostream>
#include <chrono>

// 获取当前时间的毫秒时间戳
long long getCurrentMilliseconds() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
}

int main() {
    // 获取当前毫秒时间戳
    long long milliseconds = getCurrentMilliseconds();
    
    std::cout << "当前毫秒时间戳: " << milliseconds << std::endl;
    
    // 多次调用显示毫秒精度
    std::cout << "\n连续获取毫秒时间戳:" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << "时间戳: " << getCurrentMilliseconds() << std::endl;
    }
    
    return 0;
}