void TestStopWordExeption() // Поддержка стоп-слов. Стоп-слова исключаются из текста документов.
{
    SearchServer server;

    int id = 41;
    string content = "stop the war and stop putin"s;
    vector<int> ratings = { 1,2,3 };
    server.AddDocument(id, content, DocumentStatus::ACTUAL, ratings);

    string stop_word = "the and"s;
    server.SetStopWords(stop_word);
    ASSERT(server.FindTopDocuments(stop_word).empty());
}

void TestMinusWordExeption()    // Поддержка минус-слов. Документы, содержащие минус-слова поискового запроса, 
{                               // не должны включаться в результаты поиска.    
    SearchServer server;

    int id_1 = 26;
    string content_1 = "stop the war and stop putin"s;
    vector<int> ratings_1 = { 1,2,3 };

    const string minus_word = "-putin"s;
    const string stop_word = "the and"s;

    server.SetStopWords(stop_word);
    server.SetStopWords(minus_word);
    server.AddDocument(id_1, content_1, DocumentStatus::ACTUAL, ratings_1);

    int id_2 = 41;
    string content_2 = "peace in the world stop war"s;
    vector<int> ratings_2 = { 1,1,2 };

    server.SetStopWords(stop_word);
    server.SetStopWords(minus_word);
    server.AddDocument(id_2, content_2, DocumentStatus::ACTUAL, ratings_2);

    ASSERT(server.FindTopDocuments(minus_word).empty());
}

void TestMatchDocument()    // Матчинг документов. При матчинге документа по поисковому запросу должны быть 
{                           // возвращены все слова из поискового запроса, присутствующие в документе. 
    SearchServer server;    // Если есть соответствие хотя бы по одному минус-слову, должен возвращаться 
                            // пустой список слов.

    int id_1 = 26;
    string content_1 = "the world is in danger"s;
    vector<int> ratings_1 = { 1,2,3 };

    int id_2 = 41;
    string content_2 = "death is salvation"s;
    vector<int> ratings_2 = { 1,1,2 };

    server.SetStopWords("the is in"s);

    server.AddDocument(id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
    server.AddDocument(id_2, content_2, DocumentStatus::ACTUAL, ratings_2);

    auto [str_1, _1] = server.MatchDocument("world"s, id_1);
    ASSERT(!str_1.empty());

    auto [str_2, _2] = server.MatchDocument("-death"s, id_2);
    ASSERT(str_2.empty());
}

void TestSortByRelevance()  // Сортировка найденных документов по релевантности. Возвращаемые при поиске 
{                           // документов результаты должны быть отсортированы в порядке убывания релевантности.
    SearchServer server;

    int id1 = 14;
    int id2 = 21;
    int id3 = 32;
    int id4 = 8;
    int id5 = 2;

    string s1 = "I live in a small city"s;
    string s2 = "live in the city small city"s;
    string s3 = "small city in a big country"s;

    string s4 = "city small and city dirt in lol city"s;
    string s5 = "monstr live in the city"s;

    vector<int> r1 = { 1,2,3 };
    vector<int> r2 = { 3,7,1 };
    vector<int> r3 = { 4,1,2 };
    vector<int> r4 = { 1,13,5 };
    vector<int> r5 = { 3,7,12 };

    server.SetStopWords("the in a"s);
    server.AddDocument(id1, s1, DocumentStatus::ACTUAL, r1);
    server.AddDocument(id2, s2, DocumentStatus::ACTUAL, r2);
    server.AddDocument(id3, s3, DocumentStatus::ACTUAL, r3);

    const auto found_docs = server.FindTopDocuments("live city"s);

    const Document& doc1 = found_docs[0];
    const Document& doc2 = found_docs[1];
    const Document& doc3 = found_docs[2];

    ASSERT_EQUAL(doc1.id, id2);
    ASSERT_EQUAL(doc2.id, id1);
    ASSERT_EQUAL(doc3.id, id3);

    server.AddDocument(id4, s4, DocumentStatus::ACTUAL, r4);
    server.AddDocument(id5, s5, DocumentStatus::ACTUAL, r5);

    const auto found_docs1 = server.FindTopDocuments("live city"s);

    const Document& doc4 = found_docs1[0];
    const Document& doc5 = found_docs1[1];

    ASSERT_EQUAL(doc4.id, id5);
    ASSERT_EQUAL(doc5.id, id2);

    const auto find = server.FindTopDocuments("live city"s);
    const Document& d1 = find[0];
    const Document& d2 = find[1];
    const Document& d3 = find[2];
    const Document& d4 = find[3];
    const Document& d5 = find[4];

    ASSERT_EQUAL(d1.id, id5);
    ASSERT_EQUAL(d2.id, id2);
    ASSERT_EQUAL(d3.id, id1);
    ASSERT_EQUAL(d4.id, id4);
    ASSERT_EQUAL(d5.id, id3);
}

void TestRatingDocument()   //Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему
{                           //арифметическому оценок документа.
    SearchServer server;

    int id = 26;
    string s = "sorry you are dead"s;
    vector<int> r = { 1,2,3 };

    server.SetStopWords("the in a are"s);
    server.AddDocument(id, s, DocumentStatus::ACTUAL, r);

    const auto find = server.FindTopDocuments("dead"s);
    const Document& d = find[0];

    ASSERT_EQUAL(d.rating, 2);
}

void TestFoundStatus() //Поиск документов, имеющих заданный статус.
{
    SearchServer server;

    int id1 = 14;
    int id2 = 21;
    int id3 = 32;
    int id4 = 8;
    int id5 = 2;

    string s1 = "I live in a small city"s;
    string s2 = "live in the city small city"s;
    string s3 = "small city in a big country"s;
    string s4 = "city small and city dirt in lol city"s;
    string s5 = "monstr live in the city"s;

    vector<int> r1 = { 1,2,3 };
    vector<int> r2 = { 3,7,1 };
    vector<int> r3 = { 4,1,2 };
    vector<int> r4 = { 1,13,5 };
    vector<int> r5 = { 3,7,12 };

    server.SetStopWords("the in a"s);
    server.AddDocument(id1, s1, DocumentStatus::ACTUAL, r1);
    server.AddDocument(id2, s2, DocumentStatus::BANNED, r2);
    server.AddDocument(id3, s3, DocumentStatus::ACTUAL, r3);
    server.AddDocument(id4, s4, DocumentStatus::BANNED, r4);
    server.AddDocument(id5, s5, DocumentStatus::ACTUAL, r5);

    const auto find = server.FindTopDocuments("live city"s, DocumentStatus::ACTUAL);
    const Document& d1 = find[0];
    const Document& d2 = find[1];
    const Document& d3 = find[2];

    ASSERT_EQUAL(d1.id, id5);
    ASSERT_EQUAL(d2.id, id1);
    ASSERT_EQUAL(d3.id, id3);
}


void TestSearchServer()
{
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestFoundStatus);
    RUN_TEST(TestRatingDocument);
    RUN_TEST(TestSortByRelevance);
    RUN_TEST(TestMatchDocument);
    RUN_TEST(TestMinusWordExeption);
    RUN_TEST(TestStopWordExeption);
    RUN_TEST(TestAddDocumentsFindRequest);
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() 
{
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}
