#include "BytePairEncoding.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

BPE::BPE(vector<vector<unsigned char>> production_rules) : production_rules(std::move(production_rules)) {
    /*for (const auto& rule : this->production_rules) {
        string utf8_string(rule.begin(), rule.end());
        cout << "rule length: " << rule.size() << endl;
        cout << utf8_string << endl;
    }*/
    //production_rules now hold the production rules.
    //sort the production rules by length, production_rules[0] being the longest
    sort(this->production_rules.begin(), this->production_rules.end(),
        [](const vector<unsigned char>& a, const vector<unsigned char>& b) {
            return a.size() > b.size();
        });
    //that we just rearranged the tokens doesnt matter.
}

vector<int> BPE::encode(string inp){
    vector<int> ret;
    //now we need to use the production rules to encode the input
    //we will use a greedy algorithm to do this
    //we will start with the longest production rule and work our way down
    vector<int> active_candidates; //indices into production_rules that are currently being considered
    for (int i = 0; i < inp.size(); i++){
        //production rules start at 0, but if they are emitted, they are added + 256
        int current_token = inp[i];

    }
    return ret;
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