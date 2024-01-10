#pragma once

#include <iostream>
#include <cstring>
#include <vector>
#include <cstdint>

enum Options {
  Create = 0, List = 1, Extract = 2, Append = 3, Delete = 4, Concatenate = 5, InfoBits = 6, DopBits = 7
};

// info_bits или dop_bits при Create,Append
struct Operations {
  std::string name_of_archive;
  std::vector<char*> files;
  uint16_t info_bits;
  uint16_t dop_bits;
  // для extract
  bool all_files;
  // для concatenate
  std::string second_name_of_archive;
  std::string third_name_of_archive;
  bool flag[8];
};

void GetNames(int i, int argc, char** argv, Operations& arguments);
void Parsing(int argc, char** argv, Operations& arguments);
void CheckAdditionalArguments(Operations& arguments);