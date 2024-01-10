#include "archive.h"
#include "Hamming.h"
#include "writingToFile.h"

const uint8_t kSizeOfHeaderWithoutName = 14;
const uint8_t kSizeInHeaderAfterNameX3 = 18;
const uint8_t kSizeInHeaderBeforeNameX3 = 24;

uint32_t DivisionUp(uint32_t a, uint32_t b) {
  return static_cast<uint32_t>((a + b - 1) / b);
}

std::string CreatePath(const std::string& path) {
  return "D:/my_projects/lab6/labwork6-Miraines/" + path + ".haf";
}

std::string CreatePathExtractFile(const std::string& name_archive_without_haf, char* name_file, uint16_t name_size) {
  for (int i = 0; i < name_size; ++i) {
    if (name_file[i] == '/') {
      name_file[i] = '_';
    }
  }
  std::string ans = "Ham_Arc/" + name_archive_without_haf + "/";
  for (int i = 0; i < name_size; ++i) {
    ans = ans + name_file[i];
  }
  return ans;
}

void AddFileToArchive(const Operations& arguments, char* file_name) {

  // Открытие архива для записи данных.
  std::ofstream fout;
  fout.open(CreatePath(arguments.name_of_archive), std::ios::app);
  if (!fout.is_open()) {
    std::cerr << " File archive ne otkrit " << CreatePath(arguments.name_of_archive);
    exit(1);
  }

  // Инициализация структуры для записи битов в файл.
  BitWrite enter_bit_to_archive;
  bool bin_ch[8]; // Массив для хранения двоичного представления байта.

  // Открытие файла, который будет добавлен в архив.
  std::ifstream fin(file_name);
  if (!fin.is_open()) {
    std::cerr << " Oshibka otkritiya file " << file_name;
    exit(1);
  }

  // Вычисление размера файла.
  uint32_t size_of_file;
  fin.seekg(0, std::ios_base::end);
  size_of_file = fin.tellg();
  fin.seekg(0, std::ios_base::beg);

  // Расчет размера заголовка файла в архиве.
  const uint16_t size_of_header = (kSizeOfHeaderWithoutName + strlen(file_name)) * 3;
  const int count_full_part_of_inf_bit = ((size_of_file * 8) / arguments.info_bits);
  uint16_t ost_bits = (size_of_file * 8) % arguments.info_bits;

  // Пересчет размера файла с учетом кодирования Хэмминга и заголовка.
  size_of_file = ((size_of_file * 8) / static_cast<uint32_t>(arguments.info_bits)) *
      (static_cast<uint32_t>(arguments.info_bits) + static_cast<uint32_t>(arguments.dop_bits)) +
      static_cast<uint32_t>(3 * ost_bits);
  size_of_file = DivisionUp(size_of_file, 8);
  size_of_file = size_of_file + static_cast<uint32_t>(size_of_header);
  const uint16_t size_of_name = strlen(file_name);

  // Запись данных о файле в архив (размер заголовка, размер файла, размер имени файла).
  WriteIbytesHamming31(size_of_header, fout, enter_bit_to_archive, 2);
  WriteIbytesHamming31(size_of_file, fout, enter_bit_to_archive, 4);
  WriteIbytesHamming31(size_of_name, fout, enter_bit_to_archive, 2);

  // Запись имени файла в архив с кодированием Хэмминга.
  for (int i = 0; i < strlen(file_name); ++i) {
    uint8_t q = static_cast<uint8_t>(file_name[i]);
    GetBin(q, bin_ch);
    for (int j = 0; j < 8; ++j) {
      enter_bit_to_archive.Enter(bin_ch[j], fout);
      enter_bit_to_archive.Enter(bin_ch[j], fout);
      enter_bit_to_archive.Enter(bin_ch[j], fout);
    }
  }

  // Запись информации о количестве информационных и дополнительных битов в архив.
  WriteIbytesHamming31(arguments.info_bits, fout, enter_bit_to_archive, 2);
  WriteIbytesHamming31(arguments.dop_bits, fout, enter_bit_to_archive, 2);
  WriteIbytesHamming31(ost_bits, fout, enter_bit_to_archive, 2);

  // Инициализация структуры для чтения битов из файла.

  struct BitRead read_bit_brom_file;
  bool mass_info_bits[arguments.info_bits];
  bool kod_slovo[arguments.dop_bits + arguments.info_bits];

  // Чтение, кодирование и запись основной части файла.
  for (int i = 0; i < count_full_part_of_inf_bit; ++i) {
    for (int j = 0; j < arguments.info_bits; ++j) {
      mass_info_bits[j] = read_bit_brom_file.Read(fin);
    }
    Encode(mass_info_bits, arguments.info_bits, arguments.dop_bits, kod_slovo);
    for (int j = 1; j <= (arguments.dop_bits + arguments.info_bits); j++) {
      enter_bit_to_archive.Enter(kod_slovo[j], fout);
    }
  }

  // Чтение, кодирование и запись основной части файла.
  if (ost_bits != 0) {
    for (int i = 0; i < ost_bits; ++i) {
      bool x = read_bit_brom_file.Read(fin);
      enter_bit_to_archive.Enter(x, fout);
      enter_bit_to_archive.Enter(x, fout);
      enter_bit_to_archive.Enter(x, fout);
    }
  }

  // Запись оставшихся битов, не образующих полный байт.
  enter_bit_to_archive.EnterRemainder(fout);
  fout.close();
}

