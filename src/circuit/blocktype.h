//
// Created by Yihang Yang on 2019-06-27.
//

#ifndef HPCC_BLOCKTYPE_HPP
#define HPCC_BLOCKTYPE_HPP

#include <string>
#include <map>
#include "pin.h"
#include "common/misc.h"

#define STR_INT_PAIR_PTR std::pair<std::string, int>*

class BlockType {
private:
  /****essential data entries****/
  STR_INT_PAIR_PTR name_num_pair_ptr_;
  int width_, height_;
public:
  BlockType(STR_INT_PAIR_PTR name_num_pair_ptr, int width, int height);

  std::vector<Pin> pin_list;
  std::map<std::string, int> pin_name_num_map;

  const std::string *Name() const;
  int Num() const;
  int Width() const;
  int Height() const;
  int Area() const;
  void SetWidth(int width);
  void SetHeight(int height);

  bool PinExist(std::string &pin_name);
  int PinIndex(std::string &pin_name);
  Pin *AddPin(std::string &pin_name);

  friend std::ostream& operator<<(std::ostream& os, const BlockType &block_type) {
    os << "block type Name: " << *block_type.Name() << "\n";
    os << "Width and Height: " << block_type.Width() << " " << block_type.Height() << "\n";
    os << "assigned primary key: " << block_type.Num() << "\n";
    os << "pin list:\n";
    for( auto &&it: block_type.pin_name_num_map) {
      os << "  " << it.first << " " << it.second << "  " << block_type.pin_list[it.second].XOffset() << " " << block_type.pin_list[it.second].YOffset() << "\n";
    }
    return os;
  }
};


#endif //HPCC_BLOCKTYPE_HPP