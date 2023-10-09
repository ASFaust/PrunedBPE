#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <list>
#include <stdexcept> // for std::invalid_argument
#include <utility>   // for std::pair
#include <algorithm> // for std::max_element
#include "BytePairEncoding.h"

using namespace std;

BPE learn(std::string filename, int max_read_bytes, int num_tokens);

std::pair<std::pair<int, int>, int> get_max_pair(std::vector<std::vector<int>> token_pair_counts);

void read_file(std::string fname, int max_read_bytes, std::vector<unsigned char>& buffer);

void init_algorithm(
    std::vector<std::vector<int>>& byte_pair_counts,
    std::vector<int>& token_counts,
    std::list<int>& encoded,
    std::vector<unsigned char>& buffer
);

bool get_min_token(
    vector<vector<int>>& token_pair_counts,
    vector<int>& token_counts,
    map<int,vector<int> > &production_rules
);

void update_algorithm_info(
    std::list<int>& encoded,
    std::vector<std::vector<int>>& token_pair_counts,
    std::vector<int>& token_counts,
    int token_counter
);

vector<vector<unsigned char> > unwrap_production_rules(
    map<int, vector<int> > production_rules,
    int num_tokens
);

std::vector<unsigned char> recursive_production(
    int token,
    const std::map<int, std::pair<int, int>>& production_rules
);

void replace_pair(std::list<int>& encoded, std::pair<int, int> max_pair, int c_token);

void inflate(list<int> &encoded, int min_token, vector<int> production_rule);

void replace_token(list<int> &encoded, int to_replace, int replacement);

void alter_production_rules(
    map<int, vector<int> >& production_rules,
    pair<int, int> max_pair,
    int min_token
);