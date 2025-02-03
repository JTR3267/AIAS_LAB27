#include "EXEStage.hh"

EXEStage::EXEStage(const std::string& name) : acalsim::SimModule(name) {}

EXEStage::~EXEStage() {}

void EXEStage::init() { CLASS_INFO << "EXEStage Initialization"; }

void EXEStage::step() {}
