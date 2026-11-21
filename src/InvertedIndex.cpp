#include "InvertedIndex.h"
#include <string>
#include <unordered_set>
using namespace std;



string standardize_word(string& raw_word) {
    string result = "";
    for (unsigned char c : raw_word)
        if (isalnum(c)) result += static_cast<char>(tolower(c));
    return result;
}

void InvertedIndex::clear() {
    index.clear();
    statics.docs_num = 0;
    statics.build_time_ms = 0;
    statics.threads_num = 0;
}

void InvertedIndex::add_document(const unordered_set<string>& terms, string doc_id) {
    for (const string& term : terms)
        index.insert(term, doc_id);
}

vector<string> InvertedIndex::find_documents(string term) {
    term = standardize_word(term);

    if (term.empty()) return {};

    return index.find(term);
}