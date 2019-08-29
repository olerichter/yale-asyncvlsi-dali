//
// Created by Yihang Yang on 2019-07-25.
//

#ifndef HPCC_SRC_COMMON_MISC_H_
#define HPCC_SRC_COMMON_MISC_H_

#include <cassert>
#include <iostream>
#include <string>

void Assert(bool e, const std::string &error_message);
void Warning(bool e, const std::string &warning_message);
double Random();

#endif //HPCC_SRC_COMMON_MISC_H_
