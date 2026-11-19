#include "../src/Ollama.h"
#include <iostream>
#include <string>
using namespace std;

int main() {
    Ollama ollama;
    ollama.api_url = "localhost";
    string query = "what is the computer";
    if (ollama.is_working())
        cout << "Ollama API is working." << endl;
    vector<string> results = ollama.expand_query(query);
    if (results.empty()) cout << "No results found." << endl;
    else {
        for (const string& result : results)
            cout << result << endl;
    }

    return 0;
}