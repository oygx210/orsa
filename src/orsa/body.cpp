#include <orsa/body.h>

#include <orsa/print.h>
#include <orsa/util.h>

using namespace orsa;

// IBPS 

IBPS::IBPS() {
  
  // ORSA_DEBUG("creating new IBPS, address: %x",this);
  
  tmp = false;
}

IBPS::IBPS(const IBPS & ibps) {
  
  /* ORSA_DEBUG("creating IBPS, address: %x   copy of address: %x",this,&ibps);
     ORSA_DEBUG("%x.translational.get(): %x   %x.translational.get(): %x",
     &ibps,ibps.translational.get(),
     this,translational.get());
  */
  
  if (ibps.time.isSet()) time = ibps.time.getRef();
  /* 
     if (ibps.dynamic()) {
     // time = ibps.time.getRef();
     // ORSA_DEBUG("copy constructor, time: %.6f",time.getRef().get_d());
     }  
  */
  if (ibps.inertial.get()) {
    inertial = ibps.inertial->clone();
  } else {
    inertial = 0;
  }
  if (ibps.translational.get()) {
    translational = ibps.translational->clone();
  } else {
    translational = 0;
  }
  if (ibps.rotational.get()) {
    rotational = ibps.rotational->clone();
  } else {
    rotational = 0;
  }
  tmp = ibps.tmp;
  
  /* ORSA_DEBUG(" --LEAVING-- %x.translational.get(): %x   %x.translational.get(): %x",
     &ibps,ibps.translational.get(),
     this,translational.get());
  */
}

IBPS::~IBPS() { 
  /* ORSA_DEBUG("destroying IBPS address: %x   %x.translational.get(): %x",
     this,
     this,translational.get());
  */
}

const IBPS & IBPS::operator = (const IBPS & ibps) {
  
  /* 
     ORSA_DEBUG("copying IBPS, from address: %x   to address %x",&ibps,this);
     ORSA_DEBUG("%x.translational.get(): %x   %x.translational.get(): %x",
     &ibps,ibps.translational.get(),
     this,translational.get());
  */
  
  if (ibps.time.isSet()) time = ibps.time.getRef();
  /* 
     if (ibps.dynamic()) {
     // time = ibps.time.getRef();
     // ORSA_DEBUG("copy operator, time: %.6f",time.getRef().get_d());
     } 
  */
  if (ibps.inertial.get()) {
    inertial = ibps.inertial->clone();
  } else {
    inertial = 0;
  }
  if (ibps.translational.get()) {
    translational = ibps.translational->clone();
  } else {
    translational = 0;
  }
  if (ibps.rotational.get()) {
    rotational = ibps.rotational->clone();
  } else {
    rotational = 0;
  }
  tmp = ibps.tmp;
  
  /* ORSA_DEBUG(" --LEAVING-- %x.translational.get(): %x   %x.translational.get(): %x",
     &ibps,ibps.translational.get(),
     this,translational.get());
  */
  
  return (*this);
}

//

orsa::Quaternion RotationalBodyProperty::qDot (const orsa::Quaternion & q,
					       const orsa::Vector     & omega) {
  return (orsa::Quaternion(omega) * q);
}

orsa::Quaternion RotationalBodyProperty::qDotDot (const orsa::Quaternion & q,
						  const orsa::Vector     & omega,
						  const orsa::Vector     & omegaDot) {
  return (orsa::Quaternion(omegaDot) * q + orsa::Quaternion(omega) * RotationalBodyProperty::qDot(q,omega));
}

orsa::Vector RotationalBodyProperty::omega (const orsa::Quaternion & q,
					    const orsa::Quaternion & qDot) {
  return (qDot * inverse(q)).getVector();
}

orsa::Vector RotationalBodyProperty::omegaDot (const orsa::Quaternion & q,
					       const orsa::Quaternion & qDot,
					       const orsa::Quaternion & qDotDot) {
  return ((qDotDot - orsa::Quaternion(RotationalBodyProperty::omega(q,qDot)) * qDot) * inverse(q)).getVector();
}

orsa::Quaternion RotationalBodyProperty::qFiniteRotation (const orsa::Quaternion & q,
							  const orsa::Vector     & omega,
							  const orsa::Time       & dt) {
  const double angle = omega.length() * dt.get_d() / 2;
  return unitQuaternion(Quaternion(cos(angle),sin(angle)*(omega.normalized()))*q);
}

