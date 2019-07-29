//
// Created by Yihang Yang on 2019-05-23.
//

#ifndef HPCC_PLACERBASE_HPP
#define HPCC_PLACERBASE_HPP

#include <string>
#include <iostream>
#include <fstream>
#include "circuit/circuit.h"

class placer_t {
protected:
  /* essential data entries */
  double _aspect_ratio; // placement region Height/Width
  double _filling_rate;

  /* the following entries are derived data
   * note that the following entries can be manually changed
   * if so, the _aspect_ratio or _filling_rate might also be changed */
  int _left, _right, _bottom, _top;
  // boundaries of the placement region
  circuit_t* _circuit;

public:
  placer_t();
  placer_t(double aspectRatio, double fillingRate);

  bool set_filling_rate(double rate = 2.0/3.0);
  double filling_rate();
  bool set_aspect_ratio(double ratio = 1.0);
  double aspect_ratio();

  double space_block_ratio();
  bool set_space_block_ratio(double ratio);
  // the ratio of total_white_space/total_block_area

  virtual bool set_input_circuit(circuit_t *circuit) = 0;
  bool auto_set_boundaries();
  void report_boundaries();
  int left();
  int right();
  int bottom();
  int top();
  bool update_aspect_ratio();
  bool set_boundary(int left_arg=0, int right_arg=0, int bottom_arg=0, int top_arg=0);

  virtual bool start_placement() = 0;
  virtual void report_placement_result() = 0;

  bool gen_matlab_disp_file(std::string const &filename="block_net_list.m");

  bool write_pl_solution(std::string const &NameOfFile);
  bool write_node_terminal(std::string const &NameOfFile="terminal.txt", std::string const &NameOfFile1="nodes.txt");
  bool save_DEF(std::string const &NameOfFile="circuit.def");

  virtual ~placer_t();
};


#endif //HPCC_PLACERBASE_HPP