//
// Created by Yihang Yang on 2019-03-26.
//

#ifndef HPCC_CIRCUIT_HPP
#define HPCC_CIRCUIT_HPP

#include <vector>
#include <map>
#include "circuitblocktype.hpp"
#include "circuitblock.hpp"
#include "circuitpin.hpp"
#include "circuitnet.hpp"

class circuit_t {
private:
  int dummy_space = 2;
protected:
  double _ave_width;
  double _ave_height;
  int _tot_block_area;
  int _tot_movable_num;
  // these data entries are all cached data
public:
  circuit_t();
  /* essential data entries */
  std::vector< block_t > block_list;
  std::vector< net_t > net_list;
  // node_list and net_list contains all the information of a circuit graph

  int HPWL;
  // HPWL of this circuit
  std::map<std::string, size_t> block_name_map;
  // string to size_t map to find the index of a block in the block_list
  std::map<std::string, size_t> net_name_map;
  // string to size_t map to find the index of a net in the net_list

  bool add_new_block(std::string &blockName, int w, int h, int llx = 0, int lly = 0, bool movable = true, std::string typeName="");
  bool create_blank_net(std::string &netName, double weight = 1);
  bool add_pin_to_net(const std::string &netName, const std::string &blockName, int xOffset, int yOffset, std::string pinName="");
  // the above three member functions should be called to add elements to block_list or net_list

  void parse_line(std::string &line, std::vector<std::string> &field_list);
  bool read_nodes_file(std::string const &NameOfFile);
  void report_block_list();
  void report_block_map();
  bool read_nets_file(std::string const &NameOfFile);
  void report_net_list();
  void report_net_map();
  bool read_pl_file(std::string const &NameOfFile);
  // the above member functions should be called when input comes from files

  int lef_database_microns = 0;
  double m2_pitch = 0;
  int def_distance_microns = 0;
  int def_left = 0, def_right = 0, def_bottom = 0, def_top = 0;
  std::vector< block_type_t > blockType_list;
  std::map<std::string, size_t> blockType_name_map;
  bool read_lef_file(std::string const &NameOfFile);
  void report_blockType_list();
  void report_blockType_map();
  bool read_def_file(std::string const &NameOfFile);

  //-----------------------------------------------------------------------------------------------
  /* the following member function calculate corresponding values in real time, the running time is O(n) */
  double ave_width_real_time();
  double ave_height_real_time();
  double ave_block_area_real_time();
  int tot_block_area_real_time();
  int tot_movable_num_real_time();
  int tot_unmovable_num_real_time();

  /* these following member functions just return cached data entries O(1)
   * or if the cached data entries have not been initialized, the corresponding real_time function will be called O(n) */
  double ave_width();
  double ave_height();
  double ave_block_area();
  int tot_block_area();
  int tot_movable_num();
  int tot_unmovable_num();

  bool write_nodes_file(std::string const &NameOfFile="circuit.nodes");
  bool write_nets_file(std::string const &NameOfFile="circuit.nets");
  bool gen_matlab_disp_file(std::string const &filename="block_net_list.m");
  bool save_DEF(std::string const &NameOfFile, std::string const &defFileName);
};

#endif //HPCC_CIRCUIT_HPP
