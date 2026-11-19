#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <numeric>
#include <algorithm>

// 1. MAP STEP: Converts data into Key-Value pairs
std::unordered_map<std::string, int> MapFunc(const std::string& text) {
    std::unordered_map<std::string, int> localMap;
    std::stringstream ss(text);
    std::string word;
    while (ss >> word) {
        localMap[word]++;
    }
    return localMap;
}

// 2. REDUCE STEP: Aggregates intermediate Key-Value pairs
std::unordered_map<std::string, int> ReduceFunc(
    std::unordered_map<std::string, int>& result, 
    const std::unordered_map<std::string, int>& localMap) {
    
    for (const auto& [key, value] : localMap) {
        result[key] += value;
    }
    return result;
}

int main() {
    // Input dataset split into chunks
    std::vector<std::string> chunks = {
        "hello world mapreduce",
        "hello c plus plus",
        "mapreduce parallel processing world"
    };

    // Store intermediate map outputs
    std::vector<std::unordered_map<std::string, int>> intermediateResults;
    
    // Execute Map phase 
    for (const auto& chunk : chunks) {
        intermediateResults.push_back(MapFunc(chunk));
    }

    // Execute Reduce phase
    std::unordered_map<std::string, int> finalCount;
    for (const auto& localMap : intermediateResults) {
        finalCount = ReduceFunc(finalCount, localMap);
    }

    // Output final results
    for (const auto& [word, count] : finalCount) {
        std::cout << word << ": " << count << "\n";
    }

    return 0;
}
