//
// Created by Yihang Yang on 3/14/20.
//

#ifndef DALI_SRC_PLACER_WELLLEGALIZER_STDCLUSTERWELLLEGALIZER_H_
#define DALI_SRC_PLACER_WELLLEGALIZER_STDCLUSTERWELLLEGALIZER_H_

#include "block_cluster.h"
#include "circuit/block.h"
#include "common/misc.h"
#include "placer/legalizer/LGTetrisEx.h"
#include "placer/placer.h"

struct Cluster {
  bool is_orient_N_ = true; // orientation of this cluster
  std::vector<Block *> blk_list_; // list of blocks in this cluster

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

  /**** member functions ****/
  int UsedSize() const;
  void SetUsedSize(int used_size);
  void UseSpace(int width);

  void SetLLX(int lx);
  void SetURX(int ux);
  int LLX() const;
  int URX() const;
  double CenterX() const;

  void SetWidth(int width);
  int Width() const;

  void SetLLY(int ly);
  void SetURY(int uy);
  int LLY() const;
  int URY() const;
  double CenterY() const;

  void SetHeight(int height);
  void UpdateWellHeightFromBottom(int p_well_height, int n_well_height);
  void UpdateWellHeightFromTop(int p_well_height, int n_well_height);
  int Height() const;
  int PHeight() const;
  int NHeight() const;
  int PNEdge() const;

  void SetLoc(int lx, int ly);

  void ShiftBlockX(int x_disp);
  void ShiftBlockY(int y_disp);
  void ShiftBlock(int x_disp, int y_disp);
  void UpdateBlockLocY();
  void LegalizeCompactX(int left);
  void LegalizeCompactX();
  void LegalizeLooseX();
  void SetOrient(bool is_orient_N);
  void InsertWellTapCell(Block &tap_cell, int loc);

  void UpdateBlockLocationCompact();
};

struct ClusterStrip {
  int lx_;
  int width_;
  int max_blk_capacity_per_cluster_;

  int contour_;
  int used_height_;
  int cluster_count_;
  Cluster *front_cluster_;
  std::vector<Cluster> cluster_list_;
  bool is_bottom_up_ = false;

  int block_count_;
  std::vector<Block *> block_list_;

  bool is_first_row_orient_N_ = true;
  std::vector<RectI> well_rect_list_;

  int Width() const;
  int LLX() const;
  int URX() const;
};

class StdClusterWellLegalizer : public Placer {
 private:
  bool is_first_row_orient_N_ = true;

  /**** well parameters ****/
  int max_unplug_length_;
  int well_tap_cell_width_;
  int well_spacing_;

  /**** strip parameters ****/
  int strip_width_;
  int tot_strip_num_;

  /**** well tap cell parameters ****/
  BlockType *well_tap_cell_;
  int tap_cell_p_height_;
  int tap_cell_n_height_;

  std::vector<IndexLocPair<int>> index_loc_list_; // list of index loc pair for location sort
  std::vector<ClusterStrip> strip_list_; // list of strips

  /**** row information ****/
  int row_height_;
  int tot_num_rows_;
  std::vector<std::vector<SegI>> white_space_in_rows_; // white space in each row

  /****parameters for legalization****/
  int max_iter_ = 5;

 public:
  StdClusterWellLegalizer();

  int StartRow(int y_loc);
  int EndRow(int y_loc);

  void Init(int cluster_width = 0);

  void SetFirstRowOrientN(bool is_N);

  int LocToCol(int x);
  void AssignBlockToStrip();

  void AppendBlockToColBottomUp(ClusterStrip &col, Block &blk);
  void AppendBlockToColTopDown(ClusterStrip &col, Block &blk);
  void AppendBlockToColBottomUpCompact(ClusterStrip &col, Block &blk);
  void AppendBlockToColTopDownCompact(ClusterStrip &col, Block &blk);

  bool StripLegalizationBottomUp(ClusterStrip &col);
  bool StripLegalizationTopDown(ClusterStrip &col);
  bool StripLegalizationBottomUpCompact(ClusterStrip &col);
  bool StripLegalizationTopDownCompact(ClusterStrip &col);

  bool BlockClustering();
  bool BlockClusteringLoose();
  bool BlockClusteringCompact();

  bool TrialClusterLegalization();
  bool TetrisLegalizeCluster();

