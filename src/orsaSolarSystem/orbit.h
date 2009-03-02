#ifndef _ORSA_SOLAR_SYSTEM_ORBIT_
#define _ORSA_SOLAR_SYSTEM_ORBIT_

#include <orsa/bodygroup.h>
#include <orsa/double.h>
#include <orsa/integrator_radau.h>
#include <orsa/multifit.h>
#include <orsa/orbit.h>

#include <orsaSolarSystem/observation.h>
#include <orsaSolarSystem/observatory.h>
#include <orsaSolarSystem/obleq.h>

#include <orsaSPICE/spice.h>
#include <orsaSPICE/spiceBodyPosVelCallback.h>

namespace orsaSolarSystem {
  
  class Residual {
  public:
    orsa::Cache<double> delta_ra, delta_dec;
  };
  
  double RMS(const std::vector<orsaSolarSystem::Residual> &);
  
  class OrbitWithEpoch : public orsa::Orbit {
  public:
    orsa::Cache<orsa::Time> epoch;
  };
  
  //! Residuals relative to orbit not integrated, good for times close to orbit epoch
  void ComputeResidual(std::vector<orsaSolarSystem::Residual> & residual,
		       const orsaSolarSystem::OrbitWithEpoch & orbit,
		       const orsaSolarSystem::ObservationVector & observationVector,
		       const orsaSolarSystem::ObservatoryPositionCallback * obsPosCB, 
		       const orsa::Body * refBody,
		       orsa::BodyGroup * bg);
  
  //! A class for Orbit Differential Corrections
  class OrbitMultifit : public orsa::Multifit {
  public:
    OrbitMultifit() :
      orsa::Multifit(),
      integrate(false) { }
      
  public:
    orsa::Cache<bool> integrate;
    
  public:
    orsa::Cache<orsa::Time> orbitEpoch;
    
  public:
    const orsaSolarSystem::ObservatoryPositionCallback * obsPosCB;
    
  protected:
    void computeAllFunctionCalls(const orsa::MultifitParameters * par, 
				 const orsa::MultifitData       * data,
				 const computeAllCallsMode        m) const { 
      
      // MODE_FDF is ignored, as MODE_F and MODE_DF will be called anyway
      
      if (m==MODE_F) {
	pre_f.resize(data->size());
	// for (unsigned int j=0; j<data->size(); ++j) {
	// pre_f[j] = fun(par,data,0,0,j);
	// pre_f[j] = realFunction(par,data,j);
	realFunction(pre_f,par,data);
	// }
      } else if (m==MODE_DF) {
	osg::ref_ptr<orsa::MultifitParameters> par_m = new orsa::MultifitParameters;
	osg::ref_ptr<orsa::MultifitParameters> par_p = new orsa::MultifitParameters;
	pre_df_m.resize(par->size());
	pre_df_p.resize(par->size());
	for (unsigned int p=0; p<par->size(); ++p) {
	  // deep copies of the original par
	  (*(par_m.get())) = (*par);
	  (*(par_p.get())) = (*par);
	  par_m->set(p,par->get(p)-par->getDelta(p));
	  par_p->set(p,par->get(p)+par->getDelta(p));
	  pre_df_m[p].resize(data->size());
	  pre_df_p[p].resize(data->size());
	  // for (unsigned int j=0; j<data->size(); ++j) {
	  // pre_df_m[p][j] = realFunction(par_m.get(),data,j);
	  // pre_df_p[p][j] = realFunction(par_p.get(),data,j);
	  realFunction(pre_df_m[p],par_m.get(),data);
	  realFunction(pre_df_p[p],par_p.get(),data);
	  // }
	}
      } else {
	ORSA_DEBUG("problems");
      }
    } 
  protected:
    mutable std::vector<double> pre_f;
    mutable std::vector<std::vector<double> > pre_df_m; // -1*delta
    mutable std::vector<std::vector<double> > pre_df_p; // +1*delta
    
  protected:
    virtual double fun(const orsa::MultifitParameters *, 
			     const orsa::MultifitData *,
			     const unsigned int p, 
			     const int          d,
			     const unsigned int row) const {
      if (d==0) { 
	return pre_f[row];
      } else {
	if (d==-1) {
	  return pre_df_m[p][row];
	} else if (d==1) {
	  return pre_df_p[p][row];
	} else {
	  ORSA_DEBUG("problems");
	  return 0;
	}
      }
    }
    
