#pragma once

#include "core/graph.hpp"
#include <string>
#include <memory>

namespace graph {

class GraphLoader {
public:
    // Загрузка из CSV формата: from,to,weight
    static std::unique_ptr<Graph> loadFromCSV(const std::string& filename, bool directed = false);
    
    // Загрузка из JSON формата
    static std::unique_ptr<Graph> loadFromJSON(const std::string& filename, bool directed = false);
    
    // Сохранение в CSV
    static bool saveToCSV(const Graph& g, const std::string& filename);
    
    // Сохранение в JSON
    static bool saveToJSON(const Graph& g, const std::string& filename);
    
private:
    static std::vector<std::string> split(const std::string& s, char delimiter);
};

} // namespace graph