orsa::Vector RotationalBodyProperty::newOmega (const orsa::Vector & omega,
					       const orsa::Vector & omegaDot,
					       const orsa::Time   & dt) {
  
  /* 
     ORSA_DEBUG("---prints---");
     print(omegaDot);
     print(dt);
  */
  
  if (omegaDot.length() < orsa::epsilon()) {
    ORSA_DEBUG("omegaDot is really tiny...");
    return (omega + omegaDot * dt.get_d());
  }
  
  // test
  // return orsa::Vector(omega + omegaDot * dt.get_d());
  
  const double  omegaMagnitude = omega.length();
  const orsa::Vector uOmega          = omega.normalized();

  const double  omegaDotMagnitude = omegaDot.length();
  const orsa::Vector uOmegaDot          = omegaDot.normalized();
  
  orsa::Vector newOmega;
  
  if (omegaMagnitude > orsa::epsilon()) {
    
    const double radialComponent = omegaDot * uOmega;
    
    ORSA_DEBUG("radialComponent/omegaDotMagnitude: %g",radialComponent/omegaDotMagnitude);
    
    const orsa::Vector  tangentOmegaDot          = omegaDot - radialComponent * uOmega;
    const double  tangentOmegaDotMagnitude = tangentOmegaDot.length();
    const orsa::Vector uTangentOmegaDot          = tangentOmegaDot.normalized();
    
    const orsa::Vector uRotationAxis = (orsa::externalProduct(uOmega,uTangentOmegaDot)).normalized();
    const double rotationAngle  = tangentOmegaDotMagnitude * dt.get_d() / omegaMagnitude;
    
    // rotate old omega direction around uRotationAxis of angle rotationAngle
    const double     qAngle = rotationAngle / 2; // factor 2 due to the quaternion angle definition
    const orsa::Quaternion qRot   = unitQuaternion(Quaternion(cos(qAngle),sin(qAngle)*uRotationAxis));
    
    newOmega = (radialComponent * dt.get_d() + omegaMagnitude) * (qRot*uOmega*conjugate(qRot)).getVector().normalized();
    //
    /* 
       newOmega = 
       radialComponent * dt.get_d() * uOmega +
       omegaMagnitude * (qRot*uOmega*conjugate(qRot)).getVector().normalized();
    */
    
  } else {
    
    newOmega = omega + omegaDot * dt.get_d();
  }
  
  // ORSA_DEBUG("omega comparison...");
  // print(omega);
  print(newOmega);
  
  ORSA_DEBUG("scalar product: %20.16f",
	     newOmega.normalized() * omega.normalized());
  
  return newOmega;  
}

// Body

Body::BodyID Body::bodyID_counter = 0;

Body::Body() : osg::Referenced(true), _id(bodyID_counter++) {
  _init();
  // ORSA_DEBUG("created body id: %i",id());
}

/* 
   Body::Body(const Body &, const CopyOption) : osg::Referenced() {
   _init();
   // more code here...
   }
*/

void Body::_init() {
  // _validBodyType = false;
  /* 
     _validBirthTime = _validDeathTime = false;
  */
  
  // _ibps      = 0;
  // _attitude  = 0;
  _shape     = 0;
  // _multipole = 0;
  // _bpvc      = 0;
  // _bac       = 0;
  
  // comet particle beta
  beta    = 0;
  betaSun = 0;
  
  // light source
  isLightSource = false;
  
  nonInteractingGroup = false;
}

Body::~Body() {
  
}

/* 
   bool Body::setBirthTime(const orsa::Time & t) {
   if (!_validBirthTime) {
   _birthTime = t;
   _validBirthTime = true;
   } else {
   // error...
   }
   return true;
   }
   
   bool Body::validBirthTime() const {
   return _validBirthTime;
   }
   
   const orsa::Time & Body::getBirthTime() const {
   if (!_validBirthTime) {
   // error...
   }
   return _birthTime;
   }
   
   bool Body::setDeathTime(const orsa::Time & t) {
   if (!_validDeathTime) {
   _deathTime = t;
   _validDeathTime = true;
   } else {
   // error...
   }
   return true;
   }
   
   bool Body::validDeathTime() const {
   return _validDeathTime;
   }
   
   const orsa::Time & Body::getDeathTime() const {
   if (!_validDeathTime) {
   // error...
   }
   return _deathTime;
   }
   
   bool Body::isActive(const orsa::Time &) {
   // code needed here!
   return true;
   }
*/

/* 
   bool Body::setMass(const double m) {
   _mass = m;
   return true;
   }
   
   double Body::getMass() const {
   return _mass;
   }
*/

bool Body::setName(const std::string & s) {
  _name = s;
  return true;
}

const std::string & Body::getName() const {
  return _name;
}