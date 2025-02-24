#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <cerrno>
#include <cstring>


// Structure to hold aggregated trade stats for a symbol.
struct TradeStats {
    int maxGap = 0;
    int volume = 0;
    long long weightedSum = 0;
    int maxPrice = 0;
    int lastTimestamp = 0;
    bool firstTrade = true;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " input.csv output.csv\n";
        return 1;
    }

    std::ifstream infile(argv[1]);
    if (!infile.is_open()) {
        std::cout << "Attempting to open: " << argv[1] << "...\n";
        std::cerr << "Error opening input file: " << argv[1] << "\n";
        std::cerr << "System error: " << ::strerror(errno) << "\n";
        return 1;
    }

    // Use a std::map so that output is automatically sorted by symbol (ascending).
    std::map<std::string, TradeStats> symbolStats;
    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string token;

        // Parse the CSV line: <TimeStamp>,<Symbol>,<Quantity>,<Price>
        std::getline(iss, token, ',');
        int timestamp = std::stoi(token);

        std::string symbol;
        std::getline(iss, symbol, ',');

        std::getline(iss, token, ',');
        int quantity = std::stoi(token);

        std::getline(iss, token, ',');
        int price = std::stoi(token);

        // Update the stats for this symbol.
        auto& stats = symbolStats[symbol];
        if (stats.firstTrade) {
            stats.firstTrade = false;
            stats.lastTimestamp = timestamp;
        }
        else {
            int gap = timestamp - stats.lastTimestamp;
            if (gap > stats.maxGap)
                stats.maxGap = gap;
            stats.lastTimestamp = timestamp;
        }
        stats.volume += quantity;
        stats.weightedSum += static_cast<long long>(quantity) * price;
        if (price > stats.maxPrice)
            stats.maxPrice = price;
    }
    infile.close();

    // Write the output file.
    std::ofstream outfile(argv[2]);
    if (!outfile.is_open()) {
        std::cerr << "Error opening output file: " << argv[2] << "\n";
        return 1;
    }

    // Output format: <symbol>,<MaxTimeGap>,<Volume>,<WeightedAveragePrice>,<MaxPrice>
    // WeightedAveragePrice is calculated per unit traded and then truncated.
    for (const auto& [symbol, stats] : symbolStats) {
        int weightedAvgPrice = (stats.volume != 0) ? static_cast<int>(stats.weightedSum / stats.volume) : 0;
        outfile << symbol << ","
                << stats.maxGap << ","
                << stats.volume << ","
                << weightedAvgPrice << ","
                << stats.maxPrice << "\n";
    }
    outfile.close();

    return 0;
}
