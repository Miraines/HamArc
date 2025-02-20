#include "parsing.h"
#include "Hamming.h"

void CheckAdditionalArguments(Operations& arguments) {
  if (arguments.flag[DopBits] && arguments.flag[InfoBits]) {
    if (!CheckGoodArguments(arguments.info_bits, arguments.dop_bits)) {
      std::cerr << "Ne mogu zakodirovat' s takimi parametrami";
      exit(1);
    }
  }

  if (arguments.flag[DopBits] && !arguments.flag[InfoBits]) {
    arguments.info_bits = CounterInfoBits(arguments.dop_bits);
  }

  if (!arguments.flag[DopBits] && arguments.flag[InfoBits]) {
    arguments.dop_bits = CounterAdditionalBits(arguments.info_bits);
  }
}

void GetNames(int i, int argc, char** argv, Operations& arguments) {
  for (int j = i + 1; j < argc; ++j) {
    if (argv[j][0] == '-') {
      if (strcmp(argv[j], "-f") == 0) {
        ++j;
        arguments.name_of_archive = argv[j];
      } else if (argv[j][2] == 'f') {
        if (strncmp(argv[j], "--file=", 7) == 0) {
          for (int l = 7; l < strlen(argv[j]); ++l) {
            arguments.name_of_archive = arguments.name_of_archive + argv[j][l];
          }
        }
      } else if (strcmp(argv[j], "--info") == 0) {
        j++;
        arguments.info_bits = static_cast<uint64_t>(atoi(argv[j]));
        arguments.flag[InfoBits] = true;
      } else if (strcmp(argv[j], "--dop") == 0) {
        j++;
        arguments.dop_bits = static_cast<uint64_t>(atoi(argv[j]));
        arguments.flag[DopBits] = true;
      }

    } else {
      arguments.files.push_back(argv[j]);
    }
  }
}

void Parsing(int argc, char** argv, Operations& arguments) {
  for (int i = 0; i < 6; ++i) {
    arguments.flag[i] = false;
  }

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--create") == 0) {
      arguments.flag[Create] = true;
      GetNames(i, argc, argv, arguments);

      if (arguments.files.size() != 0 && !arguments.flag[DopBits] &&
          !arguments.flag[InfoBits]) {
        std::cerr << "Ne vvedeni obyazatelnie parametri kodirovki";
        exit(1);
      }
      CheckAdditionalArguments(arguments);
      break;

    } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
      GetNames(i, argc, argv, arguments);
      arguments.flag[List] = true;
      break;

    } else if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--extract") == 0) {
      arguments.all_files = false;
      arguments.flag[Extract] = true;
      GetNames(i, argc, argv, arguments);

      if (arguments.files.size() == 0) {
        arguments.all_files = true;
      }
      break;

    } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--append") == 0) {
      GetNames(i, argc, argv, arguments);
      arguments.flag[Append] = true;

      if (!arguments.flag[DopBits] && !arguments.flag[InfoBits]) {
        std::cerr << "Ne vvedeni obyazatelnie parametri kodirovki";
        exit(1);
      }
      CheckAdditionalArguments(arguments);
      break;

    } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delete") == 0) {
      GetNames(i, argc, argv, arguments);
      arguments.flag[Delete] = true;
      break;

    } else if (strcmp(argv[i], "-A") == 0 || strcmp(argv[i], "--concantenate") == 0) {
      arguments.flag[Concatenate] = true;
      bool first_arch = false;
      ++i;

      for (i; i < argc; ++i) {
        if (strcmp(argv[i], "-f") == 0) {
          i++;
          arguments.third_name_of_archive = argv[i];
        } else if (argv[i][0] == '-') {
          if (strncmp(argv[i], "--file=", 7) == 0) {
            for (int l = 7; l < strlen(argv[i]); ++l) {
              arguments.third_name_of_archive = arguments.third_name_of_archive + argv[i][l];
            }
          }
        } else {
          if (!first_arch) {
            arguments.name_of_archive = argv[i];
            first_arch = true;
          } else {
            arguments.second_name_of_archive = argv[i];
          }
        }
      }
      break;

    } else {
      std::cerr << "unknown option";
      break;
    }
  }
}
