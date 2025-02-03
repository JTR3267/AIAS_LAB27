#include "WBStage.hh"

WBStage::WBStage(const std::string& name) : acalsim::SimModule(name) {}

WBStage::~WBStage() {}

void WBStage::init() { CLASS_INFO << "WBStage Initialization"; }

void WBStage::step() {}
