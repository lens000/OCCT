#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_sort.h>
#include <tbb/parallel_reduce.h>

class ParallelFileProcessor {
private:
    std::string input_filename;
    std::string output_filename;
    
public:
    ParallelFileProcessor(const std::string& input, const std::string& output) 
        : input_filename(input), output_filename(output) {}
    
    // 并行读取、处理和写入文件
    void processFile() {
        std::cout << "开始并行处理文件..." << std::endl;
        
        // 并行读取文件
        auto lines = readFileParallel();
        std::cout << "读取了 " << lines.size() << " 行数据" << std::endl;
        
        // 并行处理数据
        auto processed_lines = processLinesParallel(lines);
        std::cout << "处理了 " << processed_lines.size() << " 行数据" << std::endl;
        
        // 并行写入文件
        writeFileParallel(processed_lines);
        std::cout << "写入完成" << std::endl;
    }
    
    // 并行文件排序
    void sortFileParallel() {
        std::cout << "开始并行文件排序..." << std::endl;
        
        auto lines = readFileParallel();
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // 使用TBB并行排序
        tbb::parallel_sort(lines.begin(), lines.end());
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "并行排序完成，耗时: " << duration.count() << " ms" << std::endl;
        
        writeFileParallel(lines);
    }
    
    // 并行文件统计
    void analyzeFileParallel() {
        std::cout << "开始并行文件分析..." << std::endl;
        
        auto lines = readFileParallel();
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // 并行统计各种信息
        auto stats = tbb::parallel_reduce(
            tbb::blocked_range<size_t>(0, lines.size()),
            FileStats(),
            [&](const tbb::blocked_range<size_t>& range, FileStats init) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    init.addLine(lines[i]);
                }
                return init;
            },
            [](const FileStats& a, const FileStats& b) {
                return a + b;
            }
        );
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "文件分析完成，耗时: " << duration.count() << " ms" << std::endl;
        std::cout << "总行数: " << stats.total_lines << std::endl;
        std::cout << "总字符数: " << stats.total_chars << std::endl;
        std::cout << "平均行长度: " << (stats.total_chars / (double)stats.total_lines) << std::endl;
        std::cout << "最长行: " << stats.max_line_length << " 字符" << std::endl;
        std::cout << "最短行: " << stats.min_line_length << " 字符" << std::endl;
    }
    
private:
    struct FileStats {
        size_t total_lines = 0;
        size_t total_chars = 0;
        size_t max_line_length = 0;
        size_t min_line_length = std::numeric_limits<size_t>::max();
        
        void addLine(const std::string& line) {
            total_lines++;
            total_chars += line.length();
            max_line_length = std::max(max_line_length, line.length());
            min_line_length = std::min(min_line_length, line.length());
        }
        
        FileStats operator+(const FileStats& other) const {
            FileStats result;
            result.total_lines = total_lines + other.total_lines;
            result.total_chars = total_chars + other.total_chars;
            result.max_line_length = std::max(max_line_length, other.max_line_length);
            result.min_line_length = std::min(min_line_length, other.min_line_length);
            return result;
        }
    };
    
    std::vector<std::string> readFileParallel() {
        std::ifstream file(input_filename);
        if (!file.is_open()) {
            std::cerr << "无法打开输入文件: " << input_filename << std::endl;
            return {};
        }
        
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
        
        return lines;
    }
    
    std::vector<std::string> processLinesParallel(const std::vector<std::string>& lines) {
        std::vector<std::string> processed_lines(lines.size());
        
        tbb::parallel_for(tbb::blocked_range<size_t>(0, lines.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    processed_lines[i] = processLine(lines[i]);
                }
            });
        
        return processed_lines;
    }
    
    void writeFileParallel(const std::vector<std::string>& lines) {
        std::ofstream file(output_filename);
        if (file.is_open()) {
            for (const auto& line : lines) {
                file << line << std::endl;
            }
            file.close();
        }
    }
    
    std::string processLine(const std::string& line) {
        // 模拟行处理
        std::string processed = "已处理: " + line;
        // 转换为大写
        std::transform(processed.begin(), processed.end(), processed.begin(), ::toupper);
        return processed;
    }
};

int main() {
    std::cout << "=== TBB并行文件处理示例 ===" << std::endl;
    
    // 创建测试文件
    std::string test_input = "test_input.txt";
    std::ofstream test_file(test_input);
    for (int i = 0; i < 100000; ++i) {
        test_file << "这是第 " << i << " 行测试数据，用于演示TBB并行处理能力\n";
    }
    test_file.close();
    
    ParallelFileProcessor processor(test_input, "test_output.txt");
    
    // 测试基本文件处理
    std::cout << "\n1. 测试基本文件处理..." << std::endl;
    processor.processFile();
    
    // 测试并行排序
    std::cout << "\n2. 测试并行排序..." << std::endl;
    processor.sortFileParallel();
    
    // 测试并行文件分析
    std::cout << "\n3. 测试并行文件分析..." << std::endl;
    processor.analyzeFileParallel();
    
    // 清理测试文件
    std::remove(test_input.c_str());
    std::remove("test_output.txt");
    
    std::cout << "\n=== 示例完成 ===" << std::endl;
    return 0;
}