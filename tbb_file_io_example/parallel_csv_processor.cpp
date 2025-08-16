#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/parallel_sort.h>
#include <tbb/parallel_reduce.h>

struct CSVRow {
    std::vector<std::string> fields;
    
    CSVRow() = default;
    CSVRow(const std::vector<std::string>& f) : fields(f) {}
    
    std::string toString() const {
        std::string result;
        for (size_t i = 0; i < fields.size(); ++i) {
            if (i > 0) result += ",";
            result += fields[i];
        }
        return result;
    }
};

class ParallelCSVProcessor {
private:
    std::string input_filename;
    std::string output_filename;
    char delimiter;
    
public:
    ParallelCSVProcessor(const std::string& input, const std::string& output, char delim = ',') 
        : input_filename(input), output_filename(output), delimiter(delim) {}
    
    // 并行读取CSV文件
    std::vector<CSVRow> readCSVParallel() {
        std::cout << "开始并行读取CSV文件..." << std::endl;
        
        std::ifstream file(input_filename);
        if (!file.is_open()) {
            std::cerr << "无法打开CSV文件: " << input_filename << std::endl;
            return {};
        }
        
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
        
        std::vector<CSVRow> rows(lines.size());
        
        // 并行解析CSV行
        tbb::parallel_for(tbb::blocked_range<size_t>(0, lines.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    rows[i] = parseCSVLine(lines[i]);
                }
            });
        
        std::cout << "读取了 " << rows.size() << " 行CSV数据" << std::endl;
        return rows;
    }
    
    // 并行写入CSV文件
    void writeCSVParallel(const std::vector<CSVRow>& rows) {
        std::cout << "开始并行写入CSV文件..." << std::endl;
        
        std::ofstream file(output_filename);
        if (file.is_open()) {
            for (const auto& row : rows) {
                file << row.toString() << std::endl;
            }
            file.close();
        }
        std::cout << "写入完成" << std::endl;
    }
    
    // 并行CSV数据排序
    void sortCSVParallel(std::vector<CSVRow>& rows, size_t column_index) {
        std::cout << "开始并行排序CSV数据..." << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // 使用TBB并行排序
        tbb::parallel_sort(rows.begin(), rows.end(),
            [column_index](const CSVRow& a, const CSVRow& b) {
                if (column_index >= a.fields.size() || column_index >= b.fields.size()) {
                    return false;
                }
                return a.fields[column_index] < b.fields[column_index];
            });
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "排序完成，耗时: " << duration.count() << " ms" << std::endl;
    }
    
    // 并行CSV数据聚合
    std::unordered_map<std::string, double> aggregateCSVParallel(const std::vector<CSVRow>& rows,
                                                                 size_t key_column,
                                                                 size_t value_column) {
        std::cout << "开始并行聚合CSV数据..." << std::endl;
        
        tbb::concurrent_unordered_map<std::string, double> concurrent_aggregates;
        
        tbb::parallel_for(tbb::blocked_range<size_t>(0, rows.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    if (key_column < rows[i].fields.size() && value_column < rows[i].fields.size()) {
                        try {
                            double value = std::stod(rows[i].fields[value_column]);
                            std::string key = rows[i].fields[key_column];
                            
                            auto it = concurrent_aggregates.find(key);
                            if (it != concurrent_aggregates.end()) {
                                it->second += value;
                            } else {
                                concurrent_aggregates.insert({key, value});
                            }
                        } catch (...) {
                            // 忽略无法转换的值
                        }
                    }
                }
            });
        
        std::unordered_map<std::string, double> result(concurrent_aggregates.begin(), 
                                                      concurrent_aggregates.end());
        
        std::cout << "聚合完成，共 " << result.size() << " 个唯一键" << std::endl;
        return result;
    }
    
private:
    CSVRow parseCSVLine(const std::string& line) {
        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;
        
        while (std::getline(ss, field, delimiter)) {
            fields.push_back(field);
        }
        
        return CSVRow(fields);
    }
};

int main() {
    std::cout << "=== TBB并行CSV处理示例 ===" << std::endl;
    
    // 创建测试CSV文件
    std::string test_csv = "test_data.csv";
    std::ofstream test_file(test_csv);
    test_file << "ID,Name,Age,Salary,Department\n";
    for (int i = 0; i < 100000; ++i) {
        test_file << i << ",员工" << i << "," << (20 + (i % 50)) << "," 
                  << (3000 + (i % 10000)) << ",部门" << (i % 10) << "\n";
    }
    test_file.close();
    
    ParallelCSVProcessor processor(test_csv, "output.csv");
    
    // 读取CSV文件
    std::cout << "\n1. 读取CSV文件..." << std::endl;
    auto rows = processor.readCSVParallel();
    
    // 排序数据
    std::cout << "\n2. 排序数据..." << std::endl;
    processor.sortCSVParallel(rows, 2); // 按年龄排序
    
    // 聚合数据
    std::cout << "\n3. 聚合数据..." << std::endl;
    auto aggregates = processor.aggregateCSVParallel(rows, 4, 3); // 按部门聚合薪资
    
    // 写入结果
    std::cout << "\n4. 写入结果..." << std::endl;
    processor.writeCSVParallel(rows);
    
    // 清理测试文件
    std::remove(test_csv.c_str());
    std::remove("output.csv");
    
    std::cout << "\n=== 示例完成 ===" << std::endl;
    return 0;
}