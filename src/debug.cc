//
// Created by Yihang Yang on 2019-05-14.
//

#include <iostream>
#include <vector>
#include <ctime>
#include "opendb.h"
#include "circuit.h"
#include "placer.h"

VerboseLevel globalVerboseLevel = LOG_DEBUG;

#define TEST_WELL 1

int main() {
  Circuit circuit;

  time_t Time = clock();

  std::string lef_file_name = "Pbenchmark_10K.lef";
  std::string def_file_name = "Pbenchmark_10K.def";

#ifdef USE_OPENDB
  odb::dbDatabase* db = odb::dbDatabase::create();
  std::vector<std::string> defFileVec;
  defFileVec.push_back(def_file_name);
  odb_read_lef(db, lef_file_name.c_str());
  odb_read_def(db, defFileVec);
  circuit.InitializeFromDB(db);
#else
  circuit.ReadLefFile(lef_file_name);
  circuit.ReadDefFile(def_file_name);
#endif

#if TEST_WELL
  std::string cell_file_name("Pbenchmark_10K.cell");
  circuit.ReadWellFile(cell_file_name);
#endif

  std::cout << "File loading complete, time: " << double(Time)/CLOCKS_PER_SEC << "s\n";

  circuit.ReportBriefSummary();
  //circuit.ReportBlockType();
  circuit.ReportHPWL();

  Placer *gb_placer = new GPSimPL;
  gb_placer->SetInputCircuit(&circuit);

  gb_placer->SetBoundaryDef();
  gb_placer->SetFillingRate(1);
  gb_placer->ReportBoundaries();
  gb_placer->StartPlacement();
  //gb_placer->GenMATLABScript("gb_result.txt");
  gb_placer->GenMATLABTable("gb_result.txt");
  //gb_placer->GenMATLABWellTable("gb_result");
  //gb_placer->SaveNodeTerminal();
  //gb_placer->SaveDEFFile("circuit.def", def_file);

  /*Placer *d_placer = new MDPlacer;
  d_placer->TakeOver(gb_placer);
  d_placer->StartPlacement();
  d_placer->GenMATLABScript("dp_result.txt");*/

  Placer *legalizer = new TetrisLegalizer;
  legalizer->TakeOver(gb_placer);
  legalizer->StartPlacement();
  //legalizer->GenMATLABScript("legalizer_result.txt");
  //legalizer->SaveDEFFile("circuit.def", def_file);

#if TEST_WELL
  Placer *well_legalizer = new WellLegalizer;
  well_legalizer->TakeOver(gb_placer);
  well_legalizer->StartPlacement();
  circuit.GenMATLABWellTable("lg_result");
  delete well_legalizer;
#endif


  delete gb_placer;
  //delete d_placer;
  delete legalizer;

#ifdef USE_OPENDB
  odb::dbDatabase::destroy(db);
#endif

  Time = clock() - Time;
  std::cout << "Execution time " << double(Time)/CLOCKS_PER_SEC << "s.\n";

  return 0;
}

#ifndef USE_OPENDB
void Test(Circuit &circuit) {
  std::string adaptec1_lef = "../test/adaptec1/adaptec1.lef";
  std::string adaptec1_def = "../test/adaptec1/adaptec1.def";

  std::string lef_file, def_file;
  circuit.SetGridValue(0.01,0.01);
  circuit.ReadLefFile(adaptec1_lef);
  circuit.ReadDefFile(adaptec1_def);
}
#endif
