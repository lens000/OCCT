#include <iostream>
#include <vector>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

int main() {
    std::vector<int> data(1000);
    
    // 初始化数据
    for (int i = 0; i < 1000; ++i) {
        data[i] = i;
    }
    
    // 使用TBB并行处理
    tbb::parallel_for(tbb::blocked_range<size_t>(0, data.size()),
        [&](const tbb::blocked_range<size_t>& range) {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                data[i] *= 2;
            }
        });
    
    std::cout << "TBB并行处理完成！" << std::endl;
    std::cout << "前5个元素: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