  protected:	
    void realFunction(std::vector<double>      & result,
		      const orsa::MultifitParameters * par, 
		      const orsa::MultifitData       * data) const {
      
      if (integrate.getRef()) {
	
	/*** INTEGRATING ***/
	
	osg::ref_ptr<orsa::BodyGroup> bg = new orsa::BodyGroup;
	
	osg::ref_ptr<orsa::Body> sun = new orsa::Body;
	{
	  sun->setName("SUN");
	  sun->isLightSource = true;
	  orsaSPICE::SpiceBodyPosVelCallback * sbpvc = 
	    new orsaSPICE::SpiceBodyPosVelCallback(sun->getName());
	  orsa::IBPS ibps;
	  ibps.inertial = new orsa::ConstantMassBodyProperty(FromUnits(1,orsa::Unit::MSUN));
	  ibps.translational = sbpvc;
	  sun->setInitialConditions(ibps);
	  //
	  bg->addBody(sun.get());
	}
      
	double mSun;
	if (!bg->getInterpolatedMass(mSun,
				     sun.get(),
				     orbitEpoch.getRef())) { 
	  ORSA_DEBUG("problems...");
	}	
      
	orsa::Vector rSun, vSun;
	if (!bg->getInterpolatedPosVel(rSun,
				       vSun,
				       sun.get(),
				       orbitEpoch.getRef())) { 
	  ORSA_DEBUG("problems");
	}
      
      
	// add some planets
	if (1) {
	
	  {
	    orsa::Body * mercury = new orsa::Body;
	    //
	    mercury->setName("MERCURY BARYCENTER");
	    // mercury->setMass(FromUnits(0.33022e24,Unit::KG));
	    //
	    orsaSPICE::SpiceBodyPosVelCallback * sbpvc = new orsaSPICE::SpiceBodyPosVelCallback(mercury->getName());
	    // sbpvc->setBodyName(mercury->getName());
	    orsa::IBPS ibps;
	    ibps.inertial = new orsa::ConstantMassBodyProperty(FromUnits(0.33022e24,orsa::Unit::KG));
	    ibps.translational = sbpvc;
	    mercury->setInitialConditions(ibps);
	    //
	    bg->addBody(mercury);
	  }
	
	  {
	    orsa::Body * venus = new orsa::Body;
	    //
	    venus->setName("VENUS BARYCENTER");
	    // venus->setMass(FromUnits(4.8690e24,Unit::KG));
	    //
	    orsaSPICE::SpiceBodyPosVelCallback * sbpvc = new orsaSPICE::SpiceBodyPosVelCallback(venus->getName());
	    // sbpvc->setBodyName(venus->getName());
	    orsa::IBPS ibps;
	    ibps.inertial = new orsa::ConstantMassBodyProperty(FromUnits(4.8690e24,orsa::Unit::KG));
	    ibps.translational = sbpvc;
	    venus->setInitialConditions(ibps);
	    //
	    bg->addBody(venus);
	  }
	
	  {
	    orsa::Body * earth = new orsa::Body;
	    //
	    earth->setName("EARTH");
	    // earth->setMass(FromUnits(5.9742e24,Unit::KG));
	    //
	    orsaSPICE::SpiceBodyPosVelCallback * sbpvc = new orsaSPICE::SpiceBodyPosVelCallback(earth->getName());
	    // sbpvc->setBodyName(earth->getName());
	    orsa::IBPS ibps;
	    ibps.inertial = new orsa::ConstantMassBodyProperty(FromUnits(1,orsa::Unit::MEARTH));
	    ibps.translational = sbpvc;
	    earth->setInitialConditions(ibps);
	    //
	    bg->addBody(earth);
	  }
	
	  {
	    orsa::Body * moon = new orsa::Body;
	    //
	    moon->setName("MOON");
	    //
	    orsaSPICE::SpiceBodyPosVelCallback * sbpvc = new orsaSPICE::SpiceBodyPosVelCallback(moon->getName());
	    orsa::IBPS ibps;
	    ibps.inertial = new orsa::ConstantMassBodyProperty(FromUnits(1,orsa::Unit::MMOON));
	    ibps.translational = sbpvc;
	    moon->setInitialConditions(ibps);
	    //
	    bg->addBody(moon);
	  }
	
	  {
	    orsa::Body * mars = new orsa::Body;
	    //
	    mars->setName("MARS BARYCENTER");
	    // mars->setMass(FromUnits(0.64191e24,Unit::KG));
	    //
	    orsaSPICE::SpiceBodyPosVelCallback * sbpvc = new orsaSPICE::SpiceBodyPosVelCallback(mars->getName());
	    // sbpvc->setBodyName(mars->getName());
	    orsa::IBPS ibps;
	    ibps.inertial = new orsa::ConstantMassBodyProperty(FromUnits(0.64191e24,orsa::Unit::KG));
	    ibps.translational = sbpvc;
	    mars->setInitialConditions(ibps);
	    //
	    bg->addBody(mars);
	  }
	
	  {
	    orsa::Body * jupiter = new orsa::Body;
	    //
	    jupiter->setName("JUPITER BARYCENTER");
	    // jupiter->setMass(FromUnits(1898.8e24,Unit::KG));
	    //
	    orsaSPICE::SpiceBodyPosVelCallback * sbpvc = new orsaSPICE::SpiceBodyPosVelCallback(jupiter->getName());
	    // sbpvc->setBodyName(jupiter->getName());
	    orsa::IBPS ibps;
	    ibps.translational = sbpvc;
	    ibps.inertial = new orsa::ConstantMassBodyProperty(FromUnits(1898.8e24,orsa::Unit::KG));
	    jupiter->setInitialConditions(ibps);
	    //
	    bg->addBody(jupiter);
	  }
	
	  {
	    orsa::Body * saturn = new orsa::Body;
	    //
	    saturn->setName("SATURN BARYCENTER");
	    // saturn->setMass(FromUnits(568.50e24,Unit::KG));
	    //
	    orsaSPICE::SpiceBodyPosVelCallback * sbpvc = new orsaSPICE::SpiceBodyPosVelCallback(saturn->getName());
	    // sbpvc->setBodyName(saturn->getName());
	    orsa::IBPS ibps;
	    ibps.translational = sbpvc;
	    ibps.inertial = new orsa::ConstantMassBodyProperty(FromUnits(568.50e24,orsa::Unit::KG));
	    saturn->setInitialConditions(ibps);
	    //
	    bg->addBody(saturn);
	  }
	
	  {
	    orsa::Body * uranus = new orsa::Body;
	    //
	    uranus->setName("URANUS BARYCENTER");
	    // uranus->setMass(FromUnits(86.625e24,Unit::KG));
	    //
	    orsaSPICE::SpiceBodyPosVelCallback * sbpvc = new orsaSPICE::SpiceBodyPosVelCallback(uranus->getName());
	    // sbpvc->setBodyName(uranus->getName());
	    orsa::IBPS ibps;
	    ibps.translational = sbpvc;
	    ibps.inertial = new orsa::ConstantMassBodyProperty(FromUnits(86.625e24,orsa::Unit::KG));
	    uranus->setInitialConditions(ibps);
	    //
	    bg->addBody(uranus);
	  }
	
	  {
	    orsa::Body * neptune = new orsa::Body;
	    //
	    neptune->setName("NEPTUNE BARYCENTER");
	    // neptune->setMass(FromUnits(102.78e24,Unit::KG));
	    //
	    orsaSPICE::SpiceBodyPosVelCallback * sbpvc = new orsaSPICE::SpiceBodyPosVelCallback(neptune->getName());
	    // sbpvc->setBodyName(neptune->getName());
	    orsa::IBPS ibps;
	    ibps.translational = sbpvc;
	    ibps.inertial = new orsa::ConstantMassBodyProperty(FromUnits(102.78e24,orsa::Unit::KG));
	    neptune->setInitialConditions(ibps);
	    //
	    bg->addBody(neptune);
	  }
	
	}
	
	orsaSolarSystem::OrbitWithEpoch orbit;
	//
	orbit.epoch = orbitEpoch.getRef();
	//
	orbit.mu = mSun*orsa::Unit::instance()->getG();
	//
	orbit.a = par->get("orbit_a");
	orbit.e = par->get("orbit_e");
	orbit.i = par->get("orbit_i");
	orbit.omega_node       = par->get("orbit_omega_node");
	orbit.omega_pericenter = par->get("orbit_omega_pericenter");
	orbit.M                = par->get("orbit_M");
	
	orsa::Vector rOrbit, vOrbit;
	if (!orbit.relativePosVel(rOrbit,vOrbit)) {
	  ORSA_DEBUG("problems");
	}
	rOrbit += rSun;
	vOrbit += vSun;
      
	osg::ref_ptr<orsa::Body> b = new orsa::Body;
	{
	  b->setName("b");
	  orsa::IBPS ibps;
	  ibps.time = orbitEpoch.getRef();
	  ibps.inertial = new orsa::ConstantMassBodyProperty(0);
	  ibps.translational = new orsa::DynamicTranslationalBodyProperty;
	  ibps.translational->setPosition(rOrbit);
	  ibps.translational->setVelocity(vOrbit);
	  b->setInitialConditions(ibps);
	  //
	  bg->addBody(b.get());
	}
      
	result.resize(data->size());
      
	// orsa::Vector refBodyPosition, refBodyVelocity;
	// double mass;
	orsa::Vector bodyPosition, bodyVelocity;
      
	osg::ref_ptr<orsa::IntegratorRadau> radau = new orsa::IntegratorRadau;
	radau->_accuracy = 1.0e-3;
      
	unsigned int row=0;
	while (row<data->size()) {
	
	  const orsa::Time obsTime = orsa::Time(data->getZ("epoch",row));
	
	  radau->integrate(bg.get(),
			   orbitEpoch.getRef(),
			   obsTime,
			   orsa::Time(0,0,10,0,0));
	
	  /* 
	     if (!bg->getInterpolatedPosVel(refBodyPosition,
	     refBodyVelocity,
	     sun.get(),
	     obsTime)) { 
	     ORSA_DEBUG("problems");
	     }
	     if (!bg->getInterpolatedMass(mass,
	     sun.get(),
	     obsTime)) { 
	     ORSA_DEBUG("problems...");
	     }	
	  */
	
	  if (!bg->getInterpolatedPosVel(bodyPosition,
					 bodyVelocity,
					 b.get(),
					 obsTime)) { 
	    ORSA_DEBUG("problems");
	  }
	
	  const orsa::Vector obsPosition = data->getV("obsPosition",row);
	
	  // code from OrbitSolution::computeRMS(...)
	
	  // orsa::Vector dr =  refBodyPosition+relativePosition-obsPosition;
	  orsa::Vector dr = bodyPosition-obsPosition;
	  const double lightTimeDelay = dr.length()/orsa::Unit::instance()->getC();
	  dr -= lightTimeDelay*(bodyVelocity);
	  //
	  dr = orsaSolarSystem::eclipticToEquatorial()*dr;
	  //
	  // ORSA_DEBUG("dr.length(): %f [AU]",orsa::FromUnits(dr.length(),orsa::Unit::AU,-1)());
	  //
	  const double ra_orbit  = fmod(atan2(dr.getY(),dr.getX())+orsa::twopi(),orsa::twopi());
	  const double dec_orbit = asin(dr.getZ()/dr.length());
	  
	  // (done) #warning "remember: cos_dec factor, and check for 360 deg periodicity"
	  
	  // const double radToArcsec = 3600*orsa::radToDeg();
	
	  // add both rows...
	
	  if (row%2==0) {
	    result[row] = ra_orbit;
	  } else {
	    ORSA_DEBUG("problems");
	  }
	  //
	  ++row;
	
	  if (row%2==0) {
	    ORSA_DEBUG("problems");
	  } else {
	    result[row] = dec_orbit;
	  }
	  //
	  ++row;
	
	}
      
      } else {
      
	/*** NOT INTEGRATING ***/
      
	osg::ref_ptr<orsa::BodyGroup> bg = new orsa::BodyGroup;
      
	osg::ref_ptr<orsa::Body> sun = new orsa::Body;
	{
	  sun->setName("SUN");
	  sun->isLightSource = true;
	  orsaSPICE::SpiceBodyPosVelCallback * sbpvc = 
	    new orsaSPICE::SpiceBodyPosVelCallback(sun->getName());
	  orsa::IBPS ibps;
	  ibps.inertial = new orsa::ConstantMassBodyProperty(FromUnits(1,orsa::Unit::MSUN));
	  ibps.translational = sbpvc;
	  sun->setInitialConditions(ibps);
	  //
	  bg->addBody(sun.get());
	}
      
	orsaSolarSystem::OrbitWithEpoch orbit;
	//
	orbit.epoch = orbitEpoch.getRef();
	//
	orbit.a = par->get("orbit_a");
	orbit.e = par->get("orbit_e");
	orbit.i = par->get("orbit_i");
	orbit.omega_node       = par->get("orbit_omega_node");
	orbit.omega_pericenter = par->get("orbit_omega_pericenter");
	orbit.M                = par->get("orbit_M");
      
	result.resize(data->size());
      
	orsa::Vector refBodyPosition, refBodyVelocity;
	double mass;
	orsa::Vector relativePosition, relativeVelocity;
      
	unsigned int row=0;
	while (row<data->size()) {
	
	  const orsa::Time obsTime = orsa::Time(data->getZ("epoch",row));
	
	  if (!bg->getInterpolatedPosVel(refBodyPosition,
					 refBodyVelocity,
					 sun.get(),
					 obsTime)) { 
	    ORSA_DEBUG("problems");
	  }
	
	  if (!bg->getInterpolatedMass(mass,
				       sun.get(),
				       obsTime)) { 
	    ORSA_DEBUG("problems...");
	  }	
	
	  orbit.mu = mass*orsa::Unit::instance()->getG();
	
	  {
	    orsa::Orbit localOrbit = orbit;
	    localOrbit.M = fmod(orbit.M + orsa::twopi()*(obsTime-orbit.epoch.getRef()).get_d()/orbit.period(),orsa::twopi()); 
	    if (!localOrbit.relativePosVel(relativePosition,relativeVelocity)) { ORSA_DEBUG("problems"); }
	  }
	
	  /* 
	     ORSA_DEBUG("[r orbit]");
	     ORSA_DEBUG("orbit: %f %f %f",
	     orbit.a(),
	     orbit.e(),
	     orbit.i());
	     print(relativePosition/orsa::FromUnits(1,orsa::Unit::KM));
	  */
	
	  const orsa::Vector obsPosition = data->getV("obsPosition",row);
	
	  // ORSA_DEBUG("observer position - sun:");
	  // print((obsPosition-refBodyPosition)/orsa::FromUnits(1,orsa::Unit::KM));
	
	  // code from OrbitSolution::computeRMS(...)
	
	  orsa::Vector dr = refBodyPosition+relativePosition-obsPosition;
	
	  // ORSA_DEBUG("[dr]");
	  // print(dr/orsa::FromUnits(1,orsa::Unit::KM));
	
	  const double lightTimeDelay = dr.length()/orsa::Unit::instance()->getC();
	  dr -= lightTimeDelay*(refBodyVelocity+relativeVelocity);
	  //
	  dr = orsaSolarSystem::eclipticToEquatorial()*dr;
	  //
	  // ORSA_DEBUG("dr.length(): %f [AU]",orsa::FromUnits(dr.length(),orsa::Unit::AU,-1)());
	  //
	  double  ra_orbit = fmod(atan2(dr.getY(),dr.getX())+orsa::twopi(),orsa::twopi());
	  double dec_orbit = asin(dr.getZ()/dr.length());
	  
	  // (done) #warning "remember: cos_dec factor, and check for 360 deg periodicity"
	  
	  // const double radToArcsec = 3600*orsa::radToDeg();
	
	  // add both rows...
	
	  if (row%2==0) {
	    result[row] = ra_orbit;
	  } else {
	    ORSA_DEBUG("problems");
	  }
	  //
	  ++row;
	
	  if (row%2==0) {
	    ORSA_DEBUG("problems");
	  } else {
	    result[row] = dec_orbit;
	  }
	  //
	  ++row;
	
	}
      }
    }
    
