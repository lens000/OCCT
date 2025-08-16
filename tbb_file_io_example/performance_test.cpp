#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_sort.h>
#include <tbb/blocked_range.h>

// 串行处理函数
size_t countLinesSerial(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return 0;
    }
    
    size_t lineCount = 0;
    std::string line;
    while (std::getline(file, line)) {
        lineCount++;
    }
    
    return lineCount;
}

// TBB并行处理函数
size_t countLinesParallel(const std::string& filename, size_t chunkSize = 1024 * 1024) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return 0;
    }
    
    // 获取文件大小
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // 计算块数
    size_t numChunks = (fileSize + chunkSize - 1) / chunkSize;
    
    // 并行计算行数
    size_t totalLines = tbb::parallel_reduce(
        tbb::blocked_range<size_t>(0, numChunks),
        0,
        [&](const tbb::blocked_range<size_t>& range, size_t init) {
            size_t localLines = init;
            for (size_t i = range.begin(); i != range.end(); ++i) {
                size_t startPos = i * chunkSize;
                size_t currentChunkSize = std::min(chunkSize, fileSize - startPos);
                
                std::ifstream chunkFile(filename);
                chunkFile.seekg(startPos);
                
                std::string chunk(currentChunkSize, '\0');
                chunkFile.read(&chunk[0], currentChunkSize);
                
                // 计算这个块中的行数
                localLines += std::count(chunk.begin(), chunk.end(), '\n');
            }
            return localLines;
        },
        std::plus<size_t>()
    );
    
    return totalLines;
}

// 串行排序函数
std::vector<std::string> sortLinesSerial(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return {};
    }
    
    std::vector<std::string> lines;
    std::string line;
    
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    std::sort(lines.begin(), lines.end());
    return lines;
}

// TBB并行排序函数
std::vector<std::string> sortLinesParallel(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return {};
    }
    
    std::vector<std::string> lines;
    std::string line;
    
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    tbb::parallel_sort(lines.begin(), lines.end());
    return lines;
}

int main() {
    // 创建测试文件
    std::cout << "创建测试文件..." << std::endl;
    std::ofstream testFile("performance_test_data.txt");
    for (int i = 0; i < 500000; ++i) {
        testFile << "这是第 " << i << " 行测试数据，用于性能测试比较串行和并行处理的效率\n";
    }
    testFile.close();
    
    std::string filename = "performance_test_data.txt";
    
    std::cout << "\n=== 性能测试比较 ===\n" << std::endl;
    
    // 测试行数计算
    std::cout << "1. 行数计算性能比较:" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    size_t serialLines = countLinesSerial(filename);
    auto end = std::chrono::high_resolution_clock::now();
    auto serialDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    start = std::chrono::high_resolution_clock::now();
    size_t parallelLines = countLinesParallel(filename);
    end = std::chrono::high_resolution_clock::now();
    auto parallelDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "   串行处理: " << serialLines << " 行，耗时: " << serialDuration.count() << "ms" << std::endl;
    std::cout << "   并行处理: " << parallelLines << " 行，耗时: " << parallelDuration.count() << "ms" << std::endl;
    std::cout << "   加速比: " << (double)serialDuration.count() / parallelDuration.count() << "x" << std::endl;
    
    // 测试排序性能
    std::cout << "\n2. 排序性能比较:" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    auto serialSorted = sortLinesSerial(filename);
    end = std::chrono::high_resolution_clock::now();
    serialDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    start = std::chrono::high_resolution_clock::now();
    auto parallelSorted = sortLinesParallel(filename);
    end = std::chrono::high_resolution_clock::now();
    parallelDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "   串行排序: " << serialSorted.size() << " 行，耗时: " << serialDuration.count() << "ms" << std::endl;
    std::cout << "   并行排序: " << parallelSorted.size() << " 行，耗时: " << parallelDuration.count() << "ms" << std::endl;
    std::cout << "   加速比: " << (double)serialDuration.count() / parallelDuration.count() << "x" << std::endl;
    
    // 验证结果一致性
    std::cout << "\n3. 结果验证:" << std::endl;
    bool linesMatch = (serialLines == parallelLines);
    bool sortMatch = (serialSorted.size() == parallelSorted.size());
    
    if (linesMatch && sortMatch) {
        std::cout << "   ✓ 串行和并行处理结果一致" << std::endl;
    } else {
        std::cout << "   ✗ 结果不一致，请检查代码" << std::endl;
    }
    
    // 性能总结
    std::cout << "\n=== 性能总结 ===" << std::endl;
    double avgSpeedup = ((double)serialDuration.count() / parallelDuration.count() + 
                        (double)serialDuration.count() / parallelDuration.count()) / 2.0;
    std::cout << "平均加速比: " << avgSpeedup << "x" << std::endl;
    
    if (avgSpeedup > 1.5) {
        std::cout << "TBB并行处理显著提升了性能！" << std::endl;
    } else if (avgSpeedup > 1.1) {
        std::cout << "TBB并行处理有一定性能提升。" << std::endl;
    } else {
        std::cout << "对于此任务，并行处理开销可能超过了收益。" << std::endl;
    }
    
    // 清理测试文件
    std::remove("performance_test_data.txt");
    
    return 0;
}
