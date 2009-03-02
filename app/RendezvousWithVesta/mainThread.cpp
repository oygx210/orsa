#include "mainThread.h"

#include "multiminPhase.h"

#include <orsa/orbit.h>
#include <orsa/paulMoment.h>
#include <orsa/util.h>

#include <orsaSolarSystem/attitude.h>
#include <orsaSolarSystem/datetime.h>

#include <orsaSPICE/spice.h>
#include <orsaSPICE/spiceBodyPosVelCallback.h>
// #include <orsaSPICE/spiceBodyInterpolatedPosVelCallback.h>

#include <errno.h>

#include <SpiceUsr.h>

#include <gsl/gsl_rng.h>

#include "vesta.h"

#include <tbb/task_scheduler_init.h>

using namespace orsa;
using namespace orsaSolarSystem;
using namespace orsaSPICE;

CustomIntegrator::CustomIntegrator(const MainThread * mt) :
  QObject(),
  // IntegratorLeapFrog(),
  IntegratorRadau(),
  mainThread(mt) {
  // ORSA_DEBUG("update accuracy!");
  _accuracy = 1.0e-3;
  connect(this,
	  SIGNAL(progress(int)),
	  mainThread,
	  SIGNAL(progress(int)));
}

void CustomIntegrator::singleStepDone(orsa::BodyGroup  *,
				      const orsa::Time & t,
				      const orsa::Time & call_dt,
				      orsa::Time       &) const {
  if (mainThread != 0) {
    const int p = mpz_class((mpz_class("100")*((t+call_dt)-mainThread->orbitEpoch.getRef()).getMuSec()) /
			    mainThread->runDuration.getRef().getMuSec()).get_ui();
    // ORSA_DEBUG("p: %i",p);
    emit progress(p);
    
    ORSA_DEBUG("progress: %.6Ff \%",
	       Double((100*((t+call_dt)-mainThread->orbitEpoch.getRef()).getMuSec()) /
		      mainThread->runDuration.getRef().getMuSec()).get_mpf_t());
  }
}

/*****/

static std::string multipoleFileName(const ComboMassDistribution::MassDistributionType md,
				     const ComboShapeModel::ShapeModelType sm) {
  std::string s;
  if (md == ComboMassDistribution::mdt_uniform) {
    if (sm == ComboShapeModel::smt_ellipsoid) {
      s = "multipole_uniform_ellipsoid.dat";
    } else if (sm == ComboShapeModel::smt_thomas) {
      s = "multipole_uniform_thomas.dat";
    } else {
      ORSA_ERROR("problems");
    }
  } else if (md == ComboMassDistribution::mdt_core) {
    if (sm == ComboShapeModel::smt_ellipsoid) {
      s = "multipole_core_ellipsoid.dat";
    } else if (sm == ComboShapeModel::smt_thomas) {
      s = "multipole_core_thomas.dat";
    } else {
      ORSA_ERROR("problems");
    }
  } else {
    ORSA_ERROR("problems");
  }
  return s;
}

/*****/

MainThread::MainThread() :
  QThread() {
  
  customIntegrator = new CustomIntegrator(this);

  SPICE::instance()->setDefaultObserver("SSB");
  //
  SPICE::instance()->loadKernel("de405.bsp");
  // SPICE::instance()->loadKernel("vesta_1900_2100.bsp");
  SPICE::instance()->loadKernel("vesta-2003-2013.bsp");
}

