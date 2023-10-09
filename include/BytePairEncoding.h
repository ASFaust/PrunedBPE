#ifndef BPE_H
#define BPE_H

#include <vector>

#include <string>

using namespace std;

class BPE
{
    public:
        BPE(vector<vector<unsigned char> > production_rules);
        void save(string filename) const;

    private:
        vector<vector<unsigned char> > production_rules;
};

BPE load(string filename);

#endif