void ExtractOnefile(const Operations& arguments, std::ifstream& fin) {

  // Запоминаем текущую позицию в архиве.
  static uint64_t position = fin.tellg();
  struct BitRead read_bit_from_archive;

  // Чтение заголовка файла (размер заголовка, размер файла, размер имени файла).
  const uint16_t size_of_header = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin, 2));
  const uint32_t size_of_file = ReadIbytesHamming31(read_bit_from_archive, fin, 4);
  const uint16_t size_of_name = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin, 2));

  // Чтение и декодирование имени файла из архива.
  char name[size_of_name + 1];
  bool kod_slovo_hammin31[4];
  bool bin_ch[8];
  for (int k = 0; k < size_of_name; ++k) {
    for (int j = 0; j < 8; ++j) {
      for (int i = 1; i <= 3; ++i) {
        kod_slovo_hammin31[i] = read_bit_from_archive.Read(fin);
      }
      bin_ch[j] = DecodeHamming31(kod_slovo_hammin31);
    }
    name[k] = GetNum(bin_ch);
  }

  // Чтение оставшейся части заголовка после имени файла.
  const uint16_t info_bits = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin, 2));
  const uint16_t dop_bits = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin, 2));
  const uint16_t ost_bits = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin, 2));

  // Открытие файла для записи извлеченных данных.
  struct BitWrite writer;
  std::ofstream fout;
  fout.open(CreatePathExtractFile(arguments.name_of_archive, name, size_of_name), std::ios::binary);
  if (!fout.is_open()) {
    std::cerr << " Ошибка открытия файла " << CreatePathExtractFile(arguments.name_of_archive, name, size_of_name);
    exit(1);
  }

  // Вычисление количества полных кодовых слов в файле.
  const uint64_t count_kod_slov = ((size_of_file - size_of_header) * 8 - 3 * ost_bits) / (info_bits + dop_bits);
  bool kod_slovo[info_bits + dop_bits + 1];
  bool info_slovo[info_bits];

  // Декодирование и запись основной части файла.
  for (uint64_t k = 0; k < count_kod_slov; ++k) {
    for (int i = 1; i <= (info_bits + dop_bits); ++i) {
      kod_slovo[i] = read_bit_from_archive.Read(fin);
    }
    Decode(info_slovo, info_bits, dop_bits, kod_slovo);
    for (int j = 0; j < info_bits; ++j) {
      writer.Enter(info_slovo[j], fout);
    }
  }

  // Чтение, декодирование и запись оставшихся битов файла.
  for (int i = 0; i < ost_bits; ++i) {
    for (int j = 1; j <= 3; j++) {
      kod_slovo_hammin31[j] = read_bit_from_archive.Read(fin);
    }
    bool info_bit_hammin31 = DecodeHamming31(kod_slovo_hammin31);
    writer.Enter(info_bit_hammin31, fout);
  }

  // Дописываем оставшиеся биты, если они не образуют полный байт.
  writer.EnterRemainder(fout);

  // Сдвигаем позицию чтения в архиве на конец обработанного файла.
  position = position + size_of_file;
  fin.seekg(position, std::ios_base::beg);

  fout.close();
}

