//
// Created by Yihang Yang on 8/25/21.
//

#include "ioplacer.h"

#include "dali/common/logging.h"
#include "dali/common/timing.h"

#define NUM_OF_PLACE_BOUNDARY 4
#define LEFT 0
#define RIGHT 1
#define BOTTOM 2
#define TOP 3

namespace dali {

IoPlacer::IoPlacer() {
    InitializeBoundarySpaces();
}

IoPlacer::IoPlacer(phydb::PhyDB *phy_db, Circuit *circuit) {
    SetPhyDB(phy_db);
    SetCiruit(circuit);
    InitializeBoundarySpaces();
}

void IoPlacer::InitializeBoundarySpaces() {
    boundary_spaces_.reserve(NUM_OF_PLACE_BOUNDARY);
    // put all boundaries in a vector
    std::vector<double> boundary_loc{
        (double) circuit_ptr_->getDesignRef().region_left_,
        (double) circuit_ptr_->getDesignRef().region_right_,
        (double) circuit_ptr_->getDesignRef().region_bottom_,
        (double) circuit_ptr_->getDesignRef().region_top_
    };

    // initialize each boundary
    for (int i = 0; i < NUM_OF_PLACE_BOUNDARY; ++i) {
        boundary_spaces_.emplace_back(
            i == BOTTOM || i == TOP,
            boundary_loc[i]
        );
    }

}

void IoPlacer::SetCiruit(Circuit *circuit) {
    DaliExpects(circuit != nullptr,
                "Cannot initialize an IoPlacer without providing a valid Circuit pointer");
    circuit_ptr_ = circuit;
}

void IoPlacer::SetPhyDB(phydb::PhyDB *phy_db_ptr) {
    DaliExpects(phy_db_ptr != nullptr,
                "Cannot initialize an IoPlacer without providing a valid PhyDB pointer");
    phy_db_ptr_ = phy_db_ptr;
}

/* A function for add IOPIN to PhyDB database. This function should be called
 * before ordinary placement.
 *
 * Inputs:
 *  iopin_name: the name of the IOPIN
 *  net_name: the net this IOPIN is connected to
 *  direction: the direction of this IOPIN
 *  use: the usage of this IOPIN
 *
 * Return:
 *  true if this operation can be done successfully
 * */
bool IoPlacer::AddIoPin(std::string &iopin_name, std::string &net_name,
                        std::string &direction, std::string &use) {
    // check if this IOPIN is in PhyDB or not
    bool is_iopin_existing = phy_db_ptr_->IsIoPinExisting(iopin_name);
    if (is_iopin_existing) {
        BOOST_LOG_TRIVIAL(warning)
            << "IOPIN name is in PhyDB, cannot add it again: "
            << iopin_name << "\n";
        return false;
    }

    // check if this NET is in PhyDB or not
    bool is_net_existing = phy_db_ptr_->IsNetExisting(net_name);
    if (!is_net_existing) {
        BOOST_LOG_TRIVIAL(warning)
            << "NET name does not exist in PhyDB, cannot connect an IOPIN to it: "
            << net_name << "\n";
        return false;
    }

    // convert strings to direction and use
    phydb::SignalDirection signal_direction =
        phydb::StrToSignalDirection(direction);
    phydb::SignalUse signal_use = phydb::StrToSignalUse(use);

    // add this IOPIN to PhyDB
    phydb::IOPin *phydb_iopin =
        phy_db_ptr_->AddIoPin(iopin_name, signal_direction, signal_use);
    phydb_iopin->SetPlacementStatus(phydb::UNPLACED);
    phy_db_ptr_->AddIoPinToNet(iopin_name, net_name);

    // add it to Dali::Circuit
    circuit_ptr_->AddIoPinFromPhyDB(*phydb_iopin);

    return true;
}

/* A function for add IOPIN to PhyDB database interactively. This function should
 * be called before ordinary placement.
 *
 * Inputs:
 *  a list of argument from a user
 *
 * Return:
 *  true if this operation can be done successfully
 * */
bool IoPlacer::AddCmd(int argc, char **argv) {
    if (argc < 6) {
        BOOST_LOG_TRIVIAL(info)
            << "\033[0;36m"
            << "Add an IOPIN\n"
            << "Usage: -a/--add\n"
            << "    <iopin_name> : name of the new IOPIN\n"
            << "    <net_name>   : name of the net this IOPIN will connect to\n"
            << "    <direction>  : specifies the pin type: {INPUT | OUTPUT | INOUT | FEEDTHRU}\n"
            << "    <use>        : specifies how the pin is used: {ANALOG | CLOCK | GROUND | POWER | RESET | SCAN | SIGNAL | TIEOFF}\n"
            << "\033[0m\n";
        return false;
    }

    std::string iopin_name(argv[2]);
    std::string net_name(argv[3]);
    std::string direction(argv[4]);
    std::string use(argv[5]);

    return AddIoPin(iopin_name, net_name, direction, use);
}

/* A function for placing IOPINs interactively.
 *
 * Inputs:
 *  iopin_name: the name of the IOPIN
 *  metal_name: the metal layer to create its physical geometry
 *  shape_lx: lower left x of this IOPIN with respect to its location
 *  shape_ly: lower left y of this IOPIN with respect to its location
 *  shape_ux: upper right x of this IOPIN with respect to its location
 *  shape_uy: upper right y of this IOPIN with respect to its location
 *  place_status: placement status
 *  loc_x_: x location of this IOPIN on a boundary
 *  loc_y: y location of this IOPIN on a boundary
 *  orient: orientation of this IOPIN
 *
 * Return:
 *  true if this operation can be done successfully
 * */
bool IoPlacer::PlaceIoPin(std::string &iopin_name,
                          std::string &metal_name,
                          int shape_lx,
                          int shape_ly,
                          int shape_ux,
                          int shape_uy,
                          std::string &place_status,
                          int loc_x,
                          int loc_y,
                          std::string &orient) {
    // check if this IOPIN is in PhyDB or not
    bool is_iopin_existing_phydb = phy_db_ptr_->IsIoPinExisting(iopin_name);
    if (!is_iopin_existing_phydb) {
        BOOST_LOG_TRIVIAL(warning)
            << "IOPIN is not in PhyDB, cannot set its placement status: "
            << iopin_name << "\n";
        return false;
    }

    // get metal layer name
    int is_metal_layer_existing = circuit_ptr_->IsMetalLayerExist(metal_name);
    if (!is_metal_layer_existing) {
        BOOST_LOG_TRIVIAL(warning)
            << "The given metal layer index does not exist: "
            << metal_name << "\n";
        return false;
    }

    // set IOPIN placement status in PhyDB
    phydb::IOPin *phydb_iopin_ptr = phy_db_ptr_->GetIoPinPtr(iopin_name);
    phydb_iopin_ptr->SetShape(metal_name,
                              shape_lx,
                              shape_ly,
                              shape_ux,
                              shape_uy);
    phydb_iopin_ptr->SetPlacement(phydb::StrToPlaceStatus(place_status),
                                  loc_x, loc_y,
                                  phydb::StrToCompOrient(orient));

    // check if this IOPIN is in Dali::Circuit or not (it should)
    bool is_iopin_existing_dali = circuit_ptr_->IsIOPinExist(iopin_name);
    DaliExpects(is_iopin_existing_dali, "IOPIN in PhyDB but not in Dali?");
    IOPin *iopin_ptr_dali = circuit_ptr_->getIOPin(iopin_name);

    // set IOPIN placement status in Dali
    iopin_ptr_dali->SetLoc(circuit_ptr_->PhyDBLoc2DaliLocX(loc_x),
                           circuit_ptr_->PhyDBLoc2DaliLocY(loc_y),
                           StrToPlaceStatus(place_status));
    MetalLayer *metal_layer_ptr = circuit_ptr_->getMetalLayerPtr(metal_name);
    iopin_ptr_dali->SetLayer(metal_layer_ptr);

    return true;
}

bool IoPlacer::PlaceCmd(int argc, char **argv) {
    if (argc < 6) {
        BOOST_LOG_TRIVIAL(info)
            << "\033[0;36m"
            << "Add an IOPIN\n"
            << "Usage: -p/--place \n"
            << "    <iopin_name>  : name of the new IOPIN\n"
            << "    <metal_name>  : name of the metal layer to create its physical geometry\n"
            << "    <shape_lx>    : the pin geometry on that layer\n"
            << "    <shape_ly>    : the pin geometry on that layer\n"
            << "    <shape_ux>    : the pin geometry on that layer\n"
            << "    <shape_uy>    : the pin geometry on that layer\n"
            << "    <place_status>: placement status of this IOPIN: { COVER | FIXED | PLACED }\n"
            << "    <loc_x_>       : x location of this IOPIN\n"
            << "    <loc_y>       : y location of this IOPIN\n"
            << "    <orient>      : orientation of this IOPIN: { N | S | W | E | FN | FS | FW | FE }\n"
            << "\033[0m";
        return false;
    }

    std::string iopin_name(argv[2]);
    std::string metal_name(argv[3]);
    std::string shape_lx_str(argv[4]);
    std::string shape_ly_str(argv[5]);
    std::string shape_ux_str(argv[6]);
    std::string shape_uy_str(argv[7]);
    std::string place_status(argv[8]);
    std::string loc_x_str(argv[9]);
    std::string loc_y_str(argv[10]);
    std::string orient(argv[11]);

    int shape_lx = 0, shape_ly = 0, shape_ux = 0, shape_uy = 0;
    int loc_x = 0, loc_y = 0;
    try {
        shape_lx = std::stoi(shape_lx_str);
        shape_ly = std::stoi(shape_ly_str);
        shape_ux = std::stoi(shape_ux_str);
        shape_uy = std::stoi(shape_uy_str);
        loc_x = std::stoi(loc_x_str);
        loc_y = std::stoi(loc_y_str);
    } catch (...) {
        DaliExpects(false, "invalid IOPIN geometry or location");
    }

    return PlaceIoPin(iopin_name,
                      metal_name,
                      shape_lx,
                      shape_ly,
                      shape_ux,
                      shape_uy,
                      place_status,
                      loc_x,
                      loc_y,
                      orient);
}

bool IoPlacer::PartialPlaceIoPin() {
    return true;
}

bool IoPlacer::PartialPlaceCmd(int argc, char **argv) {
    return true;
}

bool IoPlacer::ConfigSetMetalLayer(int boundary_index, int metal_layer_index) {
    // check if this metal index exists or not
    bool is_legal_index = (metal_layer_index >= 0) &&
        (metal_layer_index
            < (int) circuit_ptr_->getTechRef().metal_list_.size());
    if (!is_legal_index) return false;
    MetalLayer *metal_layer =
        &(circuit_ptr_->getTechRef().metal_list_[metal_layer_index]);
    boundary_spaces_[boundary_index].AddLayer(metal_layer);
    return true;
}

bool IoPlacer::ConfigSetGlobalMetalLayer(int metal_layer_index) {
    for (int i = 0; i < NUM_OF_PLACE_BOUNDARY; ++i) {
        bool is_successful = ConfigSetMetalLayer(i, metal_layer_index);
        if (!is_successful) {
            return false;
        }
    }
    return true;
}

bool IoPlacer::ConfigAutoPlace() {
    return true;
}

void IoPlacer::ReportConfigUsage() {
    BOOST_LOG_TRIVIAL(info) << "\033[0;36m"
                            << "Usage: place-io -c/--config\n"
                            << "  -h/--help\n"
                            << "      print out function usage\n"
                            << "  -m/--metal <left/right/bottom/top> <metal layer>\n"
                            << "      use this command to specify which metal layers to use for IOPINs on each placement boundary\n"
                            << "      example: -m left m1, for IOPINs on the left boundary, using layer m1 to create physical geometry\n"
                            << "      'place-io <metal layer>' is a shorthand for 'place-io -c -m left m1 right m1 bottom m1 top m1'\n"
                            << "\033[0m\n";
}

bool IoPlacer::ConfigBoundaryMetal(int argc, char **argv) {
    if (argc < 5) {
        ReportConfigUsage();
        return false;
    }
    for (int i = 3; i < argc;) {
        std::string arg(argv[i++]);
        if (i < argc) {
            std::string metal_name = std::string(argv[i++]);
            bool is_layer_existing =
                circuit_ptr_->IsMetalLayerExist(metal_name);
            if (!is_layer_existing) {
                BOOST_LOG_TRIVIAL(fatal) << "Invalid metal layer name!\n";
                ReportConfigUsage();
                return false;
            }
            MetalLayer *metal_layer =
                circuit_ptr_->getMetalLayerPtr(metal_name);
            int metal_index = metal_layer->Num();
            if (arg == "left") {
                bool is_success = ConfigSetMetalLayer(LEFT, metal_index);
                std::cout << is_success << "\n";
            } else if (arg == "right") {
                bool is_success = ConfigSetMetalLayer(RIGHT, metal_index);
                std::cout << is_success << "\n";
            } else if (arg == "bottom") {
                bool is_success = ConfigSetMetalLayer(BOTTOM, metal_index);
                std::cout << is_success << "\n";
            } else if (arg == "top") {
                bool is_success = ConfigSetMetalLayer(TOP, metal_index);
                std::cout << is_success << "\n";
            } else {
                BOOST_LOG_TRIVIAL(fatal)
                    << "Invalid boundary, possible values: left, right, bottom, top\n";
                ReportConfigUsage();
                return false;
            }
            std::cout << arg << "  " << metal_name << "\n";
        } else {
            BOOST_LOG_TRIVIAL(fatal)
                << "Boundary specified, but metal layer is not given\n";
            ReportConfigUsage();
            return false;
        }
    }
    return true;
}

bool IoPlacer::ConfigCmd(int argc, char **argv) {
    if (argc < 2) {
        ReportConfigUsage();
        return false;
    }

    std::string option_str(argv[1]);
    bool is_config_flag = (option_str == "-c" or option_str == "--config");

    // when the command is like 'place-io <metal layer>'
    if (!is_config_flag) {
        bool is_metal = circuit_ptr_->IsMetalLayerExist(option_str);
        if (is_metal) {
            MetalLayer *metal_layer =
                circuit_ptr_->getMetalLayerPtr(option_str);
            return ConfigSetGlobalMetalLayer(metal_layer->Num());
        }
        BOOST_LOG_TRIVIAL(fatal) << "Invalid metal layer\n";
        ReportConfigUsage();
        return false;
    }

    // when the command is like 'place-io -c/--config ...'
    if (argc < 3) {
        ReportConfigUsage();
        return false;
    }
    option_str.assign(argv[2]);
    if (option_str == "-h" or option_str == "--help") {
        ReportConfigUsage();
        return true;
    } else if (option_str == "-m" or option_str == "--metal") {
        return ConfigBoundaryMetal(argc, argv);
    } else {
        BOOST_LOG_TRIVIAL(fatal) << "Unknown flag\n";
        ReportConfigUsage();
        return false;
    }
}

// resource should be large enough for all IOPINs
bool IoPlacer::CheckConfiguration() {
    return true;
}

bool IoPlacer::BuildResourceMap() {
    std::vector<std::vector<Seg<double>>> all_used_segments(
        NUM_OF_PLACE_BOUNDARY,
        std::vector<Seg<double>>(0)
    ); // TODO: carry layer info
    for (auto &iopin: *(circuit_ptr_->getIOPinList())) {
        if (iopin.IsPrePlaced()) {
            double spacing = iopin.Layer()->Spacing();
            if (iopin.X() == circuit_ptr_->getDesignRef().region_left_) {
                double lly = iopin.LY(spacing);
                double ury = iopin.UY(spacing);
                all_used_segments[LEFT].emplace_back(lly, ury);
            } else if (iopin.X()
                == circuit_ptr_->getDesignRef().region_right_) {
                double lly = iopin.LY(spacing);
                double ury = iopin.UY(spacing);
                all_used_segments[RIGHT].emplace_back(lly, ury);
            } else if (iopin.Y()
                == circuit_ptr_->getDesignRef().region_bottom_) {
                double llx = iopin.LX(spacing);
                double urx = iopin.UX(spacing);
                all_used_segments[BOTTOM].emplace_back(llx, urx);
            } else if (iopin.Y() == circuit_ptr_->getDesignRef().region_top_) {
                double llx = iopin.LX(spacing);
                double urx = iopin.UX(spacing);
                all_used_segments[TOP].emplace_back(llx, urx);
            } else {
                DaliExpects(false,
                            "Pre-placed IOPIN is not on placement boundary? "
                                + *(iopin.Name()));
            }
        }
    }

    std::vector<double> boundary_loc{
        (double) circuit_ptr_->getDesignRef().region_left_,
        (double) circuit_ptr_->getDesignRef().region_right_,
        (double) circuit_ptr_->getDesignRef().region_bottom_,
        (double) circuit_ptr_->getDesignRef().region_top_
    };

    for (int i = 0; i < NUM_OF_PLACE_BOUNDARY; ++i) {
        std::vector<Seg<double>> &used_segments = all_used_segments[i];
        std::sort(used_segments.begin(),
                  used_segments.end(),
                  [](const Seg<double> &lhs, const Seg<double> &rhs) {
                      return (lhs.lo < rhs.lo);
                  });
        std::vector<Seg<double>> avail_space;
        if (i == LEFT || i == RIGHT) {
            double lo = circuit_ptr_->getDesignRef().region_bottom_;
            double span = 0;
            int len = (int) used_segments.size();
            if (len == 0) {
                span = circuit_ptr_->getDesignRef().region_top_ - lo;
                boundary_spaces_[i].layer_spaces_[0].AddCluster(lo, span);
            }
            for (int j = 0; j < len; ++j) {
                if (lo < used_segments[j].lo) {
                    double hi = circuit_ptr_->getDesignRef().region_top_;
                    if (j + 1 < len) {
                        hi = used_segments[j + 1].lo;
                    }
                    span = hi - lo;
                    boundary_spaces_[i].layer_spaces_[0].AddCluster(lo, span);
                }
                lo = used_segments[j].hi;
            }
        } else {
            double lo = circuit_ptr_->getDesignRef().region_left_;
            double span = 0;
            int len = (int) used_segments.size();
            if (len == 0) {
                span = circuit_ptr_->getDesignRef().region_right_ - lo;
                boundary_spaces_[i].layer_spaces_[0].AddCluster(lo, span);
            }
            for (int j = 0; j < len; ++j) {
                if (lo < used_segments[j].lo) {
                    double hi = circuit_ptr_->getDesignRef().region_right_;
                    if (j + 1 < len) {
                        hi = used_segments[j + 1].lo;
                    }
                    span = hi - lo;
                    boundary_spaces_[i].layer_spaces_[0].AddCluster(lo, span);
                }
                lo = used_segments[j].hi;
            }
        }
    }
    return true;
}

bool IoPlacer::AssignIoPinToBoundaryLayers() {
    for (auto &iopin: *(circuit_ptr_->getIOPinList())) {
        // do nothing for placed IOPINs
        if (iopin.IsPrePlaced()) continue;

        // find the bounding box of the net containing this IOPIN
        Net *net = iopin.GetNet();
        if (net->blk_pin_list.empty()) {
            // if this net only contain this IOPIN, do nothing
            BOOST_LOG_TRIVIAL(warning) << "Net " << net->NameStr()
                                       << " only contains IOPIN "
                                       << *(iopin.Name())
                                       << ", skip placing this IOPIN\n";
            continue;
        }
        net->UpdateMaxMinIndex();
        double net_minx = net->MinX();
        double net_maxx = net->MaxX();
        double net_miny = net->MinY();
        double net_maxy = net->MaxY();

        // compute distances from edges of this bounding box to the corresponding placement boundary
        std::vector<double> distance_to_boundary{
            net_minx - circuit_ptr_->getDesignRef().region_left_,
            circuit_ptr_->getDesignRef().region_right_ - net_maxx,
            net_miny - circuit_ptr_->getDesignRef().region_bottom_,
            circuit_ptr_->getDesignRef().region_top_ - net_maxy
        };

        // compute candidate locations for this IOPIN on each boundary
        std::vector<double> loc_candidate_x{
            (double) circuit_ptr_->getDesignRef().region_left_,
            (double) circuit_ptr_->getDesignRef().region_right_,
            (net_minx + net_maxx) / 2,
            (net_minx + net_maxx) / 2
        };
        std::vector<double> loc_candidate_y{
            (net_maxy + net_miny) / 2,
            (net_maxy + net_miny) / 2,
            (double) circuit_ptr_->getDesignRef().region_bottom_,
            (double) circuit_ptr_->getDesignRef().region_top_
        };

        // determine which placement boundary this net bounding box is most close to
        std::vector<bool> close_to_boundary{false, false, false, false};
        double min_distance_x = std::min(distance_to_boundary[0],
                                         distance_to_boundary[1]);
        double min_distance_y = std::min(distance_to_boundary[2],
                                         distance_to_boundary[3]);
        if (min_distance_x < min_distance_y) {
            close_to_boundary[0] =
                distance_to_boundary[0] < distance_to_boundary[1];
            close_to_boundary[1] = !close_to_boundary[0];
        } else {
            close_to_boundary[2] =
                distance_to_boundary[2] < distance_to_boundary[3];
            close_to_boundary[3] = !close_to_boundary[2];
        }

        // set this IOPIN to the best candidate location
        for (int i = 0; i < NUM_OF_PLACE_BOUNDARY; ++i) {
            if (close_to_boundary[i]) {
                iopin.SetLoc(loc_candidate_x[i], loc_candidate_y[i], PLACED);
                boundary_spaces_[i].layer_spaces_[0].iopin_ptr_list.push_back(&iopin);
                break;
            }
        }
    }
    return true;
}

bool IoPlacer::PlaceIoPinOnEachBoundary() {
    for (auto &boundary_space: boundary_spaces_) {
        boundary_space.AutoPlaceIoPin();
    }
    return true;
}

bool IoPlacer::AutoPlaceIoPin() {
    if (!CheckConfiguration()) {
        return false;
    }

    BuildResourceMap();
    AssignIoPinToBoundaryLayers();
    PlaceIoPinOnEachBoundary();

    return true;
}

bool IoPlacer::AutoPlaceCmd(int argc, char **argv) {
    bool is_config_successful = ConfigCmd(argc, argv);
    if (!is_config_successful) {
        BOOST_LOG_TRIVIAL(fatal)
            << "Cannot successfully configure the IoPlacer\n";
        return false;
    }
    return AutoPlaceIoPin();
}

}