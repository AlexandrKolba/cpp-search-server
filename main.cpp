//file main.cpp
//---------------------------------------------------------------------------------------
#include "read_input_functions.h"
#include "string_processing.h"
#include "document.h"
#include "paginator.h"
#include "request_queue.h"

void PrintDocument(const Document& document) 
{
    std::cout << "{ "
        << "document_id = " << document.id << ", "
        << "relevance = " << document.relevance << ", "
        << "rating = " << document.rating << " }" << std::endl;
}

void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status) 
{
    std::cout << "{ "
        << "document_id = " << document_id << ", "
        << "status = " << static_cast<int>(status) << ", "
        << "words =";

    for (const std::string& word : words) 
    {
        std::cout << ' ' << word;
    }
    std::cout << "}" << std::endl;
}

void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
                 const std::vector<int>& ratings) 
{
    try 
    {
        search_server.AddDocument(document_id, document, status, ratings);
    }
    catch (const std::invalid_argument& e) 
    {
        std::cout << "Ошибка добавления документа " << document_id << ": " << e.what() << std::endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) 
{
    std::cout << "Результаты поиска по запросу: " << raw_query << std::endl;

    try 
    {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) 
        {
            PrintDocument(document);
        }
    }
    catch (const std::invalid_argument& e) 
    {
        std::cout << "Ошибка поиска: " << e.what() << std::endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const std::string& query) 
{
    try 
    {
        std::cout << "Матчинг документов по запросу: " << query << std::endl;
        const int document_count = search_server.GetDocumentCount();

        for (int index = 0; index < document_count; ++index) 
        {
            const int document_id = search_server.GetDocumentId(index);
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    }
    catch (const std::invalid_argument& e) 
    {
        std::cout << "Ошибка матчинга документов на запрос " << query << ": " << e.what() << std::endl;
    }
}

using namespace std;

int main() 
{
    SearchServer search_server("and in at"s);
    RequestQueue request_queue(search_server); 

    search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
    search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

    for (int i = 0; i < 1439; ++i) 
    {
        request_queue.AddFindRequest("empty request"s);
    }
 
    request_queue.AddFindRequest("curly dog"s);
    request_queue.AddFindRequest("big collar"s);
    request_queue.AddFindRequest("sparrow"s);
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
    return 0;
}
// end file main.cpp
//---------------------------------------------------------------------------------------
// file document.cpp

#include "document.h"

using namespace std;

ostream& operator<<(ostream& out, const Document& document) 
{
    out << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s;
    return out;
}
// end file document.cpp
//---------------------------------------------------------------------------------------
// file document.h
#pragma once

#include <iostream>

const int MAX_RESULT_DOCUMENT_COUNT = 5;

enum class DocumentStatus 
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document 
{
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

std::ostream& operator<<(std::ostream& out, const Document& document);
//end file document.h
//---------------------------------------------------------------------------------------
// file paginator.h
#pragma once

template <typename Iterator>
class IteratorRange 
{
    public:
        IteratorRange(Iterator begin, Iterator end)
            : first_(begin)
            , last_(end)
            , size_(distance(first_, last_)) {
        }

        Iterator begin() const {
            return first_;
        }

        Iterator end() const {
            return last_;
        }

        size_t size() const {
            return size_;
        }

    private:
        Iterator first_, last_;
        size_t size_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& range) 
{
    for (Iterator it = range.begin(); it != range.end(); ++it) 
    {
        out << *it;
    }
    return out;
}

template <typename Iterator>
class Paginator 
{
    public:
        Paginator(Iterator begin, Iterator end, size_t page_size) 
        {
            for (size_t left = distance(begin, end); left > 0;) 
            {
                const size_t current_page_size = std::min(page_size, left);
                const Iterator current_page_end = next(begin, current_page_size);
                pages_.push_back({ begin, current_page_end });

                left -= current_page_size;
                begin = current_page_end;
            }
        }

        auto begin() const {
            return pages_.begin();
        }

        auto end() const {
            return pages_.end();
        }

        size_t size() const {
            return pages_.size();
        }

    private:
        std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) 
{
    return Paginator(begin(c), end(c), page_size);
}
// end file paginator.h
//---------------------------------------------------------------------------------------
// file read_input_functions.cpp
#include "request_queue.h"

using namespace std;

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status)
{
    const auto result = search_server_.FindTopDocuments(raw_query, status);
    AddRequest(result.size());
    return result;
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query)
{
    const auto result = search_server_.FindTopDocuments(raw_query);
    AddRequest(result.size());
    return result;
}

void RequestQueue::AddRequest(int results_num)
{
    ++current_time_;
    while (!requests_.empty() && min_in_day_ <= current_time_ - requests_.front().timestamp)
    {
        if (0 == requests_.front().results)
        {
            --no_results_requests_;
        }
        requests_.pop_front();
    }

    requests_.push_back({ current_time_, results_num });
    if (0 == results_num) 
    {
        ++no_results_requests_;
    }
}
// end file read_input_functions.cpp
//---------------------------------------------------------------------------------------
// file read_input_functions.h
#pragma once

#include <string>

std::string ReadLine();

int ReadLineWithNumber();
// end file read_input_functions.h
//---------------------------------------------------------------------------------------
// file request_queue.h
#pragma once

#include <deque> 

#include "search_server.h"


class RequestQueue 
{
    public:
        explicit RequestQueue(const SearchServer& search_server)
            : search_server_(search_server)
            , no_results_requests_(0)
            , current_time_(0) {
        }
        // сделаем "обертки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
        template <typename DocumentPredicate>
        std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate)
        {
            const auto result = search_server_.FindTopDocuments(raw_query, document_predicate);
            AddRequest(result.size());

            return result;
        }
        std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
        std::vector<Document> AddFindRequest(const std::string& raw_query);

        int GetNoResultRequests() const
        {
            return no_results_requests_;
        }
    private:
        struct QueryResult
        {
            uint64_t timestamp;
            int results;
        };

        std::deque<QueryResult> requests_;
        const SearchServer& search_server_;
        int no_results_requests_;
        uint64_t current_time_;
        const static int min_in_day_ = 1440;

        void AddRequest(int results_num);
};
// end file request_queue.h
//---------------------------------------------------------------------------------------
// file search_server.cpp
#include <cmath>

#include "search_server.h"

SearchServer::SearchServer(const std::string& stop_words_text): SearchServer(SplitIntoWords(stop_words_text))
{
}

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) 
{
    if ((document_id < 0) || (documents_.count(document_id) > 0)) 
    {
        throw std::invalid_argument("Invalid document_id");
    }
    const auto words = SplitIntoWordsNoStop(document);

    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) 
    {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.push_back(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const 
{
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) 
    {
        return document_status == status;
    });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

int SearchServer::GetDocumentId(int index) const {
    return document_ids_.at(index);
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const 
{
    const auto query = ParseQuery(raw_query);

    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) 
    {
        if (word_to_document_freqs_.count(word) == 0) 
        {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) 
        {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) 
    {
        if (word_to_document_freqs_.count(word) == 0) 
        {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) 
        {
            matched_words.clear();
            break;
        }
    }
    return { matched_words, documents_.at(document_id).status };
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const std::string& word) 
{
    return none_of(word.begin(), word.end(), [](char c) 
    {
        return c >= '\0' && c < ' ';
    });
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const 
{
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) 
    {
        if (!IsValidWord(word)) 
        {
            throw std::invalid_argument("Word " + word + " is invalid");
        }
        if (!IsStopWord(word)) 
        {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) 
{
    if (ratings.empty()) 
    {
        return 0;
    }

    int rating_sum = 0;

    for (const int rating : ratings)
    {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string& text) const 
{
    if (text.empty()) 
    {
        throw std::invalid_argument("Query word is empty");
    }

    std::string word = text;
    bool is_minus = false;

    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }

    if (word.empty() || word[0] == '-' || !IsValidWord(word)) 
    {
        throw std::invalid_argument("Query word " + text + " is invalid");
    }

    return { word, is_minus, IsStopWord(word) };
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const 
{
    Query result;

    for (const std::string& word : SplitIntoWords(text)) 
    {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) 
        {
            if (query_word.is_minus) 
            {
                result.minus_words.insert(query_word.data);
            }
            else 
            {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const 
{
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}
// end file search_server.cpp
//---------------------------------------------------------------------------------------
// file search_server.h
#pragma once

#include <map>
#include <algorithm>

#include "document.h"
#include "string_processing.h"

class SearchServer {
public:

    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    explicit SearchServer(const std::string& stop_words_text);

    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    int GetDocumentCount() const;

    int GetDocumentId(int index) const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

private:

    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> document_ids_;

    bool IsStopWord(const std::string& word) const;

    static bool IsValidWord(const std::string& word);

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(const std::string& text) const;

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    Query ParseQuery(const std::string& text) const;

    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words): stop_words_(MakeUniqueNonEmptyStrings(stop_words))
{
    if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord))
    {
        throw std::invalid_argument("Some of stop words are invalid");
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const 
{
    const auto query = ParseQuery(raw_query);

    auto matched_documents = FindAllDocuments(query, document_predicate);

    sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) 
    {
        if (std::abs(lhs.relevance - rhs.relevance) < 1e-6) 
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

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const 
{
    std::map<int, double> document_to_relevance;

    for (const std::string& word : query.plus_words) 
    {
        if (word_to_document_freqs_.count(word) == 0) 
        {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) 
        {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating)) 
            {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const std::string& word : query.minus_words) 
    {
        if (word_to_document_freqs_.count(word) == 0) 
        {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) 
        {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) 
    {
        matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
    }
    return matched_documents;
}
// end file search_server.h
//---------------------------------------------------------------------------------------
// file string_processing.cpp
#include "string_processing.h"

std::vector<std::string> SplitIntoWords(const std::string& text) 
{
    std::vector<std::string> words;
    std::string word;

    for (const char c : text) 
    {
        if (c == ' ') 

            if (!word.empty()) 
            {
                words.push_back(word);
                word.clear();
            }
        }
        else 
        {
            word += c;
        }
    }
    if (!word.empty()) 
    {
        words.push_back(word);
    }

    return words;
}
// end file search_server.cpp
//---------------------------------------------------------------------------------------
// file string_processing.h
#pragma once

#include <string>
#include <vector>
#include <set>

std::vector<std::string> SplitIntoWords(const std::string& text);

template <typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings) 
{
    std::set<std::string> non_empty_strings;
    for (const std::string& str : strings) 
    {
        if (!str.empty()) 
        {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}
// end file string_processing.h
//---------------------------------------------------------------------------------------