void ExtractALlfiles(const Operations& arguments) {
  // Открытие архива для чтения.
  std::ifstream fin(CreatePath(arguments.name_of_archive), std::ios::binary);
  if (!fin.is_open()) {
    std::cerr << "Oshibka otkritiya file1";
    exit(1);
  }

  // Создание директории для извлеченных файлов.
  std::filesystem::create_directory("Ham_Arc/" + arguments.name_of_archive);

  // Определение размера архива.
  uint64_t size_of_archive = 0;
  fin.seekg(0, std::ios_base::end);
  size_of_archive = fin.tellg();
  fin.seekg(0, std::ios_base::beg);

  if (size_of_archive == 0) {
    return;
  }

  // Цикл для извлечения всех файлов из архива.
  while (fin.good()) {
    ExtractOnefile(arguments, fin);
    if (fin.tellg() >= size_of_archive) {
      break;
    }
  }
  return;
}

void ExtractSomefiles(const Operations& arguments, char* file_name) {

  // Открытие архива для чтения.
  std::ifstream fin(CreatePath(arguments.name_of_archive), std::ios::binary);
  if (!fin.is_open()) {
    std::cerr << "Oshibka otkritiya file1";
    exit(1);
  }

  // Создание директории для хранения извлеченных файлов.
  std::filesystem::create_directory("Ham_Arc/" + arguments.name_of_archive);

  // Определение размера архива.
  uint64_t size_of_archive = 0;
  fin.seekg(0, std::ios_base::end);
  size_of_archive = fin.tellg();
  fin.seekg(0, std::ios_base::beg);

  // Цикл поиска и извлечения требуемого файла из архива.
  struct BitRead read_bit_from_archive;
  uint64_t position = 0;
  bool flag = true;
  while (flag) {
    // Чтение заголовка файла.
    const uint16_t size_of_header = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin, 2));
    const uint32_t size_of_file = ReadIbytesHamming31(read_bit_from_archive, fin, 4);
    const uint16_t size_of_name = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin, 2));

    // Если размер имени файла не совпадает с искомым, пропускаем его.
    if (size_of_name != strlen(file_name)) {
      position = position + size_of_file;
      fin.seekg(position, std::ios_base::beg);
      if (fin.tellg() >= size_of_archive) { // Проверка на достижение конца архива.
        break;
      }
    } else {
      // Чтение и декодирование имени файла.
      char name[size_of_name + 1];
      bool kod_slovo_hammin31[4];
      bool bin_ch[8];
      for (uint64_t k = 0; k < size_of_name; ++k) {
        for (int j = 0; j < 8; ++j) {
          for (int i = 1; i <= 3; ++i) {
            kod_slovo_hammin31[i] = read_bit_from_archive.Read(fin);
          }
          bin_ch[j] = DecodeHamming31(kod_slovo_hammin31);
        }
        name[k] = GetNum(bin_ch);
      }

      // Проверка, совпадает ли имя файла с искомым.
      bool flag_ravn = true;
      for (uint64_t i = 0; i < size_of_name; ++i) {
        if (name[i] != file_name[i]) {
          flag_ravn = false;
          // Обновление позиции чтения.
          position = position + size_of_file;
          fin.seekg(position, std::ios_base::beg);
          if (fin.tellg() >= size_of_archive) {
            flag = false;
            break;
          }
          break;
        }
      }

      // Если имя файла совпадает с искомым, извлекаем его.
      if (flag_ravn) {
        uint64_t temp = fin.tellg();
        temp = temp + kSizeInHeaderAfterNameX3 - size_of_header;
        fin.seekg(temp, std::ios_base::beg);
        ExtractOnefile(arguments, fin);
        break;
      }
    }
  }
  return;
}

