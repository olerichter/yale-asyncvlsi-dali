//
// Created by Yihang Yang on 1/2/20.
//

#ifndef DALI_SRC_PLACER_LEGALIZER_LGTETRISEX_H_
#define DALI_SRC_PLACER_LEGALIZER_LGTETRISEX_H_

#include "placer/placer.h"

class LGTetrisEx : public Placer {
 private:
  std::vector<std::vector<int>> white_space_in_rows_;
  std::vector<int> block_contour_;
  std::vector<IndexLocPair<int>> index_loc_list_;

  int row_height_;

  bool legalize_from_left_;

  int cur_iter_;
  int max_iter_;

  double k_width_;
  double k_height_;
  double k_left_;

  //cached data
  int tot_num_rows_;

 public:
  LGTetrisEx();

  static void MergeIntervals(std::vector<std::vector<int>> &intervals);
  void InitLegalizer(int row_height = 1);

  int StartRow(int y_loc);
  int EndRow(int y_loc);

  void UseSpace(Block const &block);
  bool IsCurrentLocLegal(Value2D<int> &loc, int width, int height);
  bool FindLoc(Value2D<int> &loc, int width, int height);
  void FastShift(int failure_point);
  bool LocalLegalization();

  void UseSpaceRight(Block const &block);
  bool IsCurrentLocLegalRight(Value2D<int> &loc, int width, int height);
  bool FindLocRight(Value2D<int> &loc, int width, int height);
  void FastShiftRight(int failure_point);
  bool LocalLegalizationRight();

  double EstimatedHPWL(Block &block, int x, int y);

  void StartPlacement() override;

  void SetMaxIteration(int max_iter);
  void SetWidthHeightFactor(double k_width, double k_height);
  void SetLeftBoundFactor(double k_left);

  void GenAvailSpace(std::string const &name_of_file = "avail_space.txt");
};

inline int LGTetrisEx::StartRow(int y_loc) {
  int res = (y_loc - bottom_) / row_height_;
  return res;
}

inline int LGTetrisEx::EndRow(int y_loc) {
  int relative_y = y_loc - bottom_;
  int res = relative_y / row_height_;
  if (relative_y % row_height_ == 0) {
    --res;
  }
  return res;
}

inline void LGTetrisEx::SetMaxIteration(int max_iter) {
  assert(max_iter >= 0);
  max_iter_ = max_iter;
}

inline void LGTetrisEx::SetWidthHeightFactor(double k_width, double k_height) {
  k_width_ = k_width;
  k_height_ = k_height;
}

inline void LGTetrisEx::SetLeftBoundFactor(double k_left) {
  k_left_ = k_left;
}

#endif //DALI_SRC_PLACER_LEGALIZER_LGTETRISEX_H_
