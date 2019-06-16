//
// Created by Yihang Yang on 2019-06-15.
//

#ifndef HPCC_NETAL_HPP
#define HPCC_NETAL_HPP

#include "circuit/circuitnet.hpp"

class net_al_t: public net_t {
public:
  net_al_t();
  explicit net_al_t(std::string &name_arg, double weight_arg = 1);
  void retrieve_info_from_database(net_t &net);

  double dhpwlx();
  double dhpwly();
  size_t max_pin_index_x();
  size_t min_pin_index_x();
  size_t max_pin_index_y();
  size_t min_pin_index_y();
};


#endif //HPCC_NETAL_HPP
