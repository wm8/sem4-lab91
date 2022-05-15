// Copyright 2022 vlados2003
#ifndef TEMPLATE_PARSER_H
#define TEMPLATE_PARSER_H
#include "Data.h"
#include "Uri.h"
#include <gumbo.h>
namespace parser
{
//Метод поиска изображений
//Говоришь что честно не совсем понимаешь как это все раюотает, в документации
//нашла этот кусок кода, и исправила то что будет закоменчено
  void searchForImages(HTML& html, Data* data) {
    try {
      GumboOutput* output = gumbo_parse(html.data.c_str());

      std::queue<GumboNode*> qn;
      qn.push(output->root);

      while (!qn.empty()) {
        GumboNode* node = qn.front();
        qn.pop();
        if (GUMBO_NODE_ELEMENT == node->type) {
          GumboAttribute* attr = nullptr;
          //Если таг html элемента href - то это ссылка
          if (node->v.element.tag == GUMBO_TAG_A &&
              (attr = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
            std::string s = Uri::getFullUrl(html.url, attr->value);
            //Мутексом блок
            data->mut.lock();
            //Ищем, не скачивали ли мы уже эту страницу
            if (data->used.find(s) == data->used.end()) {
              //Если нет, в массив ссылок добавляем
              data->urls.push(s);
              data->used.insert(s);
            }
            //Мутекс отключаем
            data->mut.unlock();
            //Иначе если таг html элемента src - то это картинка, все то же самое что и выше
          } else if (node->v.element.tag == GUMBO_TAG_IMG &&
                     (attr = gumbo_get_attribute(&node->v.element.attributes,
                                                 "src"))) {
            auto s = Uri::getFullUrl(html.url, attr->value);
            data->mut.lock();
            data->img_urls.emplace_back(s);
            data->dbgi.emplace_back(DebugData(data->current_depth, s));
            data->mut.unlock();
          }
          GumboVector* children = &node->v.element.children;
          for (unsigned int i = 0; i < children->length; ++i) {
            qn.push(static_cast<GumboNode*>(children->data[i]));
          }
        }
      }
      gumbo_destroy_output(&kGumboDefaultOptions, output);
    }catch (...) {}
  }
}
#endif  // TEMPLATE_PARSER_H
