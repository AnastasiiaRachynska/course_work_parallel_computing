//
// Created by User on 15.10.2026.
//

#ifndef COURSE_WORK_PARALLEL_COMPUTING_INVERTEDINDEX_H
#define COURSE_WORK_PARALLEL_COMPUTING_INVERTEDINDEX_H
#include <vector>
#include <string>
#include <mutex>
#include <unordered_set>

using namespace std;

string standardize_word(const string& raw_word);

struct Raw{
    string term;
    vector<string> doc_ids;
};

class InvertedIndex {
    public:

    struct stats_data {
        int docs_num = 0;
        int threads_num = 0;
        int build_time_ms = 0;
    };

    vector<vector<Raw>> index;
    vector<mutex> index_mutexes;

    stats_data statics;

    InvertedIndex();
    void add_document(const unordered_set<string>& terms, string doc_id);
    void clear();
    vector<string> find_documents(string term);
    };

#endif