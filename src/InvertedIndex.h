//
// Created by User on 15.10.2026.
//

#ifndef COURSE_WORK_PARALLEL_COMPUTING_INVERTEDINDEX_H
#define COURSE_WORK_PARALLEL_COMPUTING_INVERTEDINDEX_H
#include <vector>
#include <string>
#include <mutex>

using namespace std;



struct Raw{
    string term;
    vector<int> doc_ids;
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

    //vector<int> stats_data;
    stats_data statics;

    InvertedIndex();
    void add_document(vector<string> terms, int doc_id);
    void clear();

    // vector<int> get_stats();

    };


#endif //COURSE_WORK_PARALLEL_COMPUTING_INVERTEDINDEX_H