void MainThread::run() {
  
  // TBB
  tbb::task_scheduler_init init;
  
  // const Time samplingPeriod(0,0,30,0,0);
  const Time samplingPeriod(0,0,5,0,0);

  // if false, use interpolation
  const bool accurateSPICE = true;
  // const bool accurateSPICE = false;

  /*
     SPICE::instance()->setDefaultObserver("SSB");
     //
     SPICE::instance()->loadKernel("de405.bsp");
     // SPICE::instance()->loadKernel("vesta_1900_2100.bsp");
     SPICE::instance()->loadKernel("vesta-2003-2013.bsp");
  */

  // osg::ref_ptr<BodyGroup> bg = new BodyGroup;
  bg = new BodyGroup;

  osg::ref_ptr<Body> sun = new Body;
  {
    sun->setName("SUN");
    //
    // sun->setMass(FromUnits(one(),Unit::MSUN));
    // sun->setMass(zero());
    //
    sun->isLightSource = true;
    //
    /*
       if (1) {
       const orsa::Body * b = sun.get();
       BodyGroup::TRV trv;
       orsa::Time t = orbitEpoch.getRef();
       while (t <= orbitEpoch.getRef()+runDuration.getRef()) {
       trv.t = t;
       SPICE::instance()->getPosVel(b->getName(),
       trv.t,
       trv.r,
       trv.v);
       bg->insertTRV(trv,b);
       t += samplingPeriod;
       }
       }
    */
    //
    /*
       if (1) {
       const orsa::Body * b = sun.get();
       BodyGroup::TRV trv;
       const orsa::Time dt(0,0,0,1,0);
       orsa::Time t = orbitEpoch.getRef();
       while (t <= orbitEpoch.getRef()+runDuration.getRef()) {
       trv.t = t;
       SPICE::instance()->getPosVel(b->getName(),
       trv.t,
       trv.r,
       trv.v);
       ORSA_DEBUG("sSs: %Ff %Ff %Ff %Ff",
       julianTime(t).get_mpf_t(),
       trv.r.getX().get_mpf_t(),
       trv.r.getY().get_mpf_t(),
       trv.r.getZ().get_mpf_t());
       t += dt;
       }
       }
    */
    //
    if (accurateSPICE) {
      SpiceBodyPosVelCallback * sbpvc = new SpiceBodyPosVelCallback(sun->getName());
      // sbpvc->setBodyName(sun->getName());
      orsa::IBPS ibps;
      ibps.inertial = new ConstantMassBodyProperty(FromUnits(one(),Unit::MSUN));
      ibps.translational = sbpvc;
      sun->setInitialConditions(ibps);
    } else {
      /* 
	 SpiceBodyInterpolatedPosVelCallback * sbipvc =
	 new SpiceBodyInterpolatedPosVelCallback(sun->getName(),
	 orbitEpoch.getRef(),
	 orbitEpoch.getRef()+runDuration.getRef(),
	 samplingPeriod);
	 sun->setBodyPosVelCallback(sbipvc);
      */
    }
    // }
    //
    bg->addBody(sun.get());
  }

  // add some planets
  if (1) {

    {
      Body * mercury = new Body;
      //
      mercury->setName("MERCURY BARYCENTER");
      // mercury->setMass(FromUnits(0.33022e24,Unit::KG));
      //
      SpiceBodyPosVelCallback * sbpvc = new SpiceBodyPosVelCallback(mercury->getName());
      // sbpvc->setBodyName(mercury->getName());
      orsa::IBPS ibps;
      ibps.inertial = new ConstantMassBodyProperty(FromUnits(0.33022e24,Unit::KG));
      ibps.translational = sbpvc;
      mercury->setInitialConditions(ibps);
      //
      bg->addBody(mercury);
    }

    {
      Body * venus = new Body;
      //
      venus->setName("VENUS BARYCENTER");
      // venus->setMass(FromUnits(4.8690e24,Unit::KG));
      //
      SpiceBodyPosVelCallback * sbpvc = new SpiceBodyPosVelCallback(venus->getName());
      // sbpvc->setBodyName(venus->getName());
      orsa::IBPS ibps;
      ibps.inertial = new ConstantMassBodyProperty(FromUnits(4.8690e24,Unit::KG));
      ibps.translational = sbpvc;
      venus->setInitialConditions(ibps);
      //
      bg->addBody(venus);
    }

    {
      Body * earth = new Body;
      //
      earth->setName("EARTH BARYCENTER");
      // earth->setMass(FromUnits(5.9742e24,Unit::KG));
      //
      SpiceBodyPosVelCallback * sbpvc = new SpiceBodyPosVelCallback(earth->getName());
      // sbpvc->setBodyName(earth->getName());
      orsa::IBPS ibps;
      ibps.inertial = new ConstantMassBodyProperty(FromUnits(5.9742e24,Unit::KG));
      ibps.translational = sbpvc;
      earth->setInitialConditions(ibps);
      //
      bg->addBody(earth);
    }

    {
      Body * mars = new Body;
      //
      mars->setName("MARS BARYCENTER");
      // mars->setMass(FromUnits(0.64191e24,Unit::KG));
      //
      SpiceBodyPosVelCallback * sbpvc = new SpiceBodyPosVelCallback(mars->getName());
      // sbpvc->setBodyName(mars->getName());
      orsa::IBPS ibps;
      ibps.inertial = new ConstantMassBodyProperty(FromUnits(0.64191e24,Unit::KG));
      ibps.translational = sbpvc;
      mars->setInitialConditions(ibps);
      //
      bg->addBody(mars);
    }
    
    {
      Body * jupiter = new Body;
      //
      jupiter->setName("JUPITER BARYCENTER");
      // jupiter->setMass(FromUnits(1898.8e24,Unit::KG));
      //
      SpiceBodyPosVelCallback * sbpvc = new SpiceBodyPosVelCallback(jupiter->getName());
      // sbpvc->setBodyName(jupiter->getName());
      orsa::IBPS ibps;
      ibps.translational = sbpvc;
      ibps.inertial = new ConstantMassBodyProperty(FromUnits(1898.8e24,Unit::KG));
      jupiter->setInitialConditions(ibps);
      //
      bg->addBody(jupiter);
    }

    {
      Body * saturn = new Body;
      //
      saturn->setName("SATURN BARYCENTER");
      // saturn->setMass(FromUnits(568.50e24,Unit::KG));
      //
      SpiceBodyPosVelCallback * sbpvc = new SpiceBodyPosVelCallback(saturn->getName());
      // sbpvc->setBodyName(saturn->getName());
      orsa::IBPS ibps;
      ibps.translational = sbpvc;
      ibps.inertial = new ConstantMassBodyProperty(FromUnits(568.50e24,Unit::KG));
      saturn->setInitialConditions(ibps);
      //
      bg->addBody(saturn);
    }

    {
      Body * uranus = new Body;
      //
      uranus->setName("URANUS BARYCENTER");
      // uranus->setMass(FromUnits(86.625e24,Unit::KG));
      //
      SpiceBodyPosVelCallback * sbpvc = new SpiceBodyPosVelCallback(uranus->getName());
      // sbpvc->setBodyName(uranus->getName());
      orsa::IBPS ibps;
      ibps.translational = sbpvc;
      ibps.inertial = new ConstantMassBodyProperty(FromUnits(86.625e24,Unit::KG));
      uranus->setInitialConditions(ibps);
      //
      bg->addBody(uranus);
    }
    
    {
      Body * neptune = new Body;
      //
      neptune->setName("NEPTUNE BARYCENTER");
      // neptune->setMass(FromUnits(102.78e24,Unit::KG));
      //
      SpiceBodyPosVelCallback * sbpvc = new SpiceBodyPosVelCallback(neptune->getName());
      // sbpvc->setBodyName(neptune->getName());
      orsa::IBPS ibps;
      ibps.translational = sbpvc;
      ibps.inertial = new ConstantMassBodyProperty(FromUnits(102.78e24,Unit::KG));
      neptune->setInitialConditions(ibps);
      //
      bg->addBody(neptune);
    }
    
  }
  
  osg::ref_ptr<Body> vesta = new Body;
  {
    vesta->setName("VESTA");
    // vesta->setMass(vestaMass.getRef());
    //
    /*
       if (1) {
       const orsa::Body * b = vesta.get();
       BodyGroup::TRV trv;
       orsa::Time t = orbitEpoch.getRef();
       while (t <= orbitEpoch.getRef()+runDuration.getRef()) {
       trv.t = t;
       SPICE::instance()->getPosVel(b->getName(),
       trv.t,
       trv.r,
       trv.v);
       bg->insertTRV(trv,b);
       t += samplingPeriod;
       }
       } else {
    */
    //
    if (accurateSPICE) {
      osg::ref_ptr<SpiceBodyPosVelCallback> sbpvc = new SpiceBodyPosVelCallback(vesta->getName());
      //
      // sbpvc->setBodyName(vesta->getName());
      //
      orsa::IBPS ibps;
      ibps.inertial = new ConstantMassBodyProperty(vestaMass.getRef());
      ibps.translational = sbpvc.get();
      vesta->setInitialConditions(ibps);
    } else {
      /* 
	 SpiceBodyInterpolatedPosVelCallback * sbipvc =
	 new SpiceBodyInterpolatedPosVelCallback(vesta->getName(),
	 orbitEpoch.getRef(),
	 orbitEpoch.getRef()+runDuration.getRef(),
	 samplingPeriod);
	 vesta->setBodyPosVelCallback(sbipvc);
      */
    }
    //
    // }
    //
    /*
       {
       osg::ref_ptr<orsa::BodyInitialConditions> vesta_bic = new orsa::BodyInitialConditions;
       //
       vesta_bic->setTime(orbitEpoch.getRef());
       vesta_bic->setPosition(orsa::Vector(1e12,1e12,-3e12));
       vesta_bic->setVelocity(orsa::Vector(0.1,-0.2,0.3));
       //
       vesta->setInitialConditions(vesta_bic.get());
       }
    */


    /* OLD!
       vesta->setAttitude(new ConstantZRotationEclipticAttitude(J2000(),
       287.0*degToRad(),
       twopi()/vestaPeriod.getRef(),
       vestaPoleEclipticLongitude.getRef(),
       vestaPoleEclipticLatitude.getRef()));
    */
    //
    
    {
      IBPS ibps = vesta->getInitialConditions();
      
      /* 
	 ibps.rotational = new orsa::ConstantZRotation(J2000(),
	 292.0*degToRad(),
	 twopi()/vestaPeriod.getRef());
      */
      
      ibps.rotational = new orsaSolarSystem::ConstantZRotationEcliptic_RotationalBodyProperty(J2000(),
											      292.0*degToRad(),
											      twopi()/vestaPeriod.getRef(),
											      vestaPoleEclipticLongitude.getRef(),
											      vestaPoleEclipticLatitude.getRef());
      vesta->setInitialConditions(ibps);
    }
    
    // gravitational multipole
    /* 
       {
       osg::ref_ptr<Multipole> m = new Multipole;
       
       {
       // read multipole from file
       const std::string fileName = multipoleFileName(vestaMassDistribution.getRef(),
       vestaShapeModel.getRef());
       if (!m->readFromFile(fileName)) {
       ORSA_ERROR("...");
       }
       }
       
       {
       // shape
       if (vestaShapeModel.getRef() == ComboShapeModel::smt_ellipsoid) {
       m->setShape(new EllipsoidShape(FromUnits(280,Unit::KM),
       FromUnits(272,Unit::KM),
       FromUnits(227,Unit::KM)));
       } else if (vestaShapeModel.getRef() == ComboShapeModel::smt_thomas) {
       osg::ref_ptr<VestaShape> vestaShapeThomas = new VestaShape;
       if (!vestaShapeThomas->read("vesta_thomas.dat")) {
       ORSA_ERROR("problems encountered while reading shape file...");
       }
       m->setShape(vestaShapeThomas.get());
       } else {
       ORSA_ERROR("problems");
       }
       }
       
       ORSA_DEBUG("re-enable this...");
       // vesta->setMultipole(m.get());
       
       vesta->setShape(m->getShape());
       }
    */
    
    // alternative to Multipole: PaulMoment
    {
      osg::ref_ptr<PaulMoment> pm = new PaulMoment(4);
      
#warning "code needed here, for mass distribution..."
      // implement this later...
      // pm->setMassDistribution(vestaMassDistribution.getRef());
      
      {
	// shape
	if (vestaShapeModel.getRef() == ComboShapeModel::smt_ellipsoid) {
	  pm->setShape(new EllipsoidShape(FromUnits(280,Unit::KM),
					  FromUnits(272,Unit::KM),
					  FromUnits(227,Unit::KM)));
	} else if (vestaShapeModel.getRef() == ComboShapeModel::smt_thomas) {
	  osg::ref_ptr<VestaShape> vestaShapeThomas = new VestaShape;
	  if (!vestaShapeThomas->read("vesta_thomas.dat")) {
	    ORSA_ERROR("problems encountered while reading shape file...");
	  }
	  pm->setShape(vestaShapeThomas.get());
	} else {
	  ORSA_ERROR("problems");
	}
      }
      
      // read from file... code to come...
      
      {
	pm->computeUsingShape(16384,
			      93881);
      }
      
      vesta->setPaulMoment(pm.get());
      
      vesta->setShape(pm->getShape());      
    }
    
    bg->addBody(vesta.get());
  }

  // ORSA_DEBUG("before DAWN...");

  osg::ref_ptr<Body> dawn = new Body;
  {

    Double alpha = zero();
    //
    // computations for the phase
    {
      orsa::Vector rVesta, vVesta;
      orsa::Vector rSun, vSun;
      if (bg->getInterpolatedPosVel(rVesta,
				    vVesta,
				    vesta.get(),
				    orbitEpoch.getRef()) &&
	  bg->getInterpolatedPosVel(rSun,
				    vSun,
				    sun.get(),
				    orbitEpoch.getRef())) {
	
	// osg::ref_ptr<orsa::Attitude> vesta_attitude = new BodyAttitude(vesta.get(),bg.get());
	
	// const Matrix g2l = vesta->getAttitude()->globalToLocal(orbitEpoch.getRef());
	
	// const Matrix g2l = vesta_attitude->globalToLocal(orbitEpoch.getRef());
	
	const orsa::Matrix g2l = orsa::globalToLocal(vesta.get(),bg.get(),orbitEpoch.getRef());
	
	const Vector uVesta2Sun_local = (g2l*(rSun-rVesta).normalized()).normalized();

	const Double uSun_z = uVesta2Sun_local.getZ();
	
	// initial rotation, to align the local X axis with the Sun
	// alpha = atan2(uVesta2Sun_local.getY(),uVesta2Sun_local.getX());

	// should check globally for 0 <= i <= 180
	/*
	   {
	   const Double si = sin(orbitInclination.getRef());
	   if (si != zero()) {
	   const Double dAlpha = asin(uSun_z/si);
	   alpha += dAlpha;
	   ORSA_DEBUG("dAlpha: %Ff",
	   Double(radToDeg()*dAlpha).get_mpf_t());
	   }
	   }
	*/

	/*
	   ORSA_DEBUG("initial alpha: %Ff",
	   Double(radToDeg()*alpha).get_mpf_t());
	*/

	/*
	// to cross-check...
	const Double beta   = acos(uSun_z);
	// const Double phiMin = fabs(beta-orbitInclination.getRef());
	// const Double phiMax = fabs(beta+orbitInclination.getRef());
	const Double phiMin = fabs(halfpi()-fabs(beta+orbitInclination.getRef()));
	const Double phiMax = fabs(halfpi()-fabs(beta-orbitInclination.getRef()));

	ORSA_DEBUG("beta: %Ff",
		   Double(radToDeg()*beta).get_mpf_t());

	ORSA_DEBUG("phase range: %Ff to %Ff",
		   Double(radToDeg()*phiMin).get_mpf_t(),
		   Double(radToDeg()*phiMax).get_mpf_t());
	*/

	// const Double i_z = cos(orbitInclination.getRef());

	/*
	   const Vector uI = orsa::Vector(sqrt(one()-i_z*i_z)*sin(alpha),
	   -sqrt(one()-i_z*i_z)*cos(alpha),
	   i_z).normalized();

	   const Double zProduct = i_z*uSun_z;

	   const Double scalarProduct = uVesta2Sun_local * uI;

	   // ORSA_DEBUG("zProduct: %Ff",zProduct.get_mpf_t());

	   Double tmpArg;
	   if (scalarProduct > zero()) {
	   sin(orbitPhase.getRef())-zProduct;
	   } else {
	   -sin(orbitPhase.getRef())-zProduct;
	   }
	   const Double arg = tmpArg;
	*/

	/*
	   if (orbitPhase.getRef() < phiMin) {
	   ORSA_WARNING("phase angle requested [%Ff] smaller than minimum admissible value [%Ff]",
	   Double(radToDeg()*orbitPhase.getRef()).get_mpf_t(),
	   Double(radToDeg()*phiMin).get_mpf_t());
	   alpha += zero();
	   } else if (orbitPhase.getRef() > phiMax) {
	   ORSA_WARNING("phase angle requested [%Ff] bigger than minimum admissible value [%Ff]",
	   Double(radToDeg()*orbitPhase.getRef()).get_mpf_t(),
	   Double(radToDeg()*phiMax).get_mpf_t());
	   alpha += pi();
	   } else {
	   osg::ref_ptr<MultiminPhase> mmp = new MultiminPhase;
	   const Double mmpAlpha = mmp->getAlpha(orbitPhase.getRef(),
	   uVesta2Sun_local,
	   orsa::Vector(zero(),
	   -sqrt(one()-i_z*i_z),
	   i_z));
	   ORSA_DEBUG("mmpAlpha: %Ff",
	   Double(radToDeg()*mmpAlpha).get_mpf_t());
	   alpha += mmpAlpha;
	   }
	*/

	{
	  // let's just call this in any case...

	  const Double i_z = cos(orbitInclination.getRef());
	  osg::ref_ptr<MultiminPhase> mmp = new MultiminPhase;
	  const Double mmpAlpha = mmp->getAlpha(fmod(fmod(orbitPhase.getRef(),twopi())+twopi(),twopi()),
						uVesta2Sun_local,
						orsa::Vector(zero(),
							     -sqrt(one()-i_z*i_z),
							     i_z));
	  alpha = mmpAlpha;
	}

	/*
	   if ((fabs(uSun_z) < one()) &&
	   (fabs(i_z)    < one())) {
	   const Double finalArg = arg/(sqrt(one()-i_z*i_z)*sqrt(one()-uSun_z*uSun_z));
	   ORSA_DEBUG("arg: %Ff   finalArg: %Ff",
	   arg.get_mpf_t(),
	   finalArg.get_mpf_t());
	   if (finalArg > one()) {
	   ORSA_WARNING("phase angle requested [%Ff] smaller than minimum admissible value [%Ff]",
	   Double(radToDeg()*orbitPhase.getRef()).get_mpf_t(),
	   Double(radToDeg()*phiMin).get_mpf_t());
	   alpha += zero();
	   } else if (finalArg < (-one())) {
	   ORSA_WARNING("phase angle requested [%Ff] bigger than minimum admissible value [%Ff]",
	   Double(radToDeg()*orbitPhase.getRef()).get_mpf_t(),
	   Double(radToDeg()*phiMax).get_mpf_t());
	   alpha += pi();
	   } else {
	   osg::ref_ptr<MultiminPhase> mmp = new MultiminPhase;
	   const Double mmpAlpha = mmp->getAlpha(orbitPhase.getRef(),
	   uVesta2Sun_local,
	   orsa::Vector(zero(),
	   -sqrt(one()-i_z*i_z),
	   i_z));
	   ORSA_DEBUG("mmpAlpha: %Ff",
	   Double(radToDeg()*mmpAlpha).get_mpf_t());
	   alpha += mmpAlpha;
	   }
	   } else {
	   // In this case:
	   // no matter what we do, the phase is fixed,
	   // because either uVesta2Sun_local or the orbit pole
	   // is aligned along the local Z axis.
	   ORSA_WARNING("phase angle locked at [%Ff]",
	   Double(radToDeg()*phiMin).get_mpf_t());
	   alpha += zero();
	   }
	*/

	/*
	   const Double beta   = acos(uVesta2Sun_local.getZ());
	   const Double phiMin = fabs(beta-orbitInclination.getRef());
	   const Double phiMax = fabs(beta+orbitInclination.getRef());

	   if (orbitPhase.getRef() < phiMin) {
	   ORSA_WARNING("");
	   } else if (orbitPhase.getRef() > phiMax) {
	   ORSA_WARNING("");
	   } else {
	   }
	*/


      }
    }

    /*
       ORSA_DEBUG("final alpha: %Ff",
       Double(radToDeg()*alpha).get_mpf_t());
    */

    dawn->setName("DAWN");
    // dawn->setMass(zero());
    //
    // osg::ref_ptr<orsa::BodyInitialConditions> dawn_bic = new orsa::BodyInitialConditions;
    IBPS ibps;
    //
    orsa::Orbit orbit;
    //
    orbit.mu = orsa::Unit::instance()->getG() * vestaMass.getRef();
    orbit.a  = orbitRadius.getRef();
    orbit.e  = zero();
    orbit.i  = orbitInclination.getRef();
    orbit.omega_node       = alpha;
    orbit.omega_pericenter = zero();
    orbit.M                = zero();
    //
    orsa::Vector rOrbit, vOrbit;
    orbit.relativePosVel(rOrbit,vOrbit);
    
    ORSA_DEBUG("rOrbit: %.20Fe",rOrbit.length().get_mpf_t());
    
    // ORSA_DEBUG("rotate for attitude! (and phase?)");
    {
      // osg::ref_ptr<orsa::Attitude> vesta_attitude = new BodyAttitude(vesta.get(),bg.get());
      
      // const Matrix l2g = vesta->getAttitude()->localToGlobal(orbitEpoch.getRef());
      //
      // const Matrix l2g = vesta_attitude->localToGlobal(orbitEpoch.getRef());
      
      const orsa::Matrix l2g = orsa::localToGlobal(vesta.get(),bg.get(),orbitEpoch.getRef());
      
      // ORSA_DEBUG("l2g.getM11(): %Fe",l2g.getM11().get_mpf_t());
      
      rOrbit = l2g * rOrbit;
      vOrbit = l2g * vOrbit;

      ORSA_DEBUG("rOrbit: %.20Fe",rOrbit.length().get_mpf_t());
    }
    
    {
      orsa::Vector rVesta, vVesta;
      if (bg->getInterpolatedPosVel(rVesta,
				    vVesta,
				    vesta.get(),
				    orbitEpoch.getRef())) {
	rOrbit += rVesta;
	vOrbit += vVesta;
      } else {
	ORSA_DEBUG("problems!");
      }
    }
    
    /* 
       dawn_bic->setTime(orbitEpoch.getRef());
       dawn_bic->setPosition(rOrbit);
       dawn_bic->setVelocity(vOrbit);
    */
    //
    /* 
       dawn_bic->time     = orbitEpoch.getRef();
       dawn_bic->position = rOrbit;
       dawn_bic->velocity = vOrbit;
    */
    //
    ibps.time = orbitEpoch.getRef();
    //
    ibps.inertial = new ConstantMassBodyProperty(0);
    //
    ibps.translational = new DynamicTranslationalBodyProperty;
    ibps.translational->setPosition(rOrbit);
    ibps.translational->setVelocity(vOrbit);
    
    // dawn->setInitialConditions(dawn_bic.get());
    dawn->setInitialConditions(ibps);
    
    {
      osg::ref_ptr<PaulMoment> dummy_pm = new PaulMoment(0);
      //
      dummy_pm->setM(one(),0,0,0);
      dummy_pm->setCenterOfMass(orsa::Vector(0,0,0));
      dummy_pm->setInertiaMoment(orsa::Matrix::identity());
      //
      dawn->setPaulMoment(dummy_pm.get());
    }
    
    // test
    ORSA_DEBUG("========= DAWN time: %.6Ff",ibps.time.getRef().asDouble().get_mpf_t());
    
    bg->addBody(dawn.get());
    
    // test
    ORSA_DEBUG("========= DAWN time: %.6Ff",
	       dawn->getInitialConditions().time.getRef().asDouble().get_mpf_t());
    
  }
  
  // ORSA_DEBUG("about to start the integration...");

  /* 
     if (0) {
     // leave now...
     BodyGroup::TRV trv; // dummy
     trv.r = Vector(1e9,0,-1e11);
     //
     trv.t = orbitEpoch.getRef();
     bg->insertTRV(trv,vesta.get());
     //
     trv.t = orbitEpoch.getRef()+runDuration.getRef();
     bg->insertTRV(trv,vesta.get());
     
     emit progress(100);
     
     return;
     }
  */
  
  
  // test
  ORSA_DEBUG("========= DAWN time: %.6Ff",
	     dawn->getInitialConditions().time.getRef().asDouble().get_mpf_t());
  
  emit progress(0);
  //
  const bool goodIntegration = customIntegrator->integrate(bg.get(),
							   orbitEpoch.getRef(),
							   orbitEpoch.getRef()+runDuration.getRef(),
							   samplingPeriod);

  if (goodIntegration) {
    emit progress(100);
  }

  if (runDuration.getRef() < samplingPeriod) {

    ORSA_DEBUG("zero lenght integration, not writing SPICE file");

    ORSA_DEBUG("more checks here, and progress() code...");

  } else if (goodIntegration && (strlen(outputSPICEFile.getRef().c_str()) > 0)) {

    bool doSPICE = true;
    if (samplingPeriod > runDuration.getRef()) {
      ORSA_WARNING("integration too short, not writing SPICE output file");
      doSPICE = false;
    }

    // remove old file and write a new file...
    if (doSPICE) {

      {
	// remove the file, if existing, to prevent a SPICE error
	remove(outputSPICEFile.getRef().c_str());
      }

      SpiceInt handle;

      SPICE::instance()->lock();
      spkopn_c(outputSPICEFile.getRef().c_str(),
	       "DAWN SPK file",
	       1024,
	       &handle);
      SPICE::instance()->unlock();

      // ORSA_DEBUG("handle: %i",handle);

      Vector rDAWN,  vDAWN;
      Vector rVesta, vVesta;
      Vector dr, dv;
      Time t = orbitEpoch.getRef();
      unsigned int count=0;
      while ((t+samplingPeriod) <= (orbitEpoch.getRef()+runDuration.getRef())) {

	SpiceDouble first = SPICE::SPICETime(t);
	SpiceDouble  last = SPICE::SPICETime(t+samplingPeriod);

	SpiceInt   degree = 7; // degree must be odd for SPICE file type 13
	//
	SpiceInt        n = (degree+1)/2; // n is the number of states, must be at least (degree+1)/2

       	SpiceDouble states[n][6];
	SpiceDouble epochs[n];

	for (SpiceInt k=0; k<n; ++k) {

	  const Time localTime = t + (k*samplingPeriod)/(n-1);

	  // ORSA_DEBUG("localTime: %Ff",localTime.asDouble().get_mpf_t());

	  epochs[k] = SPICE::SPICETime(localTime);

	  /*
	     ORSA_DEBUG("epochs[%i] = %f",
	     k,
	     epochs[k]);
	  */

	  if (bg->getInterpolatedPosVel(rDAWN,
					vDAWN,
					dawn.get(),
					localTime) &&
	      bg->getInterpolatedPosVel(rVesta,
					vVesta,
					vesta.get(),
					localTime)) {

	    dr  = rDAWN-rVesta;
	    dv  = vDAWN-vVesta;

	    states[k][0] = FromUnits(dr.getX(),Unit::KM,-1).get_d();
	    states[k][1] = FromUnits(dr.getY(),Unit::KM,-1).get_d();
	    states[k][2] = FromUnits(dr.getZ(),Unit::KM,-1).get_d();
	    //
	    states[k][3] = FromUnits(FromUnits(dv.getX(),Unit::KM,-1),Unit::SECOND).get_d();
	    states[k][4] = FromUnits(FromUnits(dv.getY(),Unit::KM,-1),Unit::SECOND).get_d();
	    states[k][5] = FromUnits(FromUnits(dv.getZ(),Unit::KM,-1),Unit::SECOND).get_d();

	  } else {
	    ORSA_WARNING("problems, t: %Ff",
			 orsa::FromUnits(FromUnits(localTime.getMuSec(),
						   orsa::Unit::MICROSECOND),
					 orsa::Unit::DAY,-1).get_mpf_t());
	  }

	}

	char segmentID[1024];
	sprintf(segmentID,"segment.%i",count);

	SpiceInt body = -203;
	SpiceInt center = 2000004;

	SPICE::instance()->lock();
	spkw13_c(handle,
		 body,
		 center,
		 "ECLIPJ2000",
		 first,
		 last,
		 segmentID,
		 degree,
		 n,
		 states,
		 epochs);
	SPICE::instance()->unlock();

	t += samplingPeriod;
	++count;
      }

      SPICE::instance()->lock();
      spkcls_c(handle);
      SPICE::instance()->unlock();

    }

    // verify SPICE file?
    if (0 && doSPICE) {

      SPICE::instance()->loadKernel(outputSPICEFile.getRef().c_str());

      osg::ref_ptr<Body> dawnSPICE = new Body;
      {
	dawnSPICE->setName("DAWN");
	// dawnSPICE->setMass(zero());
	//
	if (accurateSPICE) {
	  SpiceBodyPosVelCallback * sbpvc = new SpiceBodyPosVelCallback(dawnSPICE->getName());
	  //
	  // sbpvc->setBodyName(dawnSPICE->getName());
	  // 
	  orsa::IBPS ibps;
	  ibps.translational = sbpvc;
	  dawnSPICE->setInitialConditions(ibps);
	} else {
	  /* 
	     SpiceBodyInterpolatedPosVelCallback * sbipvc =
	     new SpiceBodyInterpolatedPosVelCallback(dawnSPICE->getName(),
	     orbitEpoch.getRef(),
	     orbitEpoch.getRef()+runDuration.getRef(),
	     samplingPeriod);
	     dawnSPICE->setBodyPosVelCallback(sbipvc);
	  */
	}

	bg->addBody(dawnSPICE.get());
      }

      const int randomSeed = 3242234;

      const unsigned int maxTrials = 32;
      const orsa::Double minJulian = timeToJulian(orbitEpoch.getRef());
      const orsa::Double maxJulian = timeToJulian(orbitEpoch.getRef()+runDuration.getRef()).get_d();

      // GSL rng init
      gsl_rng * rnd = gsl_rng_alloc(gsl_rng_gfsr4);
      gsl_rng_set(rnd,randomSeed);

      Vector rDAWN,      vDAWN;
      Vector rDAWNSPICE, vDAWNSPICE;
      //
      Vector dr, dv;

      for (unsigned int j=0; j<maxTrials; ++j) {

	const Time t = julianToTime(minJulian + (maxJulian-minJulian)*gsl_rng_uniform(rnd));
	// const Time t = orbitEpoch.getRef();

	/*
	   ORSA_DEBUG("t: %Ff",
	   julianTime(t).get_mpf_t());
	*/

	if (bg->getInterpolatedPosVel(rDAWN,
				      vDAWN,
				      dawn.get(),
				      t) &&
	    bg->getInterpolatedPosVel(rDAWNSPICE,
				      vDAWNSPICE,
				      dawnSPICE.get(),
				      t)) {
	  dr  = rDAWN-rDAWNSPICE;
	  dv  = vDAWN-vDAWNSPICE;

	  /*
	     ORSA_DEBUG("dr: %Ff [km]",
	     FromUnits(dr.length(),Unit::KM,-1).get_mpf_t());
	     ORSA_DEBUG("dv: %Ff [km/s]",
	     FromUnits(FromUnits(dr.length(),Unit::KM,-1),Unit::SECOND).get_mpf_t());
	  */

	}

      }

      SPICE::instance()->unloadKernel(outputSPICEFile.getRef().c_str());

      // GSL rng clean
      gsl_rng_free(rnd);

    }

  }

  if (goodIntegration && (strlen(outputASCIIFile.getRef().c_str()) > 0)) {
    FILE * fp = fopen(outputASCIIFile.getRef().c_str(),"w");
    if (fp == 0) {
      ORSA_ERROR("cannot write file [%s]: %s",
		 outputASCIIFile.getRef().c_str(),
		 strerror(errno));
    } else {

      // print header
      {
	gmp_fprintf(fp,
		    "# Description of the columns:\n"
		    "# \n"
		    "# Col Units Description\n"
		    "# ---------------------\n"
		    "#  1  [day] Julian Date\n"
		    "#  2    [s] Simulation time\n"
		    "#  3   [km] DAWN orbit: semi-major axis\n"
		    "#  4  [---] DAWN orbit: eccentricity\n"
		    "#  5  [deg] DAWN orbit: inclination relative to Vesta's equatorial plane\n"
		    "#  6  [deg] Phase angle relative to DAWN's orbital plane\n"
		    "#  7  [deg] Phase angle relative to DAWN's position\n"
		    "#  8  [deg] DAWN's latitude relative to Vesta\n"
		    "#  9  [deg] DAWN's longitude relative to Vesta\n"
		    "# 10   [km] Center-to-center distance\n"
		    "# \n"
		    );
      }

      Vector rDAWN,  vDAWN;
      Vector rVesta, vVesta;
      Vector rSun,   vSun;
      //
      Vector dr, dv, dru, dvu, uPole;
      Orbit  orbit;
      Time t = orbitEpoch.getRef();
      while (t <= (orbitEpoch.getRef()+runDuration.getRef())) {

	if (bg->getInterpolatedPosVel(rDAWN,
				      vDAWN,
				      dawn.get(),
				      t) &&
	    bg->getInterpolatedPosVel(rVesta,
				      vVesta,
				      vesta.get(),
				      t) &&
	    bg->getInterpolatedPosVel(rSun,
				      vSun,
				      sun.get(),
				      t)) {
	  dr  = rDAWN-rVesta;
	  dv  = vDAWN-vVesta;
	  dru = dr.normalized();
	  dvu = dv.normalized();
	  // pole, in global coordinates, unitary vector
	  uPole = externalProduct(dru,dvu).normalized();
	  //
	  const Double orbitPhaseAngle = fabs(halfpi() - acos(uPole*(rSun-rVesta).normalized()));
	  const Double realPhaseAngle  = acos(dru*(rSun-rVesta).normalized());


	  {
	    // osg::ref_ptr<orsa::Attitude> vesta_attitude = new BodyAttitude(vesta.get(),bg.get());
	    
	    // if (vesta_attitude.get()) {
	    // const Matrix g2l = vesta_attitude->globalToLocal(t);
	    const orsa::Matrix g2l = orsa::globalToLocal(vesta.get(),bg.get(),t);
	    dr  =  g2l * dr;
	    dv  =  g2l * dv;
	    dru = (g2l * dru).normalized();
	    dvu = (g2l * dvu).normalized();
	  }
	  
	  orsa::Double vestaMass_t;
	  if (!bg->getInterpolatedMass(vestaMass_t,vesta.get(),t)) {
	    ORSA_DEBUG("problems...");
	  } 
	  
	  // ORSA_DEBUG("remember: globalToLocal!");
	  orbit.compute(dr,
			dv,
			orsa::Unit::instance()->getG() * vestaMass_t);
	  const Double latitude  = halfpi() - acos(dru.getZ());
	  const Double longitude = fmod(twopi() + atan2(dru.getY(),dru.getX()),twopi());
	  // if (realPhaseAngle < halfpi()) {
	  gmp_fprintf(fp,
		      "%15.5Ff %14.3Ff %12.3Ff %10.6Ff %10.6Ff %10.6Ff %10.6Ff %+10.6Ff %10.6Ff %12.3Ff\n",
		      timeToJulian(t).get_mpf_t(),
		      FromUnits((t-orbitEpoch.getRef()).asDouble(),Unit::SECOND,-1).get_mpf_t(),
		      FromUnits(orbit.a,Unit::KM,-1).get_mpf_t(),
		      orbit.e.get_mpf_t(),
		      Double(radToDeg()*orbit.i).get_mpf_t(),
		      Double(radToDeg()*orbitPhaseAngle).get_mpf_t(),
		      Double(radToDeg()*realPhaseAngle).get_mpf_t(),
		      Double(radToDeg()*latitude).get_mpf_t(),
		      Double(radToDeg()*longitude).get_mpf_t(),
		      FromUnits(dr.length(),Unit::KM,-1).get_mpf_t());
	  // }
	}
	t += samplingPeriod;
      }
      fclose(fp);
    }
  }

  // cannot unload kernels if I'm using OpenGL visualization
  /*
     SPICE::instance()->unloadKernel("de405.bsp");
     // SPICE::instance()->unloadKernel("vesta_1900_2100.bsp");
     SPICE::instance()->unloadKernel("vesta-2003-2013.bsp");
  */
}

void MainThread::abort() {
  customIntegrator->abort();
  // aborted = true;
}

void MainThread::reset() {

  vestaMass.reset();
  vestaMassDistribution.reset();
  vestaShapeModel.reset();
  vestaPeriod.reset();
  vestaPoleEclipticLatitude.reset();
  vestaPoleEclipticLongitude.reset();

  orbitEpoch.reset();
  orbitRadius.reset();
  orbitInclination.reset();
  orbitPhase.reset();

  runDuration.reset();
  outputSPICEFile.reset();
  outputASCIIFile.reset();

}