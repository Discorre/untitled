#include <iostream>
#include <filesystem>

#include "CustomStructures/MyVector.hpp"
#include "CustomStructures/MyHashMap.hpp"

#include "Other/JsonParser.hpp"
#include "Other/Utilities.hpp"
#include "CRUDOperations/SelectValue.hpp"
#include "CRUDOperations/InsertValue.hpp"
#include "CRUDOperations/DeleteValue.hpp"

using namespace std;

// Парсит и выполняет SQL-запросы
void parsingQuery(const string& query, const string& filePath, const string& namesOfSchema, const int limitOfTuples, const MyHashMap<string, MyVector<string>*>& jsonStructure) {
    MyVector<string>* words = splitRow(query, ' ');  // Разбиваем запрос на слова
    if (words->data[0] == "SELECT") {
        try {
            parseSelect(*words, filePath, namesOfSchema, jsonStructure);  // Выполняем SELECT запрос
        } catch (const exception& err) {
            cerr << err.what() << endl;  // Выводим ошибку, если она возникла
        }
    
    } else if (words->data[0] == "INSERT" && words->data[1] == "INTO") {
        try {
            parseInsert(*words, filePath, namesOfSchema, limitOfTuples, jsonStructure);  // Выполняем INSERT запрос
        } catch (const exception& err) {
            cerr << err.what() << endl;  // Выводим ошибку, если она возникла
        }
    
    } else if (words->data[0] == "DELETE" && words->data[1] == "FROM") {
        try {
            parseDelete(*words, filePath, namesOfSchema, jsonStructure);  // Выполняем DELETE запрос
        } catch (const exception& err) {
            cerr << err.what() << endl;  // Выводим ошибку, если она возникла
        }
        
    } else { 
        cout << "Неизвестная команда" << endl;  // Выводим сообщение, если команда не распознана
    }
}

// Ввод имени файла и директории
int InputNames(string& jsonFileName, string& filePath) {
    while (true) {
        cout << "Введите имя json файла: ";
        getline(cin, jsonFileName);
        cout << "Введите путь к файлу: ";
        getline(cin, filePath);

        try {
            if (!filesystem::exists(filePath + "\\" + jsonFileName)) {
                throw std::runtime_error("Файл JSON не найден");  // Выбрасываем ошибку, если файл не найден
            } else {
                return 0;
            }
        } catch (const exception& err) {
            cerr << "Ошибка: " << err.what() << endl;  // Выводим ошибку, если она возникла
        }
    }
}

int main() {
    string jsonFileName;
    string filePath;
    //InputNames(jsonFileName, filePath);
    jsonFileName = "schema.json";
    filePath = ".";

    MyHashMap<string, MyVector<string>*>* jsonStructure = CreateMap<string, MyVector<string>*>(10, 50);  // Создаем хэш-таблицу для структуры JSON

    // Создание директорий
    int limitOfTuples = 0;
    string namesOfSchema = readJsonFile(jsonFileName, filePath, limitOfTuples, *jsonStructure);  // Читаем JSON файл и создаем директории
    while (true) {
        cout << endl;
        cout << "Введите запрос для выполнения или exit для выхода из программы: " << endl;
        cout << ">>> ";
        string query;
        getline(cin, query);
        // Парсинг запроса
        if (query == "exit") break;  // Выход из цикла, если введенo "exit"
        cout << endl;
        parsingQuery(query, filePath, namesOfSchema, limitOfTuples, *jsonStructure);  // Парсим и выполняем запрос
    }
    
    DestroyMap<string, MyVector<string>*>(*jsonStructure);  // Освобождаем память

    return 0;
}