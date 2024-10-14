#ifndef INSERTVALUE_HPP
#define INSERTVALUE_HPP

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

#include "../Other/Utilities.hpp"
#include "../CustomStructures/MyHashMap.hpp"
#include "../CustomStructures/MyVector.hpp"

using namespace std;

// Функция для удаления апострофов из строки
string CleanText(string& str) {
    // Удаление запятой и скобки в конце строки
    if (str[str.size() - 1] == ',' && str[str.size() - 2] == ')') {
        str = getSubstring(str, 0, str.size() - 2);
    } else if (str[str.size() - 1] == ',' || str[str.size() - 1] == ')') {
        str = getSubstring(str, 0, str.size() - 1);
    }

    // Удаление апострофов в начале и конце строки
    if (str[0] == '\'' && str[str.size() - 1] == '\'') {
        str = getSubstring(str, 1, str.size() - 1);
        return str;
    } else {
        throw runtime_error("Неверный синтаксис в VALUES " + str);
    }
}

// Функция для проверки количества аргументов относительно столбцов таблиц
void Validate(int colLen, const MyVector<string>& namesOfTable, const MyHashMap<string, MyVector<string>*>& jsonStructure) {
    for (int i = 0; i < namesOfTable.length; i++) {
        MyVector<string>* temp = GetMap<string, MyVector<string>*>(jsonStructure, namesOfTable.data[i]);
        if (temp->length != colLen) {
            throw runtime_error("Количество аргументов не равно столбцам в " + namesOfTable.data[i]);
        }
    }
}

// Функция для чтения или записи первичного ключа
int readPrKey(const string& path, const bool rec, const int newID) {
    fstream pkFile(path);
    if (!pkFile.is_open()) {
        throw runtime_error("Не удалось открыть " + path);
    }
    int lastID = 0;
    if (rec) {
        pkFile << newID;
    } else {
        pkFile >> lastID;
    }
    pkFile.close();
    return lastID;
}

// Функция для добавления строк в файл
void insertRows(MyVector<MyVector<string>*>& addNewData, MyVector<string>& namesOfTable, const string& nameOfSchema, const int limitOfTuples, const string& filePath) {
    for (int i = 0; i < namesOfTable.length; i++) {
        int lastID = 0;
        try {
            CheckTableLock(filePath + "/" + nameOfSchema + "/" + namesOfTable.data[i], namesOfTable.data[i] + "_lock.txt", 1);
            lastID = readPrKey(filePath + "/" + nameOfSchema + "/" + namesOfTable.data[i] + "/" + namesOfTable.data[i] + "_pk_sequence.txt", false, 0);
        } catch (const std::exception& err) {
            cerr << err.what() << endl;
            return;
        }

        int newID = lastID;
        for (int j = 0; j < addNewData.length; j++) {
            newID++;
            string tempPath;
            if (lastID / limitOfTuples < newID / limitOfTuples) {
                tempPath = filePath + "/" + nameOfSchema + "/" + namesOfTable.data[i] + "/" + to_string(newID / limitOfTuples + 1) + ".csv";
            } else {
                tempPath = filePath + "/" + nameOfSchema + "/" + namesOfTable.data[i] + "/" + to_string(lastID / limitOfTuples + 1) + ".csv";
            }
            fstream csvFile(tempPath, ios::app);
            if (!csvFile.is_open()) {
                throw runtime_error("Не удалось открыть " + tempPath);
            }
            csvFile << endl << newID;
            for (int k = 0; k < addNewData.data[j]->length; k++) {
                csvFile << "," << addNewData.data[j]->data[k];
            }
            csvFile.close();
        }
        readPrKey(filePath + "/" + nameOfSchema + "/" + namesOfTable.data[i] + "/" + namesOfTable.data[i] + "_pk_sequence.txt", true, newID);
        CheckTableLock(filePath + "/" + nameOfSchema + "/" + namesOfTable.data[i], namesOfTable.data[i] + "_lock.txt", 0);
    }
}

// Функция для парсинга команды INSERT
void parseInsert(const MyVector<string>& slovs, const string& filePath, const string& nameOfSchema, const int limitOfTuples, const MyHashMap<string, MyVector<string>*>& jsonStructure) {
    MyVector<string>* targetTables = CreateVector<string>(5, 50);
    MyVector<MyVector<string>*>* dataToInsert = CreateVector<MyVector<string>*>(10, 50);
    bool afterValues = false;
    int countOfTable = 0;
    int dataCount = 0;
    for (int i = 2; i < slovs.length; i++) {
        if (slovs.data[i][slovs.data[i].size() - 1] == ',') {
            slovs.data[i] = getSubstring(slovs.data[i], 0, slovs.data[i].size() - 1);
        }
        if (slovs.data[i] == "VALUES") {
            afterValues = true;
        } else if (afterValues) {
            dataCount++;
            if (slovs.data[i][0] == '(') {
                MyVector<string>* tempData = CreateVector<string>(5, 50);
                slovs.data[i] = getSubstring(slovs.data[i], 1, slovs.data[i].size());

                while (slovs.data[i][slovs.data[i].size() - 1] != ')' && slovs.data[i][slovs.data[i].size() - 2] != ')') {
                    try {
                        CleanText(slovs.data[i]);
                    } catch (const exception& err) {
                        cerr << err.what() << " " << slovs.data[i] << endl;
                        return;
                    }

                    AddVector<string>(*tempData, slovs.data[i]);
                    i++;
                }
                try {
                    CleanText(slovs.data[i]);
                    AddVector<string>(*tempData, slovs.data[i]);
                    Validate(tempData->length, *targetTables, jsonStructure);
                } catch (const exception& err) {
                    cerr << err.what() << endl;
                    return;
                }
                AddVector<MyVector<string>*>(*dataToInsert, tempData);
            }

        } else {
            countOfTable++;
            try {
                GetMap(jsonStructure, slovs.data[i]);
            } catch (const exception& err) {
                cerr << err.what() << ": Таблица " << slovs.data[i] << " отсутствует" << endl;
                return;
            }
            AddVector<string>(*targetTables, slovs.data[i]);
        }
    }
    if (countOfTable == 0 || dataCount == 0) {
        throw runtime_error("Отсутствует имя таблицы или данные в VALUES");
    }

    try {
        insertRows(*dataToInsert, *targetTables, nameOfSchema, limitOfTuples, filePath);
    } catch (const exception& err) {
        cerr << err.what() << endl;
        return;
    }
}

#endif // INSERT_HPP