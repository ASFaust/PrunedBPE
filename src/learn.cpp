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

        //TODO: create estimates for the length of the encoding after the replacement
        //according to my understanding of the formula
        bool create_new_token = get_min_token(token_pair_counts, token_counts, production_rules);

        replace_pair(encoded, max_pair, token_counter);

        if(create_new_token){
            cout << "->creating new token " << token_counter << endl;
            production_rules[token_counter] = {max_pair.first, max_pair.second};
            token_counter += 1;
        }
        else{
            cout << "->replacing old token " << min_token << endl;
            cout << " old token rule was ";
            for(auto i : production_rules[min_token]){
                cout << i << " ";
            }
            cout << endl;
            //cout << " new token rule is " << max_pair.first << " " << max_pair.second << endl;

            vector<int> production_rule = production_rules[min_token];

            inflate(encoded, min_token, production_rules[min_token]); //replace min_token with production_rules[min_token]

            replace_token(encoded, token_counter, min_token); //token_counter was just a temporary token, now we replace it with the new token

            //now we need to alter the production rules[min_token] to reflect the new rule
            //we need to insert elements into the production rule if the new production rule would be self-referential
            alter_production_rules(production_rules, max_pair, min_token);
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
        if(debug_counter >= 100){
            break;
        }
    }

    return BPE(
        unwrap_production_rules(production_rules, num_tokens)
    );
}

void alter_production_rules(
    map<int, vector<int> >& production_rules,
    pair<int, int> max_pair,
    int min_token
){
    vector<int> production_rule = production_rules[min_token];
    vector<int> left_production;
    vector<int> right_production;
    if(max_pair.first == min_token){
        left_production = production_rule;
    }else{
        left_production = {max_pair.first};
    }
    if(max_pair.second == min_token){
        right_production = production_rule;
    }else{
        right_production = {max_pair.second};
    }
    left_production.insert(left_production.end(), right_production.begin(), right_production.end());
    production_rules[min_token] = left_production;
    cout << " new token rule is ";
    for(auto i : production_rules[min_token]){
        cout << i << " ";
    }
    cout << endl;
}


pair<pair<int, int>,int> get_max_pair(vector<vector<int> > token_pair_counts){
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
    return make_pair(max_pair, max_count);
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

int get_min_token(
    vector<vector<int>>& token_pair_counts,
    vector<int>& token_counts,
    map<int, vector<int>> &production_rules
){
    int encoding_length = accumulate(token_counts.begin(), token_counts.end(), 0);
    cout << "Initial encoding length is " << encoding_length << endl;

    int min_token = -1;
    int min_encoding_length = encoding_length;

    for(int i = 0; i < token_pair_counts.size(); i++){
        for(int j = 0; j < token_pair_counts[i].size(); j++){
            for(int k = 256; k < token_counts.size(); k++){
                int new_encoding_length = encoding_length;

                int adjusted_token_counts = token_counts[k];
                if((k != i) && (k != j)){
                    //nothing happens. all k are unraveled.
                }else{
                    if((k==i) && (k != j)){
                        //we dont unravel the ks that are followed by j,
                        //which are exactly token_pair_counts many
                        adjusted_token_counts -= token_pair_counts[k][j];
                    }
                    if((k!=i) && (k == j)){
                        adjusted_token_counts -= token_pair_counts[i][k];
                    }
                }
                // we do not unravel k if
                new_encoding_length += (production_rules[k].size() - 1) * adjusted_token_counts;

                // Decrease encoding length for each [i, j] replaced by k.
                new_encoding_length -= token_pair_counts[i][j];

                // Update min_encoding_length and min_token if a shorter encoding is found.
                if(new_encoding_length < min_encoding_length){
                    min_encoding_length = new_encoding_length;
                    min_token = k;
                }
            }
        }
    }

    // Return min_token to indicate which token (if any) should be replaced.
    // Return -1 to indicate a new token should be created.
    return min_token;
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

void inflate(list<int> &encoded, int min_token, vector<int> production_rule){
    //we now need to replace the most common pair with a new token.
    auto it = encoded.begin();
    while(it != std::prev(encoded.end())) { // stop loop one before the last element
        // Use std::next(it) to access the next element without incrementing it
        if(*it == min_token) {
            it = encoded.erase(it); // erase returns iterator to following element
            for(auto i : production_rule){
                it = encoded.insert(it, i);
                it++;
            }
        } else {
            ++it; // increment iterator only if no erasure has been done because of erase return value
        }
    }
}

void replace_token(list<int> &encoded, int to_replace, int replacement){
    //we now need to replace the most common pair with a new token.
    auto it = encoded.begin();
    while(it != std::prev(encoded.end())) { // stop loop one before the last element
        // Use std::next(it) to access the next element without incrementing it
        if(*it == to_replace) {
            *it = replacement;
        }
        ++it;
    }
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