  double WireLengthCost(Cluster *cluster, int l, int r);
  void FindBestLocalOrder(std::vector<Block *> &res,
                          double &cost,
                          Cluster *cluster,
                          int cur,
                          int l,
                          int r,
                          int left_bound,
                          int right_bound,
                          int gap,
                          int range);
  void LocalReorderInCluster(Cluster *cluster, int range = 3);
  void LocalReorderAllClusters();

  void SingleSegmentClusteringOptimization();

  void UpdateClusterOrient();
  void InsertWellTap();

  void ClearCachedData();
  bool WellLegalize();

  bool StartPlacement() override;

  void GenMatlabClusterTable(std::string const &name_of_file);
  void GenMATLABWellTable(std::string const &name_of_file) override;
  void EmitDEFWellFile(std::string const &name_of_file, std::string const &input_def_file) override;

  /**** member functions for debugging ****/
  void GenAvailSpace(std::string const &name_of_file = "avail_space.txt");
};

inline int Cluster::UsedSize() const {
  return used_size_;
}

inline void Cluster::SetUsedSize(int used_size) {
  used_size_ = used_size;
}

inline void Cluster::UseSpace(int width) {
  used_size_ += width;
}

inline void Cluster::SetLLX(int lx) {
  lx_ = lx;
}

inline void Cluster::SetURX(int ux) {
  lx_ = ux - width_;
}

inline int Cluster::LLX() const {
  return lx_;
}

inline int Cluster::URX() const {
  return lx_ + width_;
}

inline double Cluster::CenterX() const {
  return lx_ + width_ / 2.0;
}

inline void Cluster::SetWidth(int width) {
  width_ = width;
}

inline int Cluster::Width() const {
  return width_;
}

inline void Cluster::SetLLY(int ly) {
  ly_ = ly;
}

inline void Cluster::SetURY(int uy) {
  ly_ = uy - height_;
}

inline int Cluster::LLY() const {
  return ly_;
}

inline int Cluster::URY() const {
  return ly_ + height_;
}

inline double Cluster::CenterY() const {
  return ly_ + height_ / 2.0;
}

inline void Cluster::SetHeight(int height) {
  height_ = height;
}

inline void Cluster::UpdateWellHeightFromBottom(int p_well_height, int n_well_height) {
  /****
   * Update the height of this cluster with the lower y of this cluster fixed.
   * So even if the height changes, the lower y of this cluster does not need be changed.
   * ****/
  p_well_height_ = std::max(p_well_height_, p_well_height);
  n_well_height_ = std::max(n_well_height_, n_well_height);
  height_ = p_well_height_ + n_well_height_;
}

inline void Cluster::UpdateWellHeightFromTop(int p_well_height, int n_well_height) {
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

inline int Cluster::Height() const {
  return height_;
}

inline int Cluster::PHeight() const {
  return p_well_height_;
}

inline int Cluster::NHeight() const {
  return n_well_height_;
}

inline int Cluster::PNEdge() const {
  /****
   * Returns the P/N well edge to the bottom of this cluster
   * ****/
  return is_orient_N_ ? PHeight() : NHeight();
}

inline void Cluster::SetLoc(int lx, int ly) {
  lx_ = lx;
  ly_ = ly;
}

inline int ClusterStrip::Width() const {
  return width_;
}

inline int ClusterStrip::LLX() const {
  return lx_;
}

inline int ClusterStrip::URX() const {
  return lx_ + width_;
}

inline int StdClusterWellLegalizer::StartRow(int y_loc) {
  return (y_loc - bottom_) / row_height_;
}

inline int StdClusterWellLegalizer::EndRow(int y_loc) {
  int relative_y = y_loc - bottom_;
  int res = relative_y / row_height_;
  if (relative_y % row_height_ == 0) {
    --res;
  }
  return res;
}

inline void StdClusterWellLegalizer::SetFirstRowOrientN(bool is_N) {
  is_first_row_orient_N_ = is_N;
}

inline int StdClusterWellLegalizer::LocToCol(int x) {
  int col_num = (x - RegionLeft()) / strip_width_;
  if (col_num < 0) {
    col_num = 0;
  }
  if (col_num >= tot_strip_num_) {
    col_num = tot_strip_num_ - 1;
  }
  return col_num;
}

#endif //DALI_SRC_PLACER_WELLLEGALIZER_STDCLUSTERWELLLEGALIZER_H_