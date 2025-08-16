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
#include <tbb/parallel_pipeline.h>

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
    
    // 并行CSV数据过滤
    std::vector<CSVRow> filterCSVParallel(const std::vector<CSVRow>& rows, 
                                          size_t column_index, 
                                          const std::string& filter_value) {
        std::cout << "开始并行过滤CSV数据..." << std::endl;
        
        tbb::concurrent_vector<CSVRow> filtered_rows;
        
        tbb::parallel_for(tbb::blocked_range<size_t>(0, rows.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    if (column_index < rows[i].fields.size() && 
                        rows[i].fields[column_index] == filter_value) {
                        filtered_rows.push_back(rows[i]);
                    }
                }
            });
        
        std::vector<CSVRow> result(filtered_rows.begin(), filtered_rows.end());
        std::cout << "过滤完成，从 " << rows.size() << " 行减少到 " << result.size() << " 行" << std::endl;
        return result;
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
    
    // 并行CSV数据转换
    void transformCSVParallel(std::vector<CSVRow>& rows, size_t column_index,
                             const std::string& transformation_type) {
        std::cout << "开始并行转换CSV数据..." << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        if (transformation_type == "uppercase") {
            tbb::parallel_for(tbb::blocked_range<size_t>(0, rows.size()),
                [&](const tbb::blocked_range<size_t>& range) {
                    for (size_t i = range.begin(); i != range.end(); ++i) {
                        if (column_index < rows[i].fields.size()) {
                            std::transform(rows[i].fields[column_index].begin(),
                                         rows[i].fields[column_index].end(),
                                         rows[i].fields[column_index].begin(), ::toupper);
                        }
                    }
                });
        } else if (transformation_type == "lowercase") {
            tbb::parallel_for(tbb::blocked_range<size_t>(0, rows.size()),
                [&](const tbb::blocked_range<size_t>& range) {
                    for (size_t i = range.begin(); i != range.end(); ++i) {
                        if (column_index < rows[i].fields.size()) {
                            std::transform(rows[i].fields[column_index].begin(),
                                         rows[i].fields[column_index].end(),
                                         rows[i].fields[column_index].begin(), ::tolower);
                        }
                    }
                });
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "转换完成，耗时: " << duration.count() << " ms" << std::endl;
    }
    
    // 并行CSV数据统计
    void analyzeCSVParallel(const std::vector<CSVRow>& rows) {
        std::cout << "开始并行分析CSV数据..." << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // 统计每列的数据类型和基本信息
        if (rows.empty()) return;
        
        size_t num_columns = rows[0].fields.size();
        std::vector<ColumnStats> column_stats(num_columns);
        
        tbb::parallel_for(tbb::blocked_range<size_t>(0, num_columns),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t col = range.begin(); col != range.end(); ++col) {
                    analyzeColumn(rows, col, column_stats[col]);
                }
            });
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "分析完成，耗时: " << duration.count() << " ms" << std::endl;
        
        // 输出分析结果
        for (size_t i = 0; i < column_stats.size(); ++i) {
            std::cout << "列 " << i << ": " << column_stats[i].toString() << std::endl;
        }
    }
    
private:
    struct ColumnStats {
        size_t total_values = 0;
        size_t numeric_values = 0;
        size_t string_values = 0;
        size_t empty_values = 0;
        double min_numeric = std::numeric_limits<double>::max();
        double max_numeric = std::numeric_limits<double>::lowest();
        double sum_numeric = 0.0;
        
        std::string toString() const {
            std::ostringstream oss;
            oss << "总数: " << total_values 
                << ", 数值: " << numeric_values 
                << ", 字符串: " << string_values 
                << ", 空值: " << empty_values;
            
            if (numeric_values > 0) {
                oss << ", 数值范围: [" << min_numeric << ", " << max_numeric 
                    << "], 平均值: " << (sum_numeric / numeric_values);
            }
            return oss.str();
        }
    };
    
    CSVRow parseCSVLine(const std::string& line) {
        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;
        
        while (std::getline(ss, field, delimiter)) {
            fields.push_back(field);
        }
        
        return CSVRow(fields);
    }
    
    void analyzeColumn(const std::vector<CSVRow>& rows, size_t column_index, ColumnStats& stats) {
        for (const auto& row : rows) {
            stats.total_values++;
            
            if (column_index >= row.fields.size() || row.fields[column_index].empty()) {
                stats.empty_values++;
                continue;
            }
            
            try {
                double value = std::stod(row.fields[column_index]);
                stats.numeric_values++;
                stats.min_numeric = std::min(stats.min_numeric, value);
                stats.max_numeric = std::max(stats.max_numeric, value);
                stats.sum_numeric += value;
            } catch (...) {
                stats.string_values++;
            }
        }
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
    
    // 过滤数据
    std::cout << "\n2. 过滤数据..." << std::endl;
    auto filtered_rows = processor.filterCSVParallel(rows, 4, "部门0");
    
    // 排序数据
    std::cout << "\n3. 排序数据..." << std::endl;
    processor.sortCSVParallel(filtered_rows, 2); // 按年龄排序
    
    // 聚合数据
    std::cout << "\n4. 聚合数据..." << std::endl;
    auto aggregates = processor.aggregateCSVParallel(rows, 4, 3); // 按部门聚合薪资
    
    // 转换数据
    std::cout << "\n5. 转换数据..." << std::endl;
    processor.transformCSVParallel(rows, 1, "uppercase"); // 姓名转大写
    
    // 分析数据
    std::cout << "\n6. 分析数据..." << std::endl;
    processor.analyzeCSVParallel(rows);
    
    // 写入结果
    std::cout << "\n7. 写入结果..." << std::endl;
    processor.writeCSVParallel(filtered_rows);
    
    // 清理测试文件
    std::remove(test_csv.c_str());
    std::remove("output.csv");
    
    std::cout << "\n=== 示例完成 ===" << std::endl;
    return 0;
}