/*******************************************************************************
 *
 * Copyright (c) 2021 Yihang Yang
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 ******************************************************************************/

#ifndef DALI_DALI_PLACER_WELLLEGALIZER_CLUSTER_H_
#define DALI_DALI_PLACER_WELLLEGALIZER_CLUSTER_H_

#include "blocksegment.h"
#include "dali/circuit/block.h"

namespace dali {

struct Cluster {
  bool is_orient_N_ = true; // orientation of this cluster
  std::vector<Block *> blk_list_; // list of blocks in this cluster
  std::vector<double2d> blk_initial_location_;

  /**** number of tap cells needed, and pointers to tap cells ****/
  int tap_cell_num_ = 0;
  Block *tap_cell_;

  /**** x/y coordinates and dimension ****/
  int lx_;
  int ly_;
  int width_;
  int height_;

  /**** total width of cells in this cluster, including reserved space for tap cells ****/
  int used_size_;
  int usable_width_; // to ensure a proper well tap cell location can be found

  /**** maximum p-well height and n-well height ****/
  int p_well_height_ = 0;
  int n_well_height_ = 0;

  /**** lly which gives minimal displacement ****/
  double min_displacement_lly_ = DBL_MIN;

  /**** member functions ****/
  int UsedSize() const { return used_size_; }
  void SetUsedSize(int used_size) { used_size_ = used_size; }
  void UseSpace(int width) { used_size_ += width; }

  void SetLLX(int lx) { lx_ = lx; }
  void SetURX(int ux) { lx_ = ux - width_; }
  int LLX() const { return lx_; }
  int URX() const { return lx_ + width_; }
  double CenterX() const { return lx_ + width_ / 2.0; }

  void SetWidth(int width) { width_ = width; }
  int Width() const { return width_; }

  void SetLLY(int ly) { ly_ = ly; }
  void SetURY(int uy) { ly_ = uy - height_; }
  int LLY() const { return ly_; }
  int URY() const { return ly_ + height_; }
  double CenterY() const { return ly_ + height_ / 2.0; }

  void SetHeight(int height) { height_ = height; }
  void UpdateWellHeightFromBottom(int p_well_height, int n_well_height) {
    /****
     * Update the height of this cluster with the lower y of this cluster fixed.
     * So even if the height changes, the lower y of this cluster does not need be changed.
     * ****/
    p_well_height_ = std::max(p_well_height_, p_well_height);
    n_well_height_ = std::max(n_well_height_, n_well_height);
    height_ = p_well_height_ + n_well_height_;
  }
  void UpdateWellHeightFromTop(int p_well_height, int n_well_height) {
    /****
     * Update the height of this cluster with the upper y of this cluster fixed.
     * So if the height changes, then the lower y of this cluster should also be changed.
     * ****/
    int old_height = height_;
    p_well_height_ = std::max(p_well_height_, p_well_height);
    n_well_height_ = std::max(n_well_height_, n_well_height);
    height_ = p_well_height_ + n_well_height_;
    ly_ -= (height_ - old_height);
  }
  int Height() const { return height_; }
  int PHeight() const { return p_well_height_; }
  int NHeight() const { return n_well_height_; }
  int PNEdge() const {
    /****
     * Returns the P/N well edge to the bottom of this cluster
     * ****/
    return is_orient_N_ ? PHeight() : NHeight();
  }

  void SetLoc(int lx, int ly) {
    lx_ = lx;
    ly_ = ly;
  }

  void AddBlock(Block *blk_ptr) {
    blk_list_.push_back(blk_ptr);
    blk_initial_location_.emplace_back(blk_ptr->LLX(),
                                       blk_ptr->LLY()
                                           + blk_ptr->TypePtr()->WellPtr()->Pheight());
  }

  void ShiftBlockX(int x_disp);
  void ShiftBlockY(int y_disp);
  void ShiftBlock(int x_disp, int y_disp);
  void UpdateBlockLocY();
  void LegalizeCompactX(int left);
  void LegalizeCompactX();
  void LegalizeLooseX(int space_to_well_tap = 0);
  void SetOrient(bool is_orient_N);
  void InsertWellTapCell(Block &tap_cell, int loc);

  void UpdateBlockLocationCompact();

  void MinDisplacementLegalization();
  void UpdateMinDisplacementLLY();
  double MinDisplacementLLY() const;
};

struct ClusterSegment {
 private:
  int ly_;
  int height_;
 public:
  ClusterSegment(Cluster *cluster_ptr, int loc)
      : ly_(loc), height_(cluster_ptr->Height()) {
    cluster_list.push_back(cluster_ptr);
  }
  std::vector<Cluster *> cluster_list;

  int LY() const { return ly_; }
  int UY() const { return ly_ + height_; }
  int Height() const { return height_; }

  bool IsNotOnBottom(ClusterSegment &sc) const {
    return sc.LY() < UY();
  }
  void Merge(ClusterSegment &sc, int lower_bound, int upper_bound);
  void UpdateClusterLocation();
};

}

#endif //DALI_DALI_PLACER_WELLLEGALIZER_CLUSTER_H_