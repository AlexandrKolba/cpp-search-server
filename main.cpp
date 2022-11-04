#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <numeric>
#include <iostream>
#include <cassert>
#include <sstream>
#include <cstdlib>
#include <optional>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    Document() = default;

    Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating) {
    }

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

template <typename T>
set<string> NoEmpty(const T& text)
{
    set<string> local_set;

    for (const string& word : text)
    {
        if (!word.empty())
        {
            local_set.insert(word);
        }
    }

    return local_set;
}

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    template <typename T>
    explicit SearchServer(const T& stop_words) : stop_words_(NoEmpty(stop_words))
    {
        for (auto &word : stop_words_)
        {
            if (!IsValidWord(word))
            {
                throw invalid_argument("Error to constructor");
            }
        }
    }

    explicit SearchServer(const string& stop_words_text) : SearchServer(SplitIntoWords(stop_words_text))
    {
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings)
    {
        if (document_id < 0 || documents_.count(document_id))
        {
            throw invalid_argument("Error add document to ID");
        }

        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();

        for (const string& word : words)
        {
            if (!IsValidWord(word))
            {
                throw invalid_argument("Error add document to word");
            }
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        num_id.push_back(document_id);
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    }

    auto GetDocumentCount() const
    {
        return documents_.size();
    }

    template <typename T>
    vector<Document> FindTopDocuments(const string& raw_query, T predicate) const
    {
        Query query;
        if (!ParseQuery(raw_query, query))
        {
            throw invalid_argument("Error to FindTopDocuments oops...");
        }

        auto matched_documents = FindAllDocuments(query, predicate);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs)
            {
                if (abs(lhs.relevance - rhs.relevance) < EPSILON)
                {
                    return lhs.rating > rhs.rating;
                }
                else
                {
                    return lhs.relevance > rhs.relevance;
                }
            });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
        {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const
    {
        return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus _status, int /*rating*/)
            {
                return  status == _status;
            });
    }

    vector<Document> FindTopDocuments(const string& raw_query) const
    {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }


    tuple<vector<string>, const DocumentStatus> MatchDocument(const string& raw_query, int document_id) const
    {
        Query query;
        if (!ParseQuery(raw_query, query))
        {
            throw invalid_argument("Error to Match Document oops...");
        }

        vector<string> matched_words;

        for (const string& word : query.plus_words)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            if (IsValidWord(word) && word_to_document_freqs_.at(word).count(document_id))
            {
                matched_words.push_back(word);
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id) || !IsMinusWord(query.minus_words)) {
                matched_words.clear();
                break;
            }
        }

        return make_tuple(matched_words, documents_.at(document_id).status);
    }


    int GetDocumentId(int i) const
    {
        if (i < 0 || i > GetDocumentCount())
        {
            throw out_of_range("Error: ID < 0 or ID > count documents");
        }
        return num_id[i];
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> num_id;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
        return rating_sum / static_cast<int>(ratings.size());
    }

    static bool IsValidWord(const string& word) {
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
            });
    }

    static bool IsMinusWord(const set<string>& query_minus_word)
    {
        for (auto& word : query_minus_word)
        {
            if (word.empty())
            {
                return false;
            }

            if (word[0] == '-')
            {
                return false;
            }

        }
        return true;
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {

        bool is_minus = false;
        string s = text;
        // Word shouldn't be empty
        if (text[0] == '-')
        {
            is_minus = true;
            s = text.substr(1);
        }
        QueryWord return_value = { s, is_minus, IsStopWord(s) };
        return return_value;
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    bool ParseQuery(const string& text, Query &return_query) const 
    {
        for (const string& word : SplitIntoWords(text)) 
        {
            if (word == "-")
            {
                return false;
            }

            if (word[0] == '-' && word[1] == '-')
            {
                return false;
            }

            if (!IsValidWord(word)) 
            {
                return false;
            }

            const QueryWord query_word = ParseQueryWord(word);

            if (!query_word.is_stop) 
            {
                if (query_word.is_minus) 
                {
                    return_query.minus_words.insert(query_word.data);
                }
                else 
                {
                    return_query.plus_words.insert(query_word.data);
                }
            }
        }

        return true;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const
    {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const
    {
        map<int, double> document_to_relevance;

        for (const string& word : query.plus_words)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }

            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);

            for (const auto& [document_id, _freq] : word_to_document_freqs_.at(word))
            {
                const auto& document_data = documents_.at(document_id);

                if (document_predicate(document_id, document_data.status, document_data.rating))
                {
                    document_to_relevance[document_id] += _freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            for (const auto& [document_id, _] : word_to_document_freqs_.at(word))
            {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto& [document_id, relevance] : document_to_relevance)
        {
            matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
        }

        return matched_documents;
    }
};
