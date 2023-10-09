#include "learn.h"
#include "BytePairEncoding.h"

BPE learn(string filename, int max_read_bytes, int num_tokens) {

    // function implementation
    // first read the file directly to a byte array

    if(num_tokens < 257){
        throw std::invalid_argument("num_tokens must be at least 257");
    }

    vector<unsigned char> buffer;

    read_file(filename, max_read_bytes, buffer);

    map<int, vector<int> > production_rules;

    // now we have the file in a byte array. we dont need to convert it to a string, as we directly operate
    // on the bytes

    vector<vector<int> > token_pair_counts;
    vector<int> token_counts;
    list<int> encoded;

    init_algorithm(token_pair_counts, token_counts, encoded, buffer);

    int token_counter = 256;
    int debug_counter = 0; //temporary. only do n iterations of the algorithm.
    while(token_counter < num_tokens){

        auto [min_token,replacing_with,predicted_encoding_length] = try_replacing(token_pair_counts, token_counts, production_rules);

        if(min_token == -1){
            cout << "->creating new token " << token_counter << endl;
            pair<int, int> most_common_pair = get_max_pair(token_pair_counts);
            cout << " most common pair is " << most_common_pair.first << " , " << most_common_pair.second << endl;
            replace_pair(encoded, most_common_pair, token_counter);

            production_rules[token_counter] = {most_common_pair.first, most_common_pair.second};
            token_counter += 1;
        }
        else{
            cout << "->replacing old token " << min_token << endl;
            cout << " old token rule was ";
            for(auto i : production_rules[min_token]){
                cout << i << " ";
            }
            cout << endl;

            vector<int> old_production_rule = production_rules[min_token];
            reencode(encoded, min_token, replacing_with, production_rules[min_token]);

            alter_production_rules(production_rules, min_token, replacing_with);

            cout << " new token rule is ";
            for(auto i : production_rules[min_token]){
                cout << i << " ";
            }
            cout << endl;

            //has signature void(vector<int>&, int, pair<int,int>, map<int, vector<int> >&)

            cout << " new encoding length is " << encoded.size() << endl;

            if(encoded.size() != predicted_encoding_length){
                cout << "encoding length mismatch! you little fool, adjust your algorithm ;) " << endl;
                cout << "encoded size is " << encoded.size() << endl;
                cout << "predicted encoding length is " << predicted_encoding_length << endl;
                cout << "difference is " << encoded.size() - predicted_encoding_length << endl;
                //throw a runtime error
                throw std::runtime_error("encoding length mismatch");
            }
        }

        cout << "at the end of the iteration, the new encoding length is " << encoded.size() << endl;
        /*
        cout << "first few tokens are \n";
        auto it = encoded.begin();
        for(int i = 0; i < 2000; i++){
            cout << *it << " ";
            it++;
        }
        cout << endl;
        */
        update_algorithm_info(encoded, token_pair_counts, token_counts, token_counter);
        debug_counter++;
        if(debug_counter >= 1000){
            break;
        }
    }

    return BPE(
        unwrap_production_rules(production_rules, num_tokens)
    );
}

void alter_production_rules(
    map<int, vector<int>>& production_rules,
    int min_token,
    pair<int, int> replacing_with
){
    // Step 1: Direct replacement of `min_token` production rule
    vector<int> old_production_rule = production_rules[min_token];
    cout << "replacing " << min_token << " with " << replacing_with.first << " , " << replacing_with.second << endl;
    cout << "old rule was ";
    for(auto i : old_production_rule){
        cout << i << " ";
    }
    cout << endl;
    vector<int> new_production_rule;
    if(replacing_with.first == min_token){
        cout << "first token is min token: " << replacing_with.first << endl;
        new_production_rule.insert(new_production_rule.end(), old_production_rule.begin(), old_production_rule.end());
    }else{
        new_production_rule.push_back(replacing_with.first);
    }

    if(replacing_with.second == min_token){
        cout << "second token is min token: " << replacing_with.second << endl;
        new_production_rule.insert(new_production_rule.end(), old_production_rule.begin(), old_production_rule.end());
    }else{
        new_production_rule.push_back(replacing_with.second);
    }

    production_rules[min_token] = new_production_rule;

    // Step 2: Update all existing production rules
    for(auto& rule : production_rules){
        // Check and update only if `min_token` is not the key being iterated (already updated in Step 1)
        if(rule.first != min_token){
            vector<int> updated_rule;
            for(int token : rule.second){
                if(token == min_token){
                    updated_rule.insert(updated_rule.end(), old_production_rule.begin(), old_production_rule.end());
                }else{
                    updated_rule.push_back(token);
                }
            }
            rule.second = updated_rule;
        }
    }
}

