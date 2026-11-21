#ifndef COURSE_WORK_PARALLEL_COMPUTING_INVERTEDINDEX_H
#define COURSE_WORK_PARALLEL_COMPUTING_INVERTEDINDEX_H
#include <vector>
#include <string>
#include <unordered_set>
#include "HashTable.h"

using namespace std;

string standardize_word(string& raw_word);


class InvertedIndex {
    public:

    struct stats_data {
        int docs_num = 0;
        int threads_num = 0;
        int build_time_ms = 0;
        bool success = true;
    };

    HashTable index;

    stats_data statics;

    void add_document(const unordered_set<string>& terms, string doc_id);
    void clear();
    vector<string> find_documents(string term);
    };

#endif