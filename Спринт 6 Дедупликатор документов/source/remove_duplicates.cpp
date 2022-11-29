#include <set>
#include <vector>

#include "remove_duplicates.h"

using namespace std;

void RemoveDuplicates(SearchServer& search_server) 
{
    vector<int> duplicates_id;

    map<string, int> words_to_id_;
    for (const int document_id : search_server) 
    {
        string document_words;
        map<string, double> words_freq = search_server.GetWordFrequencies(document_id);

        for (const auto uniq_words : words_freq) 
        {
            document_words += uniq_words.first;
        }
        if (words_to_id_.count(document_words) > 0) 
        {
            duplicates_id.push_back(document_id);
        }
        else 
        {
            words_to_id_[document_words] = document_id;
        }
    }
    for (auto doc_id_ : duplicates_id) 
    {
        cout << "Found duplicates document id " << doc_id_ << endl;
        search_server.RemoveDocument(doc_id_);
    }
}