  public:
    int f_gsl (const gsl_vector * parameters, 
	       void * dataPoints, 
	       gsl_vector * f) {
      
      // call standard one, and then rescale
      int retval =  orsa::Multifit::f_gsl(parameters, 
					  dataPoints, 
					  f);
      const orsa::MultifitData * data = (orsa::MultifitData *) dataPoints;
      const double twopi = orsa::twopi();
      for (unsigned int j=0; j<data->size(); ++j) {
	if (j%2==0) {
	  // R.A.
	  // work with fj*sigma, and divide again by sigma at the end
	  const double sj = data->getSigma(j);
	  double fj = gsl_vector_get(f,j)*sj;
	  if (fabs(fj+twopi) < fabs(fj)) fj += twopi;
	  if (fabs(fj-twopi) < fabs(fj)) fj -= twopi;
	  fj /= sj;
	  // multiply by cos(dec)
	  gsl_vector_set(f,j, 
			 orsa::cos(data->getF(j+1))*fj);
	} 
	ORSA_DEBUG("f[%02i] = %10.3f", j, gsl_vector_get(f,j));
      }
      return retval;
    }
  
  public:
    int df_gsl (const gsl_vector * v, 
		void * dataPoints, 
		gsl_matrix * J) {
    
      int retval = Multifit::df_gsl(v, 
				    dataPoints, 
				    J);
      const orsa::MultifitData * data = (orsa::MultifitData *) dataPoints;
      for (unsigned int k=0; k<_par->size(); ++k) {
	for (unsigned int j=0; j<data->size(); ++j) {
	  if (j%2==0) {
	    // R.A.
	    // need to check for twopi periodicity? (see f_gsl)
	    // multiply by cos(dec)
	    gsl_matrix_set(J,j,k,
			   orsa::cos(data->getF(j+1))*gsl_matrix_get(J,j,k));  
	  }
	}
      }
      return retval;
    }
    
  };
  
}; // namespace orsaSolarSystem

#endif // _ORSA_SOLAR_SYSTEM_ORBIT_