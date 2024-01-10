#include "parsing.h"
#include "archive.h"

int main(int argc, char** argv) {
  std::filesystem::create_directory("D:/my_projects/lab6/labwork6-Miraines/Ham_Arc");

  struct Operations arguments;
  Parsing(argc, argv, arguments);

  for (int i = 0; i < 6; ++i) {
    if (arguments.flag[Create]) {
      CreateArchive(arguments);
      break;
    }

    if (arguments.flag[List]) {
      PrintNamesFile(arguments.name_of_archive);
      break;
    }

    if (arguments.flag[Extract]) {
      if (arguments.all_files) {
        ExtractALlfiles(arguments);
      } else {
        ExtractSomefiles(arguments, arguments.files[0]);
      }
      break;
    }

    if (arguments.flag[Append]) {
      AddFileToArchive(arguments, arguments.files[0]);
      break;
    }

    if (arguments.flag[Delete]) {
      DeleteSomefiles(arguments, arguments.files[0]);
      break;
    }

    if (arguments.flag[Concatenate]) {
      MergeArchive(arguments.name_of_archive, arguments.second_name_of_archive, arguments.third_name_of_archive);
      break;
    }
  }
}
