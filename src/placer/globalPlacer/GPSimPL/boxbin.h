//
// Created by Yihang Yang on 2019-08-07.
//

#ifndef HPCC_SRC_PLACER_GLOBALPLACER_GPSIMPL_BOXBIN_H_
#define HPCC_SRC_PLACER_GLOBALPLACER_GPSIMPL_BOXBIN_H_

#include <iostream>
#include <fstream>
#include <vector>
#include "gridbinindex.h"
#include "cellcutpoint.h"
#include "gridbin.h"
#include "circuit/block.h"

class BoxBin {
 public:
  BoxBin();
  bool cut_direction_x;
  // cut line is alone x direction
  bool all_terminal;
  int total_white_space;
  float filling_rate;
  bool is_all_terminal() { return all_terminal; };
  /* Cut-line to split box white space */
  GridBinIndex ll_index;
  GridBinIndex ur_index;
  GridBinIndex cut_ll_index;
  GridBinIndex cut_ur_index;
  /* Cut-line to split cell area */
  CellCutPoint ll_point;
  CellCutPoint ur_point;
  CellCutPoint cut_ll_point;
  CellCutPoint cut_ur_point;
  /* total cell area, and the value in two children box */
  int total_cell_area;
  int total_cell_area_low;
  int total_cell_area_high;

  /* all cell_id in the box, and cell_id in two children box */
  std::vector< int > cell_list;
  std::vector< int > cell_list_low;
  std::vector< int > cell_list_high;

  /* the cell_id for terminals in the box, will be updated only when the box is a GridBin
   * if there is no terminal in the grid bin, do not have to further split the box into smaller boxs,
   * otherwise split the box into smaller boxes, until there is no terminals in any boxes*/
  std::vector< int > terminal_list;
  void update_terminal_list(std::vector<std::vector<GridBin> > &grid_bin_matrix) {terminal_list = grid_bin_matrix[ll_index.x][ll_index.y].terminal_list;};
  /* update_terminal_list can only be called when the box is a grid_bin_box */
  bool has_terminal() { return !terminal_list.empty(); };

  std::vector< int > vertical_obstacle_boundaries;
  std::vector< int > horizontal_obstacle_boundaries;
  void update_ob_boundaries(std::vector<Block> &block_list);

  /* the boundary of box if the box is smaller than a grid bin, the following for attribute will be very important
   * the left, right, bottom, top boundaries are the region where cells will be placed in */
  int left;
  int right;
  int bottom;
  int top;
  void update_boundaries(std::vector<std::vector<GridBin> > &grid_bin_matrix);
  /* update_white_space can only be called when left, right, bottom, top are updated */
  void update_terminal_list_white_space(std::vector<Block> &Nodelist, std::vector< int > &box_terminal_list);

  void update_all_terminal(std::vector< std::vector< GridBin > > &grid_bin_matrix);
  void update_cell_area(std::vector<Block> &Nodelist);
  void update_cell_area_white_space(std::vector< std::vector< GridBin > > &grid_bin_matrix);
  void update_cell_area_white_space_LUT(std::vector< std::vector< int > > &grid_bin_white_space_LUT, std::vector<std::vector<GridBin> > &grid_bin_matrix);
  void expand_box(int GRID_NUM);
  bool write_box_boundary(std::string const &NameOfFile);
  bool write_cell_region(std::string const &NameOfFile="first_cell_bounding_box.txt");
  double white_space_LUT(std::vector< std::vector< int > > &grid_bin_white_space_LUT, GridBinIndex &ll, GridBinIndex &ur);
  void update_cell_in_box(std::vector<std::vector<GridBin> > &grid_bin_matrix);
  bool write_cell_in_box(std::string const &NameOfFile, std::vector<Block> &Nodelist);
  bool update_cut_index_white_space(std::vector< std::vector< int > > &grid_bin_white_space_LUT);
  bool update_cut_point_cell_list_low_high(std::vector<Block> &Nodelist, int &box1_total_white_space, int &box2_total_white_space);
  bool update_cut_point_cell_list_low_high_leaf(std::vector<Block> &Nodelist, int &cut_line_w, int std_cell_height);
};

#endif //HPCC_SRC_PLACER_GLOBALPLACER_GPSIMPL_BOXBIN_H_