#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/concurrent_queue.h>
#include <tbb/parallel_pipeline.h>
#include <tbb/flow_graph.h>

class ParallelFileWriter {
private:
    std::string filename;
    size_t buffer_size;
    
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
    
    // 使用Flow Graph进行更复杂的并行写入
    void writeWithFlowGraph(size_t num_records) {
        std::cout << "使用Flow Graph并行写入 " << num_records << " 条记录..." << std::endl;
        
        tbb::flow::graph g;
        
        // 创建节点
        tbb::flow::source_node<size_t> source(g, 
            [num_records](size_t& index) -> bool {
                static size_t counter = 0;
                if (counter < num_records) {
                    index = counter++;
                    return true;
                }
                return false;
            }, true);
        
        tbb::flow::function_node<size_t, std::string> generator(g, tbb::flow::unlimited,
            [](size_t index) -> std::string {
                return generateRecord(index);
            });
        
        tbb::flow::function_node<std::string, std::string> processor(g, tbb::flow::unlimited,
            [](const std::string& record) -> std::string {
                return processRecord(record);
            });
        
        tbb::flow::function_node<std::string, void> writer(g, tbb::flow::unlimited,
            [this](const std::string& processed_record) {
                writeRecord(processed_record);
            });
        
        // 连接节点
        tbb::flow::make_edge(source, generator);
        tbb::flow::make_edge(generator, processor);
        tbb::flow::make_edge(processor, writer);
        
        // 启动图
        source.activate();
        g.wait_for_all();
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
    
    // 并行写入大文件块
    void writeLargeFileParallel(size_t file_size_mb) {
        std::cout << "并行写入 " << file_size_mb << " MB 大文件..." << std::endl;
        
        size_t num_blocks = (file_size_mb * 1024 * 1024) / buffer_size;
        std::vector<std::vector<char>> blocks(num_blocks);
        
        // 并行生成数据块
        tbb::parallel_for(tbb::blocked_range<size_t>(0, num_blocks),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    blocks[i] = generateDataBlock(i);
                }
            });
        
        // 并行写入文件块
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open()) {
            tbb::parallel_for(tbb::blocked_range<size_t>(0, num_blocks),
                [&](const tbb::blocked_range<size_t>& range) {
                    for (size_t i = range.begin(); i != range.end(); ++i) {
                        std::lock_guard<std::mutex> lock(file_mutex);
                        file.seekp(i * buffer_size);
                        file.write(blocks[i].data(), blocks[i].size());
                    }
                });
            file.close();
        }
    }
    
private:
    std::mutex file_mutex;
    
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
    
    std::vector<char> generateDataBlock(size_t block_index) {
        std::vector<char> block(buffer_size);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        for (size_t i = 0; i < block.size(); ++i) {
            block[i] = static_cast<char>(dis(gen));
        }
        
        return block;
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
    
    // 测试Flow Graph写入
    std::cout << "\n2. 测试Flow Graph写入..." << std::endl;
    ParallelFileWriter writer2("parallel_output2.txt");
    
    start_time = std::chrono::high_resolution_clock::now();
    writer2.writeWithFlowGraph(10000);
    end_time = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Flow Graph写入完成，耗时: " << duration.count() << " ms" << std::endl;
    
    // 测试多文件并行写入
    std::cout << "\n3. 测试多文件并行写入..." << std::endl;
    std::vector<std::string> filenames = {
        "file1.txt", "file2.txt", "file3.txt", "file4.txt"
    };
    
    ParallelFileWriter writer3("dummy.txt");
    start_time = std::chrono::high_resolution_clock::now();
    writer3.writeMultipleFiles(filenames, 1000);
    end_time = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "多文件并行写入完成，耗时: " << duration.count() << " ms" << std::endl;
    
    // 测试大文件并行写入
    std::cout << "\n4. 测试大文件并行写入..." << std::endl;
    ParallelFileWriter writer4("large_file.bin");
    
    start_time = std::chrono::high_resolution_clock::now();
    writer4.writeLargeFileParallel(10); // 10MB
    end_time = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "大文件并行写入完成，耗时: " << duration.count() << " ms" << std::endl;
    
    // 清理测试文件
    std::remove("parallel_output1.txt");
    std::remove("parallel_output2.txt");
    std::remove("large_file.bin");
    for (const auto& filename : filenames) {
        std::remove(filename.c_str());
    }
    
    std::cout << "\n=== 示例完成 ===" << std::endl;
    return 0;
}