#include "BytePairEncoding.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

BPE::BPE(vector<vector<unsigned char>> production_rules) : production_rules(std::move(production_rules)) {
    for (const auto& rule : this->production_rules) {
        string utf8_string(rule.begin(), rule.end());
        cout << "rule length: " << rule.size() << endl;
        cout << utf8_string << endl;
    }
}

void BPE::save(string filename) const {
    ofstream outFile(filename, ios::binary);
    if (!outFile) {
        // You might want to throw an exception or handle error as appropriate for your use case
        throw runtime_error("Unable to open file for writing: " + filename);
    }

    size_t outerSize = production_rules.size();
    outFile.write(reinterpret_cast<const char*>(&outerSize), sizeof(outerSize));

    for (const auto& innerVector : production_rules) {
        size_t innerSize = innerVector.size();
        outFile.write(reinterpret_cast<const char*>(&innerSize), sizeof(innerSize));
        outFile.write(reinterpret_cast<const char*>(innerVector.data()), innerVector.size());
    }
}

BPE load(const string& filename) {
    ifstream inFile(filename, ios::binary);
    if (!inFile) {
        // Handle error as appropriate for your use case
        throw runtime_error("Unable to open file for reading: " + filename);
    }

    size_t outerSize = 0;
    inFile.read(reinterpret_cast<char*>(&outerSize), sizeof(outerSize));

    vector<vector<unsigned char>> production_rules(outerSize);
    for (auto& innerVector : production_rules) {
        size_t innerSize = 0;
        inFile.read(reinterpret_cast<char*>(&innerSize), sizeof(innerSize));

        innerVector.resize(innerSize);
        inFile.read(reinterpret_cast<char*>(innerVector.data()), innerSize);
    }

    return BPE(production_rules);
}