#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_sort.h>

class FileChunkReader {
private:
    std::string filename;
    size_t chunk_size;
    
public:
    FileChunkReader(const std::string& fname, size_t chunk_sz = 1024 * 1024) 
        : filename(fname), chunk_size(chunk_sz) {}
    
    // 并行读取文件块
    std::vector<std::string> readFileChunks() {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "无法打开文件: " << filename << std::endl;
            return {};
        }
        
        size_t file_size = file.tellg();
        file.close();
        
        size_t num_chunks = (file_size + chunk_size - 1) / chunk_size;
        std::vector<std::string> chunks(num_chunks);
        
        // 使用TBB并行读取文件块
        tbb::parallel_for(tbb::blocked_range<size_t>(0, num_chunks),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    chunks[i] = readChunk(i, file_size);
                }
            });
        
        return chunks;
    }
    
    // 并行统计文件中的行数
    size_t countLines() {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return 0;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        // 使用TBB并行统计行数
        return tbb::parallel_reduce(
            tbb::blocked_range<size_t>(0, content.length()),
            0,
            [&](const tbb::blocked_range<size_t>& range, size_t init) {
                size_t count = init;
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    if (content[i] == '\n') count++;
                }
                return count;
            },
            std::plus<size_t>()
        );
    }
    
    // 并行搜索文件中的关键词
    std::vector<size_t> searchKeyword(const std::string& keyword) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return {};
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        tbb::concurrent_vector<size_t> positions;
        
        // 使用TBB并行搜索关键词
        tbb::parallel_for(tbb::blocked_range<size_t>(0, content.length() - keyword.length() + 1),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    if (content.substr(i, keyword.length()) == keyword) {
                        positions.push_back(i);
                    }
                }
            });
        
        std::vector<size_t> result(positions.begin(), positions.end());
        tbb::parallel_sort(result.begin(), result.end());
        return result;
    }
    
private:
    std::string readChunk(size_t chunk_index, size_t file_size) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return "";
        }
        
        size_t start_pos = chunk_index * chunk_size;
        size_t actual_chunk_size = std::min(chunk_size, file_size - start_pos);
        
        file.seekg(start_pos);
        std::string chunk(actual_chunk_size, '\0');
        file.read(&chunk[0], actual_chunk_size);
        file.close();
        
        return chunk;
    }
};

int main() {
    std::cout << "=== TBB并行文件读取示例 ===" << std::endl;
    
    // 创建测试文件
    std::string test_filename = "test_data.txt";
    std::ofstream test_file(test_filename);
    for (int i = 0; i < 1000000; ++i) {
        test_file << "这是第 " << i << " 行测试数据，包含一些关键词如：TBB、并行、文件读取\n";
    }
    test_file.close();
    
    FileChunkReader reader(test_filename);
    
    // 测试并行读取文件块
    std::cout << "\n1. 并行读取文件块..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    auto chunks = reader.readFileChunks();
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "读取完成，共 " << chunks.size() << " 个块，耗时: " << duration.count() << " ms" << std::endl;
    
    // 测试并行统计行数
    std::cout << "\n2. 并行统计行数..." << std::endl;
    start_time = std::chrono::high_resolution_clock::now();
    size_t line_count = reader.countLines();
    end_time = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "文件总行数: " << line_count << "，耗时: " << duration.count() << " ms" << std::endl;
    
    // 测试并行搜索关键词
    std::cout << "\n3. 并行搜索关键词..." << std::endl;
    start_time = std::chrono::high_resolution_clock::now();
    auto positions = reader.searchKeyword("TBB");
    end_time = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "关键词 'TBB' 出现 " << positions.size() << " 次，耗时: " << duration.count() << " ms" << std::endl;
    
    // 清理测试文件
    std::remove(test_filename.c_str());
    
    std::cout << "\n=== 示例完成 ===" << std::endl;
    return 0;
}