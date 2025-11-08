#include "io/loader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace graph {

std::vector<std::string> GraphLoader::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::unique_ptr<Graph> GraphLoader::loadFromCSV(const std::string& filename, bool directed) {
    auto graph = std::make_unique<Graph>(directed);
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
        return nullptr;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        auto tokens = split(line, ',');
        if (tokens.size() >= 2) {
            try {
                int from = std::stoi(tokens[0]);
                int to = std::stoi(tokens[1]);
                double weight = tokens.size() >= 3 ? std::stod(tokens[2]) : 1.0;
                graph->addEdge(from, to, weight);
            } catch (const std::exception& e) {
                std::cerr << "Ошибка при парсинге строки: " << line << std::endl;
            }
        }
    }
    
    file.close();
    return graph;
}

std::unique_ptr<Graph> GraphLoader::loadFromJSON(const std::string& filename, bool directed) {
    // Упрощенная реализация JSON загрузки
    auto graph = std::make_unique<Graph>(directed);
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
        return nullptr;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    // Простой парсер JSON (для полной реализации нужна библиотека)
    // Ищем массив edges
    size_t edgesPos = content.find("\"edges\"");
    if (edgesPos == std::string::npos) {
        std::cerr << "Ошибка: не найден массив edges в JSON" << std::endl;
        return nullptr;
    }
    
    size_t arrayStart = content.find('[', edgesPos);
    if (arrayStart == std::string::npos) return nullptr;
    
    size_t pos = arrayStart + 1;
    while (pos < content.length()) {
        size_t objStart = content.find('{', pos);
        if (objStart == std::string::npos) break;
        
        size_t objEnd = content.find('}', objStart);
        if (objEnd == std::string::npos) break;
        
        std::string obj = content.substr(objStart, objEnd - objStart + 1);
        
        // Извлечь from, to, weight
        size_t fromPos = obj.find("\"from\"");
        size_t toPos = obj.find("\"to\"");
        size_t weightPos = obj.find("\"weight\"");
        
        if (fromPos != std::string::npos && toPos != std::string::npos) {
            try {
                size_t fromValStart = obj.find(':', fromPos) + 1;
                size_t fromValEnd = obj.find_first_of(",}", fromValStart);
                int from = std::stoi(obj.substr(fromValStart, fromValEnd - fromValStart));
                
                size_t toValStart = obj.find(':', toPos) + 1;
                size_t toValEnd = obj.find_first_of(",}", toValStart);
                int to = std::stoi(obj.substr(toValStart, toValEnd - toValStart));
                
                double weight = 1.0;
                if (weightPos != std::string::npos) {
                    size_t weightValStart = obj.find(':', weightPos) + 1;
                    size_t weightValEnd = obj.find_first_of(",}", weightValStart);
                    weight = std::stod(obj.substr(weightValStart, weightValEnd - weightValStart));
                }
                
                graph->addEdge(from, to, weight);
            } catch (const std::exception& e) {
                std::cerr << "Ошибка при парсинге JSON объекта: " << obj << std::endl;
            }
        }
        
        pos = objEnd + 1;
    }
    
    return graph;
}

bool GraphLoader::saveToCSV(const Graph& g, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось создать файл " << filename << std::endl;
        return false;
    }
    
    auto edges = g.getEdges();
    for (const auto& edge : edges) {
        file << edge.from << "," << edge.to << "," << edge.weight << "\n";
    }
    
    file.close();
    return true;
}

bool GraphLoader::saveToJSON(const Graph& g, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось создать файл " << filename << std::endl;
        return false;
    }
    
    file << "{\n";
    file << "  \"directed\": " << (g.isDirected() ? "true" : "false") << ",\n";
    file << "  \"edges\": [\n";
    
    auto edges = g.getEdges();
    for (size_t i = 0; i < edges.size(); ++i) {
        const auto& edge = edges[i];
        file << "    { \"from\": " << edge.from 
             << ", \"to\": " << edge.to 
             << ", \"weight\": " << edge.weight << " }";
        if (i < edges.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    return true;
}

} // namespace graph

