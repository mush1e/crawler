#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <string>
#include <regex>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace indexer {

    struct CompareSize {
        bool operator()(const std::pair<std::string, long long>& a, const std::pair<std::string, long long>& b) {
            return a.second < b.second;
        }
    };

    class Indexer {
    public:
        Indexer(std::string dump_dir  = "../raw_dump", std::string index_file  = "./index.csv") : dump_dir(dump_dir), index_file(index_file) {
            std::cout << "Indexer Initiated...." << std::endl;
        };

        void document_parser(std::string& file_name);
        void directory_spider();
        void index_updater(std::string& f_name);
        std::string url_extractor(std::string file_name);

    private:
        std::string dump_dir {};
        std::string index_file {};
        // Inverted Index; 
        std::unordered_map<std::string, std::priority_queue<std::pair<std::string, long long>, std::vector<std::pair<std::string, long long>>, CompareSize>> term_document_matrix;
        std::unordered_set<std::string> indexed_documents;
    };

}