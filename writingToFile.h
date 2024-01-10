#pragma once

#include <iostream>
#include <fstream>
#include <math.h>
#include <cstring>
#include <cstdint>

// Структура для записи битов в файл.
struct BitWrite {
  bool bitsets[8];
  int size_now; // Текущее количество записанных битов в массив.

  BitWrite() {
    for (int i = 0; i < 8; ++i) {
      bitsets[i] = false;
    }
    size_now = 0;
  };

  // Функция для добавления бита в массив и записи в файл при заполнении массива.
  void Enter(bool x, std::ofstream& fout) {
    bitsets[size_now] = x;
    size_now++;
    if (size_now == 8) {
      uint8_t q = 0; // Переменная для формирования байта из битов.
      int j = 0;
      // Конвертация битов в байт.
      for (int i = 7; i >= 0; --i) {
        if (bitsets[i]) {
          q = q + static_cast<uint8_t>(pow(2, j));
        }
        ++j;
      }
      size_now = 0;

      //Сбрасываем массив и записываем все в файл
      for (int i = 0; i < 8; ++i) {
        bitsets[i] = false;
      }
      fout.put(q);
    }
  };

  // Функция для записи оставшихся битов, не образующих полный байт.
  void EnterRemainder(std::ofstream& fout) {
    if (size_now == 0) {
      return;
    }
    //Дополняем до полного байта
    while (size_now != 8) {
      bitsets[size_now] = false;
      ++size_now;
    }
    uint8_t q = 0;
    int j = 0;
    for (int i = 7; i >= 0; --i) {
      if (bitsets[i]) {
        q = q + static_cast<uint8_t>(pow(2, j));
      }
      ++j;
    }
    size_now = 0;
    fout.put(q);
  };
};

// Структура для чтения битов из файла.
struct BitRead {
  bool bitsets[8];
  int size_now;
  int current; // Текущая позиция чтения в массиве.

  BitRead() {
    for (int i = 0; i < 8; ++i) {
      bitsets[i] = false;
    }
    size_now = 0;
    current = 8;
  }

  // Функция для чтения следующего бита из файла.
  bool Read(std::ifstream& fout) {
    if (size_now != 0) {
      current++;
      size_now--;
      return bitsets[current - 1];
    } else {
      size_now = 7;
      current = 1;
      uint8_t q;
      q = fout.get();

      // Разбиваем байт на отдельные биты и сохраняем в массив.
      for (int i = 7; i >= 0; --i) {
        bitsets[i] = q % 2;
        q = q / 2;
      }

      return bitsets[0];
    }
  }
};

// Преобразует целое число в массив байтов в формате little-endian.
void ToNumbersI(uint32_t x, uint8_t numbers[], uint8_t byte);

// Преобразуем один байт в двоичное представление.
void GetBin(uint8_t x, bool bin_ch[]);

// Преобразуем двоичное представление обратно в байт
uint8_t GetNum(bool bin_ch[]);

// Записывает I-байтовое число x в файл fout.
void WriteIbytesHamming31(uint32_t x, std::ofstream& fout, BitWrite& enter_bit_to_archive, uint8_t byte);

// Считывает из файла данные для I байтового числа декодирует их и переводит в десятичную систему.
uint32_t ReadIbytesHamming31(struct BitRead& read_bit_from_archive, std::ifstream& fin, uint8_t byte);