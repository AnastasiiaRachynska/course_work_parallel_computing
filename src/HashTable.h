#ifndef COURSE_WORK_PARALLEL_COMPUTING_HASHTABLE_H
#define COURSE_WORK_PARALLEL_COMPUTING_HASHTABLE_H

#include <string>
#include <vector>
#include <mutex>
#include <ostream>
using namespace std;

struct Row{
    string term;
    vector<string> doc_ids;
};

class HashTable {
private:
    vector<vector<Row>> table;
    vector<mutex> mutexes;
public:
    HashTable();
    void insert(const string& term, const string& doc_id);
    vector<string> find(const string& term);
    void clear();
    void print(ostream& output);
};

#endif