//
// Created by Yihang Yang on 5/23/19.
//

#ifndef DALI_DALI_CIRCUIT_BLOCK_H_
#define DALI_DALI_CIRCUIT_BLOCK_H_

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "blocktype.h"
#include "dali/common/logging.h"
#include "dali/common/misc.h"
#include "status.h"

/****
 * The class Block provides an abstraction of the physical placement status of an instance
 * a block can be a gate, can also be a large module, it includes information like
 * the Name of a gate/module, its Width and Height, its lower left corner (LLX, LLY),
 * the movability, orientation.
 *
 * LEFDEF manual version 5.8, page 129
 * After placement, a DEF COMPONENTS placement pt indicates where the lower-left corner of the placement bounding rectangle is placed after any possible rotations or flips.
 * The bounding rectangle width and height should be a multiple of the placement grid to allow for abutting cells.
 * ****/

namespace dali {

class BlockAux;

class Block {
  public:
    Block();
    Block(
        BlockType *type_ptr,
        std::pair<const std::string, int> *name_num_pair_ptr,
        int llx,
        int lly,
        bool movable = true,
        BlockOrient orient = N
    );
    Block(
        BlockType *type_ptr,
        std::pair<const std::string, int> *name_num_pair_ptr,
        int llx,
        int lly,
        PlaceStatus place_state = UNPLACED,
        BlockOrient orient = N
    );

    /****member functions for attributes access****/
    // get the Block name
    const std::string &Name() { return name_id_pair_ptr_->first; }

    // get BlockType of this Block
    BlockType *TypePtr() const { return type_ptr_; }

    // get BlockType of this Block
    const std::string &TypeName() const { return type_ptr_->Name(); }

    // get the index of this Block in the vector of instances
    int Index() const { return name_id_pair_ptr_->second; }

    // get the width of this Block
    int Width() const { return type_ptr_->Width(); }

    // set the effective height of this Block, which can be different from the height of its type
    // effective area is also updated at the same time
    void SetHeight(int height);

    // set the Block height to its BlockType height
    // area is also updated at the same time
    void ResetHeight();

    // get the height of this Block
    int Height() const { return eff_height_; }

    // get lower left x coordinate
    double LLX() const { return llx_; }

    // get lower left y coordinate
    double LLY() const { return lly_; }

    // get upper right x coordinate
    double URX() const { return llx_ + Width(); }

    // get upper right y coordinate
    double URY() const { return lly_ + Height(); }

    // get center x coordinate
    double X() const { return llx_ + Width() / 2.0; }

    // get center y coordinate
    double Y() const { return lly_ + Height() / 2.0; }

    // get the indices of nets containing this Block
    std::vector<int> &NetList() { return net_list_; }

    // get the boolean status of whether this Block is placed
    bool IsPlaced() const {
        return place_status_ == PLACED || place_status_ == FIXED
            || place_status_ == COVER;
    }

    // get the placement status of this Block
    PlaceStatus Status() const { return place_status_; }

    // get the string placement status of this Block
    std::string StatusStr() const { return PlaceStatusStr(place_status_); }

    // get the boolean status of whether this Block is movable
    // current assumption: UNPLACED and PLACED are movable, FIXED and COVER are unmovable
    bool IsMovable() const {
        return place_status_ == UNPLACED || place_status_ == PLACED;
    }

    // get the boolean status of whether this Block is unmovable
    bool IsFixed() const { return !IsMovable(); }

    // get the area of this Block
    long int Area() const { return eff_area_; }

    // get the Orientation of this Block
    BlockOrient Orient() const { return orient_; }

    // get the pointer to the auxiliary information
    BlockAux *AuxPtr() const { return aux_ptr_; }

    // set the NameNumPair, first: name, second: number
    void SetNameNumPair(std::pair<const std::string, int> *name_num_pair_ptr) {
        name_id_pair_ptr_ = name_num_pair_ptr;
    }

    // set the BlockType of this Block
    void SetType(BlockType *type_ptr);

    // set the lower left x and y coordinate of this Block
    void SetLoc(double lx, double ly);

    // set the lower left x coordinate
    void SetLLX(double lx) { llx_ = lx; }

    // set the lower left y coordinate
    void SetLLY(double ly) { lly_ = ly; }

    // set the upper right x coordinate
    void SetURX(double ux) { llx_ = ux - Width(); }

    // set the upper right y coordinate
    void SetURY(double uy) { lly_ = uy - Height(); }

    // set the center x coordinate
    void SetCenterX(double center_x) { llx_ = center_x - Width() / 2.0; }

    // set the center y coordinate
    void SetCenterY(double center_y) { lly_ = center_y - Height() / 2.0; }

    // set the placement status of this Block
    void SetPlacementStatus(PlaceStatus place_status);

    // set the orientation of this Block
    void SetOrient(BlockOrient orient);

    // set the pointer to the auxiliary information
    void SetAux(BlockAux *aux);

    // swap the location of this Block with the Block @param blk
    // this member function ONLY swaps location
    void SwapLoc(Block &blk);

    // increase x coordinate by a certain amount
    void IncreaseX(double displacement) { llx_ += displacement; }

    // increase y coordinate by a certain amount
    void IncreaseY(double displacement) { lly_ += displacement; }

    // increase x coordinate by a certain amount, but the final location is bounded by @param lower, and upper
    void IncreaseX(double displacement, double upper, double lower);

    // increase y coordinate by a certain amount, but the final location is bounded by @param lower, and upper
    void IncreaseY(double displacement, double upper, double lower);

    // decrease x coordinate by a certain amount
    void DecreaseX(double displacement) { llx_ -= displacement; }

    // decrease y coordinate by a certain amount
    void DecreaseY(double displacement) { lly_ -= displacement; }

    // returns whether this Block overlaps with BLock @param blk
    bool IsOverlap(const Block &blk) const {
        return !(LLX() > blk.URX() || blk.LLX() > URX() || LLY() > blk.URY()
            || blk.LLY() > URY());
    }

    // returns whether this Block overlaps with BLock @param blk
    bool IsOverlap(const Block *blk) const { return IsOverlap(*blk); }

    // returns the overlap area between this Block and Block @param blk
    double OverlapArea(const Block &blk) const;

    /****Report info of this Block, for debugging purposes****/
    void Report();
    void ReportNet();

  protected:
    BlockType *type_ptr_; // type
    // cached height, also used to store effective height, the unit is grid value in the y-direction
    int eff_height_;
    long int eff_area_; // cached effective area
    // name for finding its index in block_list
    std::pair<const std::string, int> *name_id_pair_ptr_;
    double llx_; // lower x coordinate, data type double, for global placement
    double lly_; // lower y coordinate
    std::vector<int> net_list_; // the list of nets connected to this cell
    PlaceStatus place_status_; // placement status, i.e, PLACED, FIXED, UNPLACED
    BlockOrient orient_; // orientation, normally, N or FS
    BlockAux *aux_ptr_; // points to auxiliary information if needed
};

class BlockAux {
  protected:
    Block *blk_ptr_;
  public:
    explicit BlockAux(Block *blk_ptr) : blk_ptr_(blk_ptr) {
        blk_ptr->SetAux(this);
    }

    // get the pointer pointing to the Block it belongs to
    Block *getBlockPtr() const { return blk_ptr_; }
};

}

#endif //DALI_DALI_CIRCUIT_BLOCK_H_
