//
// Created by Yihang Yang on 2019-06-27.
//

#include "circuitblocktype.hpp"

block_type_t::block_type_t() {
  _name = "";
  _width = 0;
  _height = 0;
  _num = 0;
}

block_type_t::block_type_t(std::string &init_name, int init_width, int init_height) : _name(std::move(init_name)), _width(init_width), _height(init_height) {
  _num = 0;
}

void block_type_t::set_name(std::string &typeName) {
  _name = typeName;
}

std::string block_type_t::name() {
  return _name;
}

void block_type_t::set_width(int w) {
  _width = w;
}

int block_type_t::width() {
  return _width;
}

void block_type_t::set_height(int h) {
  _height = h;
}

int block_type_t::height() {
  return _height;
}

void block_type_t::set_num(size_t Num) {
  _num = (int) Num;
}

int block_type_t::num() {
  return  _num;
}

bool block_type_t::add_pin(std::string &pinName, double xOffset, double yOffset) {
  if (pinname_num_map.find(pinName) == pinname_num_map.end()) {
    pinname_num_map.insert(std::pair<std::string, size_t>(pinName, pin_list.size()));
    pin_list.emplace_back(xOffset, yOffset);
    return true;
  } else {
    std::cout << "Error!\n";
    std::cout << "Existing pin in pin_list with name: " << pinName << "\n";
    return false;
  }
}
