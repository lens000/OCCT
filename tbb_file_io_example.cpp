#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_sort.h>
#include <tbb/concurrent_vector.h>
#include <tbb/blocked_range.h>

class FileChunkReader {
private:
    std::string filename;
    size_t chunkSize;
    
public:
    FileChunkReader(const std::string& fname, size_t chunk = 1024 * 1024) 
        : filename(fname), chunkSize(chunk) {}
    
    // 并行读取文件块
    std::vector<std::string> readFileChunks() {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法打开文件: " << filename << std::endl;
            return {};
        }
        
        // 获取文件大小
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // 计算块数
        size_t numChunks = (fileSize + chunkSize - 1) / chunkSize;
        std::vector<std::string> chunks(numChunks);
        
        // 并行读取块
        tbb::parallel_for(tbb::blocked_range<size_t>(0, numChunks),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    size_t startPos = i * chunkSize;
                    size_t currentChunkSize = std::min(chunkSize, fileSize - startPos);
                    
                    std::ifstream chunkFile(filename);
                    chunkFile.seekg(startPos);
                    
                    std::string chunk(currentChunkSize, '\0');
                    chunkFile.read(&chunk[0], currentChunkSize);
                    chunks[i] = chunk;
                }
            });
        
        return chunks;
    }
    
    // 并行计算行数
    size_t countLines() {
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
    
    // 并行搜索关键词
    std::vector<size_t> searchKeyword(const std::string& keyword) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法打开文件: " << filename << std::endl;
            return {};
        }
        
        // 获取文件大小
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // 计算块数
        size_t numChunks = (fileSize + chunkSize - 1) / chunkSize;
        
        // 使用并发向量存储结果
        tbb::concurrent_vector<size_t> positions;
        
        // 并行搜索关键词
        tbb::parallel_for(tbb::blocked_range<size_t>(0, numChunks),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    size_t startPos = i * chunkSize;
                    size_t currentChunkSize = std::min(chunkSize, fileSize - startPos);
                    
                    std::ifstream chunkFile(filename);
                    chunkFile.seekg(startPos);
                    
                    std::string chunk(currentChunkSize, '\0');
                    chunkFile.read(&chunk[0], currentChunkSize);
                    
                    // 在当前块中搜索关键词
                    size_t pos = 0;
                    while ((pos = chunk.find(keyword, pos)) != std::string::npos) {
                        positions.push_back(startPos + pos);
                        pos += keyword.length();
                    }
                }
            });
        
        // 转换为普通向量并排序
        std::vector<size_t> result(positions.begin(), positions.end());
        tbb::parallel_sort(result.begin(), result.end());
        
        return result;
    }
    
    // 并行排序文件内容
    std::vector<std::string> sortFileLines() {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法打开文件: " << filename << std::endl;
            return {};
        }
        
        std::vector<std::string> lines;
        std::string line;
        
        // 读取所有行
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        
        // 并行排序
        tbb::parallel_sort(lines.begin(), lines.end());
        
        return lines;
    }
};

int main() {
    // 创建测试文件
    std::ofstream testFile("test_data.txt");
    for (int i = 0; i < 100000; ++i) {
        testFile << "这是第 " << i << " 行测试数据，包含一些关键词如：TBB、并行、文件、读写\n";
    }
    testFile.close();
    
    FileChunkReader reader("test_data.txt");
    
    // 测试并行读取
    auto start = std::chrono::high_resolution_clock::now();
    auto chunks = reader.readFileChunks();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "并行读取完成，块数: " << chunks.size() << "，耗时: " << duration.count() << "ms" << std::endl;
    
    // 测试并行行数计算
    start = std::chrono::high_resolution_clock::now();
    size_t lineCount = reader.countLines();
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "文件总行数: " << lineCount << "，耗时: " << duration.count() << "ms" << std::endl;
    
    // 测试并行关键词搜索
    start = std::chrono::high_resolution_clock::now();
    auto positions = reader.searchKeyword("TBB");
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "关键词 'TBB' 出现次数: " << positions.size() << "，耗时: " << duration.count() << "ms" << std::endl;
    
    // 测试并行排序
    start = std::chrono::high_resolution_clock::now();
    auto sortedLines = reader.sortFileLines();
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "并行排序完成，行数: " << sortedLines.size() << "，耗时: " << duration.count() << "ms" << std::endl;
    
    // 清理测试文件
    std::remove("test_data.txt");
    
    return 0;
}