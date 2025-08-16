#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <mutex>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_pipeline.h>

class ParallelFileWriter {
private:
    std::string filename;
    size_t buffer_size;
    std::mutex file_mutex;
    
public:
    ParallelFileWriter(const std::string& fname, size_t buf_sz = 1024 * 1024) 
        : filename(fname), buffer_size(buf_sz) {}
    
    // 并行生成数据并写入文件
    void writeParallelData(size_t num_records) {
        std::cout << "开始并行生成 " << num_records << " 条记录..." << std::endl;
        
        // 使用TBB并行管道处理数据
        tbb::parallel_pipeline(
            std::thread::hardware_concurrency(),
            // 第一阶段：生成数据
            tbb::make_filter<void, std::string>(
                tbb::filter::serial_in_order,
                [num_records](tbb::flow_control& fc) -> std::string {
                    static size_t counter = 0;
                    if (counter >= num_records) {
                        fc.stop();
                        return "";
                    }
                    
                    std::string record = generateRecord(counter++);
                    return record;
                }
            ) &
            // 第二阶段：处理数据
            tbb::make_filter<std::string, std::string>(
                tbb::filter::parallel,
                [](const std::string& record) -> std::string {
                    // 模拟数据处理
                    return processRecord(record);
                }
            ) &
            // 第三阶段：写入文件
            tbb::make_filter<std::string, void>(
                tbb::filter::serial_in_order,
                [this](const std::string& processed_record) {
                    writeRecord(processed_record);
                }
            )
        );
    }
    
    // 并行写入多个文件
    void writeMultipleFiles(const std::vector<std::string>& filenames, size_t records_per_file) {
        std::cout << "并行写入 " << filenames.size() << " 个文件..." << std::endl;
        
        tbb::parallel_for(tbb::blocked_range<size_t>(0, filenames.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    writeSingleFile(filenames[i], records_per_file);
                }
            });
    }
    
private:
    static std::string generateRecord(size_t index) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 1000);
        
        return "记录#" + std::to_string(index) + 
               ", 值: " + std::to_string(dis(gen)) + 
               ", 时间戳: " + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "\n";
    }
    
    static std::string processRecord(const std::string& record) {
        // 模拟数据处理
        return "已处理: " + record;
    }
    
    void writeRecord(const std::string& record) {
        std::lock_guard<std::mutex> lock(file_mutex);
        std::ofstream file(filename, std::ios::app);
        if (file.is_open()) {
            file << record;
            file.close();
        }
    }
    
    void writeSingleFile(const std::string& filename, size_t num_records) {
        std::ofstream file(filename);
        if (file.is_open()) {
            for (size_t i = 0; i < num_records; ++i) {
                file << generateRecord(i);
            }
            file.close();
        }
    }
};

int main() {
    std::cout << "=== TBB并行文件写入示例 ===" << std::endl;
    
    // 测试并行数据写入
    std::cout << "\n1. 测试并行管道写入..." << std::endl;
    ParallelFileWriter writer1("parallel_output1.txt");
    
    auto start_time = std::chrono::high_resolution_clock::now();
    writer1.writeParallelData(10000);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "并行管道写入完成，耗时: " << duration.count() << " ms" << std::endl;
    
    // 测试多文件并行写入
    std::cout << "\n2. 测试多文件并行写入..." << std::endl;
    std::vector<std::string> filenames = {
        "file1.txt", "file2.txt", "file3.txt", "file4.txt"
    };
    
    ParallelFileWriter writer2("dummy.txt");
    start_time = std::chrono::high_resolution_clock::now();
    writer2.writeMultipleFiles(filenames, 1000);
    end_time = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "多文件并行写入完成，耗时: " << duration.count() << " ms" << std::endl;
    
    // 清理测试文件
    std::remove("parallel_output1.txt");
    for (const auto& filename : filenames) {
        std::remove(filename.c_str());
    }
    
    std::cout << "\n=== 示例完成 ===" << std::endl;
    return 0;
}