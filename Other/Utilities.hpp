#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <string>

#include "../CustomStructures/MyVector.hpp"

#include <fstream>

using namespace std;

// Возвращает длину строки
int getLen(const string &str) {
    int length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

// Возвращает подстроку от start до end (не включая end)
string getSubstring(const string &str, int start, int end) {
    string result;
    for (int i = start; i < end; i++) {
        result += str[i];
    }
    return result;
}

// Разбивает строку на слова с разделителем delim.
MyVector<string>* splitRow(const string &str, char delim) {
    int index = 0;
    MyVector<string>* words = CreateVector<string>(10, 50);  // Создаем вектор для слов
    int length = getLen(str);

    while (true) {
        int delimIndex = index;
        while (str[delimIndex] != delim && delimIndex != length) delimIndex++;  // Ищем разделитель

        string word = getSubstring(str, index, delimIndex);  // Получаем слово
        AddVector(*words, word);  // Добавляем слово в вектор
        index = delimIndex + 1;
        if (delimIndex == length) break;  // Если достигли конца строки, выходим из цикла
    }

    return words;
}

// Проверка на занятость таблицы другим пользователем
void CheckTableLock(const string& path, const string& fileName, const int rank) {
    fstream lockFile(path + "/" + fileName);
    if (!lockFile.is_open()) {
        throw runtime_error("Не удалось открыть " + (path + "/" + fileName));  // Выбрасываем ошибку, если не удалось открыть файл
    }
    int lock = 0;
    lockFile >> lock;  // Читаем текущее состояние блокировки
    if (lock == 1 && rank == 1) {
        lockFile.close();
        throw runtime_error("Таблица " + fileName + " заблокирована другим процессом");  // Выбрасываем ошибку, если таблица заблокирована
    } else {
        lockFile << rank;  // Устанавливаем новое состояние блокировки
    }
    lockFile.close();
}

#endif