void DeleteSomefiles(const Operations& arguments, char* file_name) {

  // Открытие исходного архива для чтения.
  std::ifstream fin(CreatePath(arguments.name_of_archive), std::ios::binary);
  if (!fin.is_open()) {
    std::cerr << " Oshibka otkritiya archive " << CreatePath(arguments.name_of_archive);
    exit(1);
  }

  // Определение размера исходного архива.
  uint64_t size_of_archive = 0;
  fin.seekg(0, std::ios_base::end);
  size_of_archive = fin.tellg();
  fin.seekg(0, std::ios_base::beg);

  // Создание временного файла для хранения данных из исходного архива.
  std::ofstream fout;
  std::string temp_dir = std::filesystem::temp_directory_path().string();
  fout.open(temp_dir + arguments.name_of_archive, std::ios::binary);
  if (!fout.is_open()) {
    std::cerr << " File vspomogatelnogo archive ne sozdan" << temp_dir + arguments.name_of_archive;
    exit(1);
  }

  struct BitRead read_bit_from_archive; // Структура для чтения битов из файла.
  bool flag = true;
  while (flag) {

    // Чтение заголовка файла.
    const uint16_t size_of_header = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin, 2));
    const uint32_t size_of_file = ReadIbytesHamming31(read_bit_from_archive, fin, 4);
    const uint16_t size_of_name = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin, 2));

    // Проверка является ли текущий файл целевым для удаления.
    if (size_of_name != strlen(file_name)) {
      // Если нет переписываем его во временный файл.
      uint64_t temp = fin.tellg();
      temp = temp - kSizeInHeaderBeforeNameX3;
      // Перемещаемся в начало файла, чтоб записать Header
      fin.seekg(temp, std::ios_base::beg);
      uint8_t ch;
      for (int j = 0; j < (size_of_file); j++) {
        ch = fin.get();
        fout.put(ch);
      }
      if (fin.tellg() >= size_of_archive) {
        break;
      }
    } else {
      // Чтение и декодирование имени файла.
      char name[size_of_name + 1];
      bool kod_slovo_hammin31[4];
      bool bin_ch[8];
      for (int k = 0; k < size_of_name; k++) {
        for (int j = 0; j < 8; j++) {
          for (int i = 1; i <= 3; i++) {
            kod_slovo_hammin31[i] = read_bit_from_archive.Read(fin);
          }
          bin_ch[j] = DecodeHamming31(kod_slovo_hammin31);
        }
        name[k] = GetNum(bin_ch);
      }

      // Проверка, совпадает ли имя файла с искомым.
      bool flag_ravn = true;
      for (int i = 0; i < size_of_name; i++) {
        if (name[i] != file_name[i]) {
          flag_ravn = false;
          // Если не совпадает, переписываем его во временный файл.
          uint64_t temp = fin.tellg();
          temp = temp - size_of_header + kSizeInHeaderAfterNameX3;
          fin.seekg(temp, std::ios_base::beg);
          uint8_t ch;
          for (int j = 0; j < (size_of_file); j++) {
            ch = fin.get();
            fout.put(ch);
          }

          // Проверка на достижение конца архива.
          if (fin.tellg() >= size_of_archive) {
            flag = false;
            break;
          }
          break;
        }
      }
      // Если имя файла совпадает с искомым, пропускаем его.
      if (flag_ravn) {
        uint64_t temp = fin.tellg();
        temp = temp + size_of_file - size_of_header + kSizeInHeaderAfterNameX3;
        fin.seekg(temp, std::ios_base::beg);
        if (fin.tellg() >= size_of_archive) {
          flag = false;
        }
      }
    }
  }

  // Удаление исходного архива и переименование временного файла в основной архив.
  remove(CreatePath(arguments.name_of_archive).c_str());
  if (rename((temp_dir + arguments.name_of_archive).c_str(), CreatePath(arguments.name_of_archive).c_str()) !=
      0) {
    std::cerr << " Oshibka peremesheniya file " << std::endl;
  }
  return;
}

