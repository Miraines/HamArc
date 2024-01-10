#pragma once
#include <iostream>
#include <cstring>
#include <vector>
#include <math.h>
#include <cstdint>

//Проверяет подходит ли данное соотношение инф/доп битов и если нет корректирует это
bool CheckGoodArguments(uint16_t count_inf_bit, uint16_t& count_dop_bit);

//Если не указано --dop, высчитывает его
uint16_t CounterAdditionalBits(uint16_t count_inf_bit);

//Если не указано --info, высчитывает его
uint16_t CounterInfoBits(uint16_t count_dop_bit);

void Encode(bool inf_bits[], int count_inf_bits, int count_dop_bits, bool KodSlovo[]);

void GetInfoBits(bool inf_bits[], int count_inf_bits, int count_dop_bits, bool KodSlovo[]);

// Проверяет правильные ли все биты в кодовом слове и определяет ошибочные дополнительные биты
bool CheckGoodCodeWord(int count_inf_bits, int count_dop_bits, bool KodSlovo[], std::vector<int>& mistakes_dop_bits);

void Decode(bool inf_bits[], int count_inf_bits, int count_dop_bits, bool KodSlovo[]);

bool DecodeHamming31(bool KodSlovo[]);
