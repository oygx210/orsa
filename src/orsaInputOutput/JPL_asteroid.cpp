#include <orsaInputOutput/JPL_asteroid.h>

#include <orsa/util.h>

#include <orsaSolarSystem/datetime.h>

using namespace orsa;
using namespace orsaInputOutput;
using namespace orsaSolarSystem;

// Numbered

bool JPLNumberedAsteroidFile::goodLine(const char * line) {
  
  if (strlen(line) < 105) return false;
  
  if (!isdigit(line[36])) return false;
  if (!isdigit(line[47])) return false;
  
  return true;
}

bool JPLNumberedAsteroidFile::processLine(const char * line) {
  
  std::string s_number;
  std::string s_designation;
  std::string s_epoch_MJD;
  std::string s_a, s_e, s_i, s_peri, s_node, s_M;
  std::string s_H;
  
  s_number.assign(line,0,6);
  
  s_designation.assign(line,7,17);
  removeLeadingAndTrailingSpaces(s_designation);
  
  s_epoch_MJD.assign(line,25,5);
  
  s_a.assign(line,31,10);
  s_e.assign(line,42,10);
  s_i.assign(line,53,9);
  s_peri.assign(line,63,9);
  s_node.assign(line,73,9);
  s_M.assign(line,83,11);
  
  s_H.assign(line,95,5);
  
  /* 
     ORSA_DEBUG("des: %s   MJD: %s",
     s_designation.c_str(),
     s_epoch_MJD.c_str());
  */
  
  // timescale?
  const Time epoch = orsaSolarSystem::julianToTime(orsaSolarSystem::MJD2JD(atof(s_epoch_MJD.c_str())));
  
  Orbit orbit;
  orbit.mu = orsa::Unit::instance()->getG()*orsa::FromUnits(1,orsa::Unit::MSUN); 
  orbit.e  = atof(s_e.c_str());
  //
  if (orbit.e > 0.99) {
    // non-periodic orbit, not included for the moment
    return false;
  }
  //
  orbit.a                = FromUnits(atof(s_a.c_str()),orsa::Unit::AU);
  orbit.i                = degToRad() * atof(s_i.c_str());
  orbit.omega_node       = degToRad() * atof(s_node.c_str());
  orbit.omega_pericenter = degToRad() * atof(s_peri.c_str());
  orbit.M                = degToRad() * atof(s_M.c_str());
  
  JPLAsteroidDataElement element;
  //
  element.orbit       = orbit;
  element.epoch       = epoch;
  element.H           = atof(s_H.c_str());
  element.number      = mpz_class(s_number);
  element.designation = s_designation;
  
  _data.push_back(element);
  
  return true;
}

// Unnumbered

bool JPLUnnumberedAsteroidFile::goodLine(const char * line) {
  
  if (strlen(line) < 88) return false;
  
  if (!isdigit(line[23])) return false;
  if (!isdigit(line[35])) return false;
  
  return true;
}

bool JPLUnnumberedAsteroidFile::processLine(const char * line) {
  
  // std::string s_number;
  std::string s_designation;
  std::string s_epoch_MJD;
  std::string s_a, s_e, s_i, s_peri, s_node, s_M;
  std::string s_H;
  
  // s_number.assign(line,0,6);
  
  s_designation.assign(line,0,11);
  removeLeadingAndTrailingSpaces(s_designation);
  
  s_epoch_MJD.assign(line,12,5);
  
  s_a.assign(line,18,11);
  s_e.assign(line,30,10);
  s_i.assign(line,41,9);
  s_peri.assign(line,51,9);
  s_node.assign(line,61,9);
  s_M.assign(line,71,11);
  
  s_H.assign(line,83,5);
  
  /* 
     ORSA_DEBUG("des: %s   MJD: %s",
     s_designation.c_str(),
     s_epoch_MJD.c_str());
  */
  
  // timescale?
  const Time epoch = orsaSolarSystem::julianToTime(orsaSolarSystem::MJD2JD(atof(s_epoch_MJD.c_str())));
  
  Orbit orbit;
  orbit.mu = orsa::Unit::instance()->getG()*orsa::FromUnits(1,orsa::Unit::MSUN); 
  orbit.e  = atof(s_e.c_str());
  //
  if (orbit.e > 0.99) {
    // non-periodic orbit, not included for the moment
    return false;
  }
  //
  orbit.a                = FromUnits(atof(s_a.c_str()),orsa::Unit::AU);
  orbit.i                = degToRad() * atof(s_i.c_str());
  orbit.omega_node       = degToRad() * atof(s_node.c_str());
  orbit.omega_pericenter = degToRad() * atof(s_peri.c_str());
  orbit.M                = degToRad() * atof(s_M.c_str());
  
  JPLAsteroidDataElement element;
  //
  element.orbit       = orbit;
  element.epoch       = epoch;
  element.H           = atof(s_H.c_str());
  // element.number      = mpz_class(s_number);
  element.designation = s_designation;
  
  _data.push_back(element);
  
  return true;
}