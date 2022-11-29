#include "test_example_functions.h"
#include "search_server.h"

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, 
                const std::string& func, unsigned line, const std::string& hint) 
{
    if (!value) 
    {
        std::cout << file << "(" << line << "): " << func << ": ";
        std::cout << "ASSERT(" << expr_str << ") failed.";
        if (!hint.empty()) {
            std::cout << " Hint: " << hint;
        }
        std::cout << '\n';
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

void TestAddingDocument() 
{
    const int doc_id = 42;
    const std::string doc = "cat in the city";
    const std::vector<int> ratings = { 1, 2, 3 };
    SearchServer search_server;
    search_server.AddDocument(doc_id, doc, DocumentStatus::ACTUAL, ratings);
   
    const auto found_docs = search_server.FindTopDocuments("cat in the city");
    ASSERT_HINT(found_docs.size() == 1, "Something wrong with adding document");
    ASSERT_HINT(found_docs.at(0).id == 42, "Something wrong with reading document_id");
    ASSERT_HINT(found_docs.at(0).rating == 2, "Something wrong with calculating average rating");
}