pair<int, int> get_max_pair(vector<vector<int> > token_pair_counts){
    int max_count = 0;
    pair<int, int> max_pair;
    for(int i = 0; i < token_pair_counts.size(); i++){
        for(int j = 0; j < token_pair_counts[i].size(); j++){
            if(token_pair_counts[i][j] > max_count){
                max_count = token_pair_counts[i][j];
                max_pair = make_pair(i, j);
            }
        }
    }
    return max_pair;
}

void read_file(string fname, int max_read_bytes, vector<unsigned char> &buffer){
    ifstream file(fname, ios::binary | ios::ate);
    streamsize size = file.tellg();
    if (size > max_read_bytes && max_read_bytes > 0) {
        size = max_read_bytes;
    }
    if(size < 2){
        throw std::invalid_argument("file must be at least 2 bytes long");
    }
    file.seekg(0, ios::beg);
    buffer.resize(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    file.close();
}

void init_algorithm(
        vector<vector<int> > &token_pair_counts,
        vector<int> &token_counts,
        list<int> &encoded,
        vector<unsigned char> &buffer
     ){
    token_counts.resize(256, 0);
    token_pair_counts.resize(256);
    for(int i = 0; i < 256; i++){
        token_pair_counts[i].resize(256, 0);
    }
    streamsize size = buffer.size();

    pair<int, int> current_pair;
    pair<int, int> last_pair = make_pair(-1, -1);
    for (int i = 0; i < size - 1; i++){
        if (i % 1000 == 0) {  // Print progress every 1000 iterations instead of every iteration
            cout << "\r" << i << "/" << size << flush;
        }
        auto first_byte = static_cast<unsigned char>(buffer[i]);
        auto second_byte = static_cast<unsigned char>(buffer[i + 1]);

        encoded.push_back(first_byte);
        token_counts[first_byte]++;
        current_pair = make_pair(first_byte, second_byte);
        if((last_pair.first == last_pair.second) && (current_pair == last_pair)){
            last_pair = make_pair(-1, -1);
            continue;
        }
        token_pair_counts[first_byte][second_byte]++;
    }
    auto last_byte = static_cast<unsigned char>(buffer[size - 1]);
    encoded.push_back(last_byte);
    token_counts[last_byte]++;
}

tuple<int,pair<int,int>,int>  try_replacing(
    vector<vector<int>>& token_pair_counts,
    vector<int>& token_counts,
    map<int, vector<int>> &production_rules
){
    //formula for encoding length change:
    // -1 for each pair replaced -> we can use token_pair_counts[replacing_with.first][replacing_with.second]
    // + (production_rule.size() - 1) for each min_token replaced
    //we cant use token_count[min_token] if min_token == replacing_with.first or replacing_with.second
    //we need to subtract token_pair_counts[min_token] if min_token == replacing_with.first or replacing_with.second
    //we need to subtract it twice if min_token == replacing_with.first and replacing_with.second, i just realized
    int encoding_length = 0;
    for(int i = 0; i < token_counts.size(); i++){
        encoding_length += token_counts[i];
    }
    cout << "encoding length: " << encoding_length << endl;
    int min_token = -1;
    int min_encoding_length = encoding_length;
    int ri = 0;
    int rj = 0;
    int min_adjusted_token_counts = 0;
    for(int k = 256; k < token_counts.size(); k++){
        for(int i = 0; i < token_pair_counts.size(); i++){
            for(int j = 0; j < token_pair_counts[i].size(); j++){
                int adjusted_token_counts = token_counts[k];
                int new_encoding_length = encoding_length;

                if(k == i){
                    adjusted_token_counts -= token_pair_counts[k][j];
                }
                if(k == j){
                    adjusted_token_counts -= token_pair_counts[i][k];
                }
                //adjusted_token_counts -= token_pair_counts[i][j];
                //used to be production_rules[k].size() - 1!!
                new_encoding_length += (production_rules[k].size() - 1) * adjusted_token_counts;

                // Decrease encoding length for each [i, j] replaced by k.
                new_encoding_length -= token_pair_counts[i][j];

                // Update min_encoding_length and min_token if a shorter encoding is found.
                if(new_encoding_length < min_encoding_length){
                    min_encoding_length = new_encoding_length;
                    min_token = k;
                    ri = i;
                    rj = j;
                    min_adjusted_token_counts = adjusted_token_counts;
                }
            }
        }
    }
    if(min_token != -1){
        cout << "min token: " << min_token << endl;
        cout << "min encoding length: " << min_encoding_length << endl;
        cout << "new token pair: " << ri << " " << rj << endl;
        cout << "tokens that will be expanded: " << min_adjusted_token_counts << endl;
        cout << "length of the production that is going to be unrolled: " << production_rules[min_token].size() << endl;
        cout << "token pair counts of the new token pair: " << token_pair_counts[ri][rj] << endl;
    }
    //auto [min_token,replacing_with,predicted_encoding_length] = try_replacin
    return make_tuple(min_token, make_pair(ri, rj), min_encoding_length);
}

void replace_pair(list<int> &encoded, pair<int, int> max_pair, int c_token){
    //we now need to replace the most common pair with a new token.
    auto it = encoded.begin();
    while(it != std::prev(encoded.end())) { // stop loop one before the last element
        // Use std::next(it) to access the next element without incrementing it
        if(*it == max_pair.first && *std::next(it) == max_pair.second) {
            *it = c_token;
            it = encoded.erase(std::next(it)); // erase returns iterator to following element
        } else {
            ++it; // increment iterator only if no erasure has been done because of erase return value
        }
    }
}

void reencode(
    list<int>& encoded,
    int min_token,
    pair<int,int> replacing_with,
    vector<int>& old_production_rule
    ){
    //we now need to replace the most common pair with a new token.
    auto it = encoded.begin();
    while(next(it) != encoded.end()){
        if(((*it) == replacing_with.first) && ((*next(it) == replacing_with.second))){
            *it = min_token;
            it = encoded.erase(next(it));
        }else{
            if((*it) == min_token){
                it = encoded.erase(it);
                //production rules needs to already be expanded, any self-references need already be expanded
                it = encoded.insert(it, old_production_rule.begin(), old_production_rule.end());
                advance(it, old_production_rule.size());
            }else{
                it++; //oops forgot this
            }
        }
    }
    //formula for encoding length change:
    // -1 for each pair replaced -> we can use token_pair_counts[replacing_with.first][replacing_with.second]
    // + (production_rule.size() - 1) for each min_token replaced
    //we cant use token_count[min_token] if min_token == replacing_with.first or replacing_with.second
    //we need to subtract token_pair_counts[min_token] if min_token == replacing_with.first or replacing_with.second
    //we need to subtract it twice if min_token == replacing_with.first and replacing_with.second, i just realized
}

void update_algorithm_info(
        std::list<int> &encoded,
        std::vector<std::vector<int> > &token_pair_counts,
        std::vector<int> &token_counts,
        int token_counter) {
    token_counts.resize(token_counter, 0);

    // Initialize the token pair counts to a token_counter x (c_token + 1) matrix of 0s
    token_pair_counts.resize(token_counter);
    for(int i = 0; i < token_counter; i++) {
        token_counts[i] = 0;
        token_pair_counts[i].resize(token_counter, 0);
        for(int j = 0; j < token_counter; j++) {
            token_pair_counts[i][j] = 0;
        }
    }

    // Using iterators to traverse 'encoded' instead of indices.
    auto it = encoded.begin();
    pair<int, int> current_pair;
    pair<int, int> last_pair = make_pair(-1, -1);
    while (it != std::prev(encoded.end())) {
        current_pair = make_pair(*it, *std::next(it));
        //it's not that easy.
        token_counts[*it]++;
        ++it;
        if ((current_pair == last_pair) && (last_pair.first == last_pair.second)) {
            last_pair = make_pair(-1, -1);
            continue;
        }
        token_pair_counts[current_pair.first][current_pair.second]++;
        last_pair = current_pair;
    }

    // Here 'it' points to the last element of 'encoded', if it is not empty.
    if (it != encoded.end()) {
        token_counts[*it]++;
    }

}

vector<vector<unsigned char> > unwrap_production_rules(
    map<int, vector<int> > production_rules,
    int num_tokens
){
    vector<vector<unsigned char> > unwrapped_production_rules;

    /*unwrapped_production_rules.resize(num_tokens);
    for(int i = 0; i < num_tokens; i++){
        unwrapped_production_rules[i] = recursive_production(i, production_rules);
    }*/
    return unwrapped_production_rules;
}

std::vector<unsigned char> recursive_production(
    int token,
    const std::map<int, std::pair<int, int>>& production_rules
){
    // Retrieve left and right using the provided token.
    auto [left, right] = production_rules.at(token);

    // Define two vectors to hold the recursively obtained results.
    std::vector<unsigned char> left_ret, right_ret;

    // Get left results.
    if(left < 256){
        left_ret.push_back(static_cast<unsigned char>(left));
    }else{
        left_ret = recursive_production(left, production_rules);
    }

    // Get right results.
    if(right < 256){
        right_ret.push_back(static_cast<unsigned char>(right));
    }else{
        right_ret = recursive_production(right, production_rules);
    }

    // Concatenate left and right results and return.
    left_ret.insert(left_ret.end(), right_ret.begin(), right_ret.end());
    return left_ret;
}
