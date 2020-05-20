//
// Created by Yihang Yang on 2019-05-23.
//

#ifndef DALI_NET_HPP
#define DALI_NET_HPP

#include <string>
#include <vector>

#include "block.h"
#include "blockpinpair.h"
#include "common/misc.h"

class NetAux;

class Net {
 protected:
  std::pair<const std::string, int> *pname_num_pair_;
  double weight_;
  int cnt_fixed_;

  // cached data
  int max_pin_x_, min_pin_x_;
  int max_pin_y_, min_pin_y_;
  double inv_p_; // 1.0/(p-1), where p is the number of pins connected by this net

  // auxiliary information
  NetAux *paux_;
 public:
  std::vector<BlockPinPair> blk_pin_list;

  Net(std::pair<const std::string, int> *name_num_pair_ptr, int capacity, double weight);

  // API to add block/pin pair
  void AddBlockPinPair(Block *block_ptr, Pin *pin);

  const std::string *Name() const { return &(pname_num_pair_->first); }
  std::string NameStr() const { return pname_num_pair_->first; }
  int Num() const { return pname_num_pair_->second; }
  void SetWeight(double weight) { weight_ = weight; }
  double Weight() const { return weight_; }
  double InvP() const { return inv_p_; }
  int P() const { return (int) blk_pin_list.size(); }
  int FixedCnt() const { return cnt_fixed_; }
  void SetAux(NetAux *aux) {
    Assert(aux != nullptr, "Cannot set @param aux to nullptr in void Net::SetAux()\n");
    paux_ = aux;
  }
  NetAux *Aux() const { return paux_; }

  void XBoundExclude(Block *blk_ptr, double &lo, double &hi);
  void XBoundExclude(Block &blk_ptr, double &lo, double &hi) {
    XBoundExclude(&blk_ptr, lo, hi);
  }

  void YBoundExclude(Block *blk_ptr, double &lo, double &hi);
  void YBoundExclude(Block &blk_ptr, double &lo, double &hi) {
    YBoundExclude(&blk_ptr, lo, hi);
  }

  void SortBlkPinList();
  void UpdateMaxMinIndexX();
  void UpdateMaxMinIndexY();
  void UpdateMaxMinIndex() {
    UpdateMaxMinIndexX();
    UpdateMaxMinIndexY();
  }

  int MaxBlkPinNumX() const { return max_pin_x_; }
  int MinBlkPinNumX() const { return min_pin_x_; }
  int MaxBlkPinNumY() const { return max_pin_y_; }
  int MinBlkPinNumY() const { return min_pin_y_; }
  Block *MaxBlockX() const { return blk_pin_list[max_pin_x_].GetBlock(); }
  Block *MinBlockX() const { return blk_pin_list[min_pin_x_].GetBlock(); }
  Block *MaxBlockY() const { return blk_pin_list[max_pin_y_].GetBlock(); }
  Block *MinBlockY() const { return blk_pin_list[min_pin_y_].GetBlock(); }
  double HPWLX();
  double HPWLY();
  double HPWL() { return HPWLX() + HPWLY(); }

  double MinX() const { return blk_pin_list[min_pin_x_].AbsX(); }
  double MaxX() const { return blk_pin_list[max_pin_x_].AbsX(); }
  double MinY() const { return blk_pin_list[min_pin_y_].AbsY(); }
  double MaxY() const { return blk_pin_list[max_pin_y_].AbsY(); }

  void UpdateMaxMinCtoCX();
  void UpdateMaxMinCtoCY();
  void UpdateMaxMinCtoC();
  int MaxPinCtoCX();
  int MinPinCtoCX();
  int MaxPinCtoCY();
  int MinPinCtoCY();
  double HPWLCtoCX();
  double HPWLCtoCY();
  double HPWLCtoC() { return HPWLCtoCX() + HPWLCtoCY(); }
};

class NetAux {
 protected:
  Net *net_;
 public:
  explicit NetAux(Net *net) : net_(net) { net_->SetAux(this); }
  Net *GetNet() const { return net_; }
};

#endif //DALI_NET_HPP
