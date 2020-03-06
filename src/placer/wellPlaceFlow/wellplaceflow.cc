//
// Created by Yihang Yang on 3/5/20.
//

#include "wellplaceflow.h"

#include <placer/placer.h>

WellPlaceFlow::WellPlaceFlow() : GPSimPL() {}

void WellPlaceFlow::StartPlacement() {
  double wall_time = get_wall_time();
  double cpu_time = get_cpu_time();
  if (globalVerboseLevel >= LOG_CRITICAL) {
    std::cout << "---------------------------------------\n"
              << "Start global placement\n";
  }
  SanityCheck();
  CGInit();
  LookAheadLgInit();
  //BlockLocCenterInit();
  BlockLocRandomInit();
  if (circuit_->net_list.empty()) {
    if (globalVerboseLevel >= LOG_CRITICAL) {
      std::cout << "\033[0;36m"
                << "Global Placement complete\n"
                << "\033[0m";
    }
    return;
  }

  well_legalizer_.TakeOver(this);
  well_legalizer_.InitializeClusterLegalizer();
  well_legalizer_.ReportWellRule();

  InitialPlacement();
  //std::cout << cg_total_hpwl_ << "  " << circuit_->HPWL() << "\n";

  for (cur_iter_ = 0; cur_iter_ < max_iter_; ++cur_iter_) {
    if (globalVerboseLevel >= LOG_DEBUG) {
      std::cout << cur_iter_ << "-th iteration\n";
    }
    LookAheadLegalization();
    UpdateLALConvergeState();
    if (globalVerboseLevel >= LOG_CRITICAL) {
      printf("It %d: \t%e  %e\n", cur_iter_, cg_total_hpwl_, lal_total_hpwl_);
    }
    if (cur_iter_ > 10) {
      well_legalizer_.ClusterBlocks();
      well_legalizer_.LegalizeCluster(4 );
      well_legalizer_.UpdateBlockLocation();
      well_legalizer_.LocalReorderAllClusters();
      well_legalizer_.GenMatlabClusterTable("cl" + std::to_string(cur_iter_) + "_result");
    }
    if (HPWL_LAL_converge) { // if HPWL sconverges
      if (cur_iter_ >= 30) {
        if (globalVerboseLevel >= LOG_CRITICAL) {
          std::cout << "Iterative look-ahead legalization complete" << std::endl;
          std::cout << "Total number of iteration: " << cur_iter_ + 1 << std::endl;
        }
        break;
      }
    }
    QuadraticPlacementWithAnchor();

  }
  if (globalVerboseLevel >= LOG_CRITICAL) {
    std::cout << "\033[0;36m"
              << "Global Placement complete\n"
              << "\033[0m";
    printf("(cg time: %.4fs, lal time: %.4fs)\n", tot_cg_time, tot_lal_time);
  }
  LookAheadClose();
  //CheckAndShift();
  UpdateMovableBlkPlacementStatus();
  ReportHPWL(LOG_CRITICAL);

  wall_time = get_wall_time() - wall_time;
  cpu_time = get_cpu_time() - cpu_time;
  if (globalVerboseLevel >= LOG_CRITICAL) {
    printf("(wall time: %.4fs, cpu time: %.4fs)\n", wall_time, cpu_time);
  }
  ReportMemory(LOG_CRITICAL);
}