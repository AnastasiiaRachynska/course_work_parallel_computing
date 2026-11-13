//
// Created by User on 15.10.2026.
//

#ifndef COURSE_WORK_PARALLEL_COMPUTING_INVERTEDINDEX_H
#define COURSE_WORK_PARALLEL_COMPUTING_INVERTEDINDEX_H
#include <vector>
#include <string>

using namespace std;

struct stats_data {
    int docs_num = 0;
    int threads_num = 0;
    int build_time_ms = 0;
};

class InvertedIndex {
    public:
    //vector<int> stats_data;
    stats_data statics;

    void add_document(vector<string> terms, int doc_id);
    void create_index();

    // vector<int> get_stats();

    };


#endif //COURSE_WORK_PARALLEL_COMPUTING_INVERTEDINDEX_H