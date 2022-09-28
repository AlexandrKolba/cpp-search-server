#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>
//---------------------------------------------------------------------------------------------
using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
//---------------------------------------------------------------------------------------------
string ReadLine()
{
    string s;
    getline(cin, s);
    return s;
}
//---------------------------------------------------------------------------------------------
int ReadLineWithNumber()
{
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}
//---------------------------------------------------------------------------------------------
vector<string> SplitIntoWords(const string& text)
{
    vector<string> words;
    string word;

    for (const char c : text)
    {
        if (c == ' ')
        {
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
        }
        else word += c;
    }
    if (!word.empty()) words.push_back(word);


    return words;
}
//---------------------------------------------------------------------------------------------
struct Query
{
    set<string> str_plus_word;
    set<string> str_minus_wort;
};
//---------------------------------------------------------------------------------------------
struct Document
{
    int id;
    double relevance;
};
//---------------------------------------------------------------------------------------------
class SearchServer 
{
public:
    void SetStopWords(const string& text)
    {
        for (const string& word : SplitIntoWords(text))
        {
            stop_words_.insert(word);
        }
    }
    //---------------------------------------------------------------------------------------------
    void AddDocument(int document_id, const string& document) 
    {
        const vector<string> words = SplitIntoWordsNoStop(document);
           
        for (const auto& key : words)
        {
            word_to_document_freqs_[key][document_id] += (1.0 / words.size());
            //word_to_document_[key].insert(document_id);
        }
        document_count_++;
    }
    //---------------------------------------------------------------------------------------------
    vector<Document> FindTopDocuments(const string& raw_query) const
    {
        const Query query_words = ParseQuery(raw_query);

        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs)
            {
                return lhs.relevance > rhs.relevance;
            });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
        {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }
    //---------------------------------------------------------------------------------------------
private:
    //map<string, set<int>> word_to_documents_;
    map<string, map<int, double>> word_to_document_freqs_; // document : id : tf
    set<string> stop_words_;
    unsigned int document_count_ = 0;

    //---------------------------------------------------------------------------------------------
    bool IsStopWord(const string& word) const
    {
        return stop_words_.count(word) > 0;
    }
    //---------------------------------------------------------------------------------------------
    vector<string> SplitIntoWordsNoStop(const string& text) const
    {
        vector<string> words;
        for (const string& word : SplitIntoWords(text))
        {
            if (!IsStopWord(word))
            {
                words.push_back(word);
            }
        }
        return words;
    }
    //---------------------------------------------------------------------------------------------
    Query ParseQuery(const string& text) const
    {
        Query query_words;

        for (const string& word : SplitIntoWordsNoStop(text))
        {
            if (word[0] != '-')
                query_words.str_plus_word.insert(word);
            else
                query_words.str_minus_wort.insert(word.substr(1));
        }
        return query_words;
    }
    //---------------------------------------------------------------------------------------------
    vector<Document> FindAllDocuments(const Query& query_words) const
    {
        vector<Document> matched_documents;
        map<int, double> document_to_relevance;

       // double md_IDF = 0.0;
       // double lg = log(1.0 * document_count_ / );

        for (const auto& plus_word : query_words.str_plus_word)
        {
            if (word_to_document_freqs_.count(plus_word) != 0)
            {
                for (auto [index,mn_tf] : word_to_document_freqs_.at(plus_word))
                {
                    double x = log(1.0 * document_count_ / word_to_document_freqs_.at(plus_word).size());
                    document_to_relevance[index] += (mn_tf * x);
                    //++document_to_relevance[p];
                }
            }
        }
        
        //vector<int> del;

        for (const auto& minus_word : query_words.str_minus_wort)
        {
            if (word_to_document_freqs_.count(minus_word) != 0)
            {
                for (auto& p : word_to_document_freqs_.at(minus_word))
                {
                    document_to_relevance.erase(p.first);
                }
            }
        }

        for (const auto& p : document_to_relevance)
        {
            matched_documents.push_back({ p.first,p.second });
        }

        return matched_documents;
    }
    //---------------------------------------------------------------------------------------------
    /*
    static int MatchDocument(const DocumentContent& content, const Query& query_words)
    {
         if (query_words.str_plus_word.empty())
         {
             return 0;
         }
        set<string> matched_words;
        for (const string& word : content.words)
        {
            if (matched_words.count(word) != 0)
            {
                continue;
            }

            if (query_words.str_minus_wort.count(word) != 0)
            {
                return 0;
            }

            if (query_words.str_plus_word.count(word) != 0)
            {
                matched_words.insert(word);
            }
        }
        return static_cast<int>(matched_words.size());

    }*/
};
//---------------------------------------------------------------------------------------------
SearchServer CreateSearchServer()
{
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    

    for (int document_id = 0; document_id < document_count; ++document_id)
    {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}
//---------------------------------------------------------------------------------------------
int main()
{
    setlocale(LC_ALL, "Russian");
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();
    

    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) 
    {
        cout << "{ document_id = "s << document_id << ", "
            << "relevance = "s << relevance << " }"s << endl;
    }

    //for (auto& sd : search_server.FindTopDocuments(query))
    //{
    //    cout << "{ document_id = "s << sd.id << ", "
    //        << "relevance = "s << sd.relevance << " }"s << endl;
    //}
}
//---------------------------------------------------------------------------------------------