void CreateArchive(const Operations& arguments) {
  std::ofstream fout;
  fout.open(CreatePath(arguments.name_of_archive), std::ios::app);
  if (!fout.is_open()) {
    std::cerr << " File archive ne sozdan " << CreatePath(arguments.name_of_archive);
    exit(1);
  }

  // Проверка на наличие файлов для архивации.
  if (arguments.files.empty()) {
    fout.close();
    return;
  }

  // Добавление каждого файла из списка в архив.
  for (int i = 0; i < arguments.files.size(); ++i) {
    AddFileToArchive(arguments, arguments.files[i]);
  }
}

void PrintOneName(std::ifstream& fin) {
  static uint64_t position = 0;
  struct BitRead read_bit_from_archive;

  // Чтение информации о файле (размер заголовка, размер файла, размер имени).
  const uint16_t size_of_header = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin, 2));
  const uint32_t size_of_file = ReadIbytesHamming31(read_bit_from_archive, fin, 4);
  const uint16_t size_of_name = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin, 2));

  // Чтение и декодирование имени файла.
  char name[size_of_name + 1]; // Массив для хранения имени файла.
  bool kod_slovo_hammin31[4]; // Для хранения кодового слова Хэмминга.
  bool bin_ch[8]; // Для хранения двоичного представления символа.
  for (int k = 0; k < size_of_name; ++k) {
    for (int j = 0; j < 8; ++j) {
      for (int i = 1; i <= 3; ++i) {
        kod_slovo_hammin31[i] = read_bit_from_archive.Read(fin);
      }
      bin_ch[j] = DecodeHamming31(kod_slovo_hammin31);
    }
    name[k] = GetNum(bin_ch); // Преобразование двоичных данных в символ.
  }

  // Вывод имени файла.
  for (int i = 0; i < size_of_name; ++i) {
    std::cout << name[i];
  }

  // Передвижение указателя чтения к следующему файлу в архиве.
  position = position + size_of_file;
  fin.seekg(position, std::ios_base::beg);
}

void PrintNamesFile(const std::string& archive_name) {
  std::ifstream fin(CreatePath(archive_name), std::ios::binary);
  if (!fin.is_open()) {
    std::cerr << " Oshibka otkritiya file " << CreatePath(archive_name);
    exit(1);
  }

  // Определение размера архива.
  uint64_t size_of_archive = 0;
  fin.seekg(0, std::ios_base::end);
  size_of_archive = fin.tellg();
  fin.seekg(0, std::ios_base::beg);

  if (size_of_archive == 0) {
    return;
  }

  // Цикл вывода имен всех файлов в архиве.
  while (fin.good()) {
    PrintOneName(fin);
    if (fin.tellg() >= size_of_archive) { // Проверка на достижение конца архива.
      break;
    }
    std::cout << std::endl;
  }
  return;
}

void MergeArchive(const std::string& name_of_archive,
                  const std::string& second_name_of_archive,
                  const std::string& third_name_of_archive) {
  // Создание (открытие) файла нового архива для записи.
  std::ofstream fout;
  fout.open(CreatePath(third_name_of_archive), std::ios::app);
  if (!fout.is_open()) {
    std::cerr << " File archive ne sozdan " << CreatePath(third_name_of_archive);
    exit(1);
  }

  // Открытие первого архива для чтения.
  std::ifstream fin1(CreatePath(name_of_archive), std::ios::binary);
  if (!fin1.is_open()) {
    std::cerr << " Oshibka otkritiya file " << CreatePath(name_of_archive);
    exit(1);
  }

  // Открытие второго архива для чтения.
  std::ifstream fin2(CreatePath(second_name_of_archive), std::ios::binary);
  if (!fin2.is_open()) {
    std::cerr << " Oshibka otkritiya file " << CreatePath(second_name_of_archive);
    exit(1);
  }

  // Копирование содержимого первого архива в новый архив.
  uint8_t q;
  while (fin1.good()) {
    q = fin1.get();
    if (fin1.good()) {
      fout.put(q);
    }
  }

  // Копирование содержимого второго архива в новый архив.
  while (fin2.good()) {
    q = fin2.get();
    if (fin2.good()) {
      fout.put(q);
    }
  }
  fout.close();
}
