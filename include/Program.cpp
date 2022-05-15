// Copyright 2022 vlados2003
#include <iostream>
#include <exception>
#include <thread>
#include <vector>
#include <chrono>
#include <boost/program_options.hpp>
#include <fstream>
#include "Program.h"
#include "Loader.h"
#include "Parser.h"
namespace po = boost::program_options;
bool Program::isRunning;
//Парсим аргументы коммандной строки
bool Program::ParseArgs(int argc, char **argv)
{
  //Указываем описание
  po::options_description desc("Allowed options");
  desc.add_options()
      ("depth", po::value<int>(), "глубина поиска по странице")
      ("url", po::value<std::string>(), "адрес HTML страницы")
      ("network_threads", po::value<int>(),
          "количество потоков для скачивания страниц")
      ("parser_threads", po::value<int>(),
          "количество потоков для обработки страниц")
          ("output", po::value<std::string>(), "путь до выходного файла");
  po::variables_map map;
  //Парсинг аргументов
  try {
    po::store(po::parse_command_line(argc, argv, desc), map);
    po::notify(map);
  } catch (std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return false;
  }
  //Прверяем переданы ли все аргументы
  if (!map.count("depth") || !map.count("url")
      || !map.count("network_threads") || !map.count("parser_threads")
      || !map.count("output"))
  {
    //Выдаем ошибку
    std::cout << "Not enough args\n";
    return false;
  }
  //Читаем аргументы
  depth = map["depth"].as<int>();
  network_threads = map["network_threads"].as<int>();
  parser_threads = map["parser_threads"].as<int>();
  input_url = map["url"].as<string>();
  output_filename = map["output"].as<string>();
  return true;
}
Data* Program::Run()
{
  isRunning = true;
  Data* data = new Data();
  //Добавляем в очередь ссылок начальную ссылку (из аргументов ком строки)
  data->urls.push(input_url);
  data->used.insert(input_url);
  //Указываем текущю глубину (номер обхода)
  data->current_depth = depth;
  //Пока глубина не равна 0, мы вычитаем из нее 1
  while (data->current_depth--)
  {
    std::cout << data->current_depth << ") Loading "
              << data->urls.size() << " urls\n";
    //Запускаем загрузчики
    std::vector<std::thread> workMachines;
    for (int i=0; i != network_threads; i++) {
      workMachines.emplace_back(Program::DownloadFunction, data);
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    //Ждем пока они все закончат
    for (int i=0; i != network_threads; i++)
      if (workMachines[i].joinable())
        workMachines[i].join();
    workMachines.clear();

    std::cout << data->current_depth << ") Parsing "
        << data->htmls.size() << " htmls\n";
    //Запускаем парсеры
    for (int i=0; i != parser_threads; i++) {
      workMachines.emplace_back(Program::ParseFunction, data);
      //Спим по 20 миллисекнуд
      //Нужно, чтобы парсеры не обрабатывали 1 и те же данные
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    //Ждем пока закончат
    for (int i=0; i != parser_threads; i++)
      if (workMachines[i].joinable())
        workMachines[i].join();
    //Если парсер вдруг закрыли, заканчиваем все
    if (!isRunning)
      break;
  }
  //Сохраняем данные в файл
  SaveToFile(data);
  return data;
}
//Функция загрузчика
void Program::DownloadFunction(Data* data) {
  //Пока очередь из ссылок не пустая и прога работает
  while (!data->urls.empty() && isRunning)
  {
    //Мутексом блокируем данные (нужно чтобы программа не падала)
    //ПРО ЭТО МОЖЕТ СПРОСИТЬ
    //Крч штуказапрещает другим потокам работать с этими даннымм пока стоит lock
    data->mut.lock();
    //Достаем элмент из очереди
    string url = data->urls.front();
    //Удаляем его из очереди
    data->urls.pop();
    //Отключаем мутекс
    data->mut.unlock();
    //Запхиваем в очередь скачаных html страниц скачанную html
    data->htmls.push(HTML(url, /*loader::*/getHTML(url)));
  }
}
//Функция парсера
void Program::ParseFunction(Data* data) {
  //Почти тоже самое что выше ток тут все для очереди html
  while (!data->htmls.empty() && isRunning)
  {
    data->mut.lock();
    auto html = data->htmls.front();
    data->htmls.pop();
    data->mut.unlock();
    //Ищем картинки на странице
    parser::searchForImages(html, data);
    //Спим 10 мс на всякий
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}
//Метод сохранения резултата в файл
void Program::SaveToFile(Data* data) {
  std::ofstream file(output_filename);
  file << "Input URL: " << input_url << std::endl
      << "URLs to images: " << std::endl;
  for (auto iu : data->img_urls)
    file << iu << std::endl;
  file.close();
}
//Функция вызввается при нажатии Ctrl+C
void Program::Terminate([[maybe_unused]] int exit_code) {
  isRunning = false;
}
