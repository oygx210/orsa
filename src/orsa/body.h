#ifndef _ORSA_BODY_
#define _ORSA_BODY_

#include <orsaTBB/malloc.h>

#include <osg/Referenced>
#include <osg/ref_ptr>

#include <orsa/cache.h>
#include <orsa/datetime.h>
#include <orsa/debug.h>
#include <orsa/paulMoment.h>
#include <orsa/propulsion.h>
#include <orsa/quaternion.h>
// #include <orsa/reference_frame.h>
#include <orsa/vector.h>
// #include <orsa/print.h>
#include <orsa/shape.h>

#include <string>

namespace orsa {
  
  class Body;
  class BodyInitialConditions;
  
  /***/
  
  class BodyProperty : public osg::Referenced {
  public:
    BodyProperty() : osg::Referenced(true) { }
  protected:
    virtual ~BodyProperty() { }
  public:
    //! BP_CONSTANT is not a function of time.
    //! BP_PRECOMPUTED is a funcion of time, but is already computed, useful for callbacks.
    //! BP_DYNAMIC is a function of time, and needs to be computed.
    enum BodyPropertyType {
      BP_CONSTANT,   
      BP_PRECOMPUTED, 
      BP_DYNAMIC   
    };
  public:
    virtual BodyPropertyType type() const = 0;
  public:
    virtual bool dynamic() const { return (type() == BP_DYNAMIC); }
  public:	
    /* update() should be implemented by all properties of type BP_PRECOMPUTED */
    virtual bool update(const orsa::Time &) = 0; 
  };
  
  /***/
  
  /* 
   * centerOfMass is in the coordinates used for the shape
   * shapeToLocal is the rotation matrix from the shape system to the diagonal system ( = "local")
   * inertialMatrix is diagonal if shapeToLocal and localToShape are correct
   * "local" is the principal axis system
   * paulMoment is about the centerOfMass and in the principalAxis system
   *
   * usually, to compute these vars, a mass distribution is assumed,
   * or the data is otherwise obtained experimentally
   *
   * Again: "principal" == "diagonal" == "local" ref. sys.
   *	
   */
  class InertialBodyProperty : public BodyProperty {
  public:
    virtual double mass() const = 0;
    virtual const orsa::Shape * shape() const = 0;
    virtual orsa::Vector centerOfMass() const = 0;
    virtual orsa::Matrix shapeToLocal() const = 0;
    virtual orsa::Matrix localToShape() const = 0;
    virtual orsa::Matrix inertiaMatrix() const = 0;
    virtual const orsa::PaulMoment * paulMoment() const = 0;
  public:
    virtual bool setMass(const double &) = 0;
    virtual bool setShape(const orsa::Shape *) = 0;
    virtual bool setCenterOfMass(const orsa::Vector &) = 0;
    virtual bool setShapeToLocal(const orsa::Matrix &) = 0;
    virtual bool setLocalToShape(const orsa::Matrix &) = 0;
    virtual bool setInertiaMatrix(const orsa::Matrix &) = 0;
    virtual bool setPaulMoment(const orsa::PaulMoment *) = 0;
  public:
    virtual InertialBodyProperty * clone() const = 0;
  };
  
  class ConstantInertialBodyProperty : public InertialBodyProperty {
  public:
    ConstantInertialBodyProperty(const double & m) :
      InertialBodyProperty(), 
      _m(m),
      _s(0),
      _cm(orsa::Vector(0,0,0)),
      _s2l(orsa::Matrix::identity()),
      _l2s(orsa::Matrix::identity()),
      _I(orsa::Matrix::identity()),
      _pm(0) { }
  public:
    ConstantInertialBodyProperty(const double           & m,
				 const orsa::Shape      * s,
				 const orsa::Vector     & cm,
				 const orsa::Matrix     & s2l,
				 const orsa::Matrix     & l2s,
				 const orsa::Matrix     & I,
				 const orsa::PaulMoment * pm) :
      InertialBodyProperty(), 
      _m(m),
      _s(s),
      _cm(cm),
      _s2l(s2l),
      _l2s(l2s),
      _I(I),
      _pm(pm) { }
  protected:
    const double _m;
    osg::ref_ptr<const orsa::Shape> _s;
    const orsa::Vector _cm;
    const orsa::Matrix _s2l;
    const orsa::Matrix _l2s;
    const orsa::Matrix _I;
    osg::ref_ptr<const orsa::PaulMoment> _pm;
  public:
    double mass() const { return _m; }
    const orsa::Shape * shape() const { return _s.get(); }
    orsa::Vector centerOfMass() const { return _cm; }
    orsa::Matrix shapeToLocal() const { return _s2l; }
    orsa::Matrix localToShape() const { return _l2s; }
    orsa::Matrix inertiaMatrix() const { return _I; }
    const orsa::PaulMoment * paulMoment() const { return _pm.get(); }
  public:
    bool setMass(const double &) {
      ORSA_ERROR("this method should not have been called, please check your code.");
      return false;
    }
    bool setShape(const orsa::Shape *) {
      ORSA_ERROR("this method should not have been called, please check your code.");
      return false;
    }
    bool setCenterOfMass(const orsa::Vector &) {
      ORSA_ERROR("this method should not have been called, please check your code.");
      return false;
    }
    bool setShapeToLocal(const orsa::Matrix &) {
      ORSA_ERROR("this method should not have been called, please check your code.");
      return false;
    }
    bool setLocalToShape(const orsa::Matrix &) {
      ORSA_ERROR("this method should not have been called, please check your code.");
      return false;
    }
    bool setInertiaMatrix(const orsa::Matrix &) {
      ORSA_ERROR("this method should not have been called, please check your code.");
      return false;
    }
    bool setPaulMoment(const orsa::PaulMoment *) {
      ORSA_ERROR("this method should not have been called, please check your code.");
      return false;
    }
  public:
    InertialBodyProperty * clone() const {
      return new ConstantInertialBodyProperty(*this);
    }
  public:
    BodyPropertyType type() const { return BP_CONSTANT; }
  public:
    bool update(const orsa::Time &) { return true; }
  };
  

  // two notes: 1) is this used anywhere? 2) is clone() needed?
  /* class PrecomputedInertialBodyProperty : public InertialBodyProperty {
     public:
     BodyPropertyType type() const { return BP_PRECOMPUTED; }
     public:
     bool setMass(const double &) {
     ORSA_ERROR("this method should not have been called, please check your code.");
     return false;
     }
     };
  */
  
  
  // used anywhere yet?
  /* class DynamicInertialBodyProperty : public InertialBodyProperty {
     public:
     BodyPropertyType type() const { return BP_DYNAMIC; }
     public:
     bool update(const orsa::Time &) { return true; }
     public:
     double mass() const {
     return _mass.getRef();
     }
     public:
     bool setMass(const double & m) {
     _mass = m;
     return true;
     }
     public:
     InertialBodyProperty * clone() const {
     return new DynamicInertialBodyProperty(*this);
     }
     protected:
     orsa::Cache<double> _mass;
     };
  */
  
  /***/

  class TranslationalBodyProperty : public BodyProperty {
  public:
    virtual orsa::Vector position() const = 0;
    virtual orsa::Vector velocity() const = 0;
  public:
    virtual bool setPosition(const orsa::Vector &) = 0;
    virtual bool setVelocity(const orsa::Vector &) = 0;
  public:
    virtual TranslationalBodyProperty * clone() const = 0;
  };
  
  class ConstantTranslationalBodyProperty : public TranslationalBodyProperty {
  public:
    BodyPropertyType type() const { return BP_CONSTANT; }
  public:
    bool update(const orsa::Time &) { return true; }
  };
  
  class PrecomputedTranslationalBodyProperty : public TranslationalBodyProperty {
  public:
    BodyPropertyType type() const { return BP_PRECOMPUTED; }
  public:
    bool setPosition(const orsa::Vector &) {
      ORSA_ERROR("this method should not have been called, please check your code.");
      return false;
    }
  public:
    bool setVelocity(const orsa::Vector &) {
      ORSA_ERROR("this method should not have been called, please check your code.");
      return false;
    }
  };
  
  class DynamicTranslationalBodyProperty : public TranslationalBodyProperty {
  public:
    BodyPropertyType type() const { return BP_DYNAMIC; }
  public:
    bool update(const orsa::Time &) { return true; }
  public:
    orsa::Vector position() const { 
      // ORSA_DEBUG("_position: %.20e",_position.getRef().length());
      // orsa::print(_position.getRef());
      return _position.getRef();
    }
  public:
    orsa::Vector velocity() const {
      // ORSA_DEBUG("_velocity: %.20e",_velocity.getRef().length());
      // orsa::print(_velocity.getRef());
      return _velocity.getRef();
    }
  public:
    bool setPosition(const orsa::Vector & r) { 
      _position = r;
      // ORSA_DEBUG("_position: %.20e",_position.getRef().length());
      // orsa::print(_position.getRef());
      return true;
    }
  public: 
    bool setVelocity(const orsa::Vector & v) {
      _velocity = v;
      // ORSA_DEBUG("_velocity: %.20e",_velocity.getRef().length());
      // orsa::print(_velocity.getRef());
      return true;
    }
  public:
    TranslationalBodyProperty * clone() const {
      return new DynamicTranslationalBodyProperty(*this);
    }
  protected:
    orsa::Cache<orsa::Vector> _position;
    orsa::Cache<orsa::Vector> _velocity;
  };
  
  /***/
  
  class RotationalBodyProperty : public BodyProperty {
  public:
    virtual bool get(orsa::Quaternion & q,
		     orsa::Vector     & omega) const = 0;
  public:
    virtual orsa::Quaternion getQ()    const = 0;
    virtual orsa::Vector     getOmega() const = 0;
    
  public:
    /* 
       orsa::Quaternion qRotated(const orsa::Vector & omega,
       const orsa::Time   & dt) const;
    */
    
  public:
    virtual bool set(const orsa::Quaternion & q,
		     const orsa::Vector     & omega) = 0;
  public:
    virtual RotationalBodyProperty * clone() const = 0;
  
  public:
    static orsa::Quaternion qDot (const orsa::Quaternion & q,
				  const orsa::Vector     & omega);
    
    static orsa::Quaternion qDotDot (const orsa::Quaternion & q,
				     const orsa::Vector     & omega,
				     const orsa::Vector     & omegaDot);
    
    static orsa::Vector omega (const orsa::Quaternion & q,
			       const orsa::Quaternion & qDot);
    
    static orsa::Vector omegaDot (const orsa::Quaternion & q,
				  const orsa::Quaternion & qDot,
				  const orsa::Quaternion & qDotDot);
    
    static orsa::Quaternion qFiniteRotation (const orsa::Quaternion & q,
					     const orsa::Vector     & omega,
					     const orsa::Time       & dt);
    
    static orsa::Vector newOmega (const orsa::Vector & omega,
				  const orsa::Vector & omegaDot,
				  const orsa::Time   & dt);
  };
  
  class ConstantRotationalBodyProperty : public RotationalBodyProperty {
  public:
    BodyPropertyType type() const { return BP_CONSTANT; }
  public:
    bool update(const orsa::Time &) { return true; }
  };
  
  class PrecomputedRotationalBodyProperty : public RotationalBodyProperty {
  public:
    BodyPropertyType type() const { return BP_PRECOMPUTED; }
  public:
    virtual bool set(const orsa::Quaternion &,
		     const orsa::Vector     &) {
      ORSA_ERROR("this method should not have been called, please check your code.");
      return false;
    }
  };
  
  class DynamicRotationalBodyProperty : public RotationalBodyProperty {
  public:
    BodyPropertyType type() const { return BP_DYNAMIC; }
  public:
    bool update(const orsa::Time &) { return true; }
  public:
    bool get(orsa::Quaternion & q,
	     orsa::Vector     & omega) const {
      q     = _q.getRef();
      omega = _omega.getRef();
      return true;
    }
  public:
    orsa::Quaternion getQ()     const { return _q.getRef(); }
    orsa::Vector     getOmega() const { return _omega.getRef(); }
  public:
    bool set(const orsa::Quaternion & q,
	     const orsa::Vector     & omega) {
      _q     = q;
      _omega = omega;
      // ORSA_DEBUG("omega: %Fg",_omega.getRef().length());
      return true;
    }
  public:
    RotationalBodyProperty * clone() const {
      return new DynamicRotationalBodyProperty(*this);
    }
  protected:
    orsa::Cache<orsa::Quaternion> _q;
    orsa::Cache<orsa::Vector>     _omega;
  };
  
  /***/
  
  class IBPS {
  public:
    IBPS();
  public:
    IBPS(const IBPS &);
  public:
    virtual ~IBPS();
  public:
    orsa::Cache<orsa::Time> time;
  public:
    osg::ref_ptr<InertialBodyProperty> inertial;
  public:
    osg::ref_ptr<TranslationalBodyProperty> translational;
  public:
    osg::ref_ptr<RotationalBodyProperty> rotational;
    
  public:
    const IBPS & operator = (const IBPS &);
    
  public:
    virtual bool update(const orsa::Time & t) {
      if (inertial.get()) {
	inertial->update(t);
      }
      if (translational.get()) {
	translational->update(t);
      }
      if (rotational.get()) {
	rotational->update(t);
      }
      return true;
    }
    
  public:
    bool dynamic() const {
      if (inertial.get()) {
	if (inertial->dynamic()) return true;
      }
      if (translational.get()) {
	if (translational->dynamic()) return true;
      }
      if (rotational.get()) {
	if (rotational->dynamic()) return true;
      }
      return false;
    }
    
  public:
    bool tmp;
    
    // time ordering, needed when used in orsa::Interval<>
  public:
    inline bool operator == (const IBPS & rhs) const {
      return (time.getRef() == rhs.time.getRef());
    }
  public:
    inline bool operator != (const IBPS & rhs) const {
      return (time.getRef() != rhs.time.getRef());
    }
  public:
    inline bool operator < (const IBPS & rhs) const {
      return (time.getRef() < rhs.time.getRef());
    }
  public:
    inline bool operator > (const IBPS & rhs) const {
      return (time.getRef() > rhs.time.getRef());
    }
  public:
    inline bool operator <= (const IBPS & rhs) const {
      return (time.getRef() <= rhs.time.getRef());
    }
  public:
    inline bool operator >= (const IBPS & rhs) const {
      return (time.getRef() >= rhs.time.getRef());
    }
  };
  
  typedef orsa::IBPS InstantaneousBodyPropertySet;
  
  /***/
  
  class Body : public osg::Referenced {
  public:	
    Body();
  private:
    void _init();
  protected:
    virtual ~Body();
    
  public:
    typedef unsigned int BodyID;
    BodyID id() const { return _id; }
  private:
    static BodyID bodyID_counter;
    const BodyID _id;
    
    /* 
       public:
       virtual bool setMass(const double & mass) {
       // ORSA_DEBUG("called setMass(%Fg) [***]",mass());
       return (_mass.set(mass) && _mu.set(Unit::instance()->getG()*mass));
       }
       public:
       virtual bool setMu(const double & mu) {
       // ORSA_DEBUG("called setMu(%Fg) [***]",mu());
       return (_mass.set(mu/Unit::instance()->getG()) && _mu.set(mu));
       }
       public:
       virtual const double & getMass() const {
       if (_mass.isSet()) {
       return _mass.getRef();
       } else {
       ORSA_ERROR("mass has never been set for this Body.");
       }
       return _mass.getRef();
       }
       public:
       virtual const double & getMu() const {
       if (_mu.isSet()) {
       return _mu.getRef();
       } else {
       ORSA_ERROR("mu has never been set for this Body.");
       }
       return _mu.getRef(); 
       }
       protected:
       orsa::Cache<double> _mass;
       orsa::Cache<double> _mu;
    */
    
  public:
    bool setInitialConditions(const orsa::IBPS & ibps) {
      // these checks do not completely enforce presence of inertial and translational components, but help indeed achieve this goal.
      if (ibps.inertial.get() == 0) {
	ORSA_DEBUG("ibps.inertial must be set.");
	return false;
      }	 
      if (ibps.translational.get() == 0) {
	ORSA_DEBUG("ibps.translational must be set.");
	return false;
      }	
      _ibps = ibps;
      return true;
    }
  public:
    const orsa::IBPS & getInitialConditions() const {
      return _ibps.getRef();
    }
  protected:
    orsa::Cache<orsa::IBPS> _ibps;
    
    /* 
       public:
       bool setRadius(const double & r) {
       if (_shape.get()) {
       ORSA_ERROR("cannot set radius, this body has a predefined shape");
       return false;
       } else {
       _radius = r;
       return true;
       }
       }
       public:	
       const double getRadius() const {
       if (_shape.get()) {
       return _shape->boundingRadius();
       } else if (_radius.isSet()) {
       return _radius.getRef();
       } else {
       return 0;
       }
       }
       protected:
       orsa::Cache<double> _radius;
    */
    
  public:
    virtual bool setName(const std::string &);
    virtual const std::string & getName() const;
  protected:
    std::string _name;
    
    /* 
       public:
       inline virtual bool setShape(const orsa::Shape * s) {
       _shape = s;
       return true;
       }
       public:
       inline virtual const orsa::Shape * getShape() const {
       return _shape.get();
       }
       protected:
       osg::ref_ptr<const orsa::Shape> _shape;
    */
    
    /* 
       public:
       inline virtual bool setPaulMoment(const orsa::PaulMoment * m) {
       _paulMoment = m;
       return true;
       }
       public:
       inline virtual const orsa::PaulMoment * getPaulMoment() const {
       return _paulMoment.get();
       }
       protected:
       osg::ref_ptr<const orsa::PaulMoment> _paulMoment;
    */
    
    /* 
       public:
       inline virtual bool setPropulsion(const orsa::Propulsion * p) {
       _propulsion = p;
       return true;
       }
       public:
       inline virtual const orsa::Propulsion * getPropulsion() const {
       return _propulsion.get();
       }
       protected:
    */
  public:
    osg::ref_ptr<const orsa::Propulsion> propulsion;
    
    // comet particle beta
  public:
    orsa::Cache<double>      beta;
    osg::ref_ptr<const orsa::Body> betaSun;
    
    // bio
  public:
    mutable orsa::Cache<orsa::Time> birthTime;
    mutable orsa::Cache<orsa::Time> deathTime;
  public:
    bool alive(const orsa::Time & t) const {
      if (birthTime.isSet()) {
	if (t < birthTime.getRef()) return false;
      }
      if (deathTime.isSet()) {
	if (t > deathTime.getRef()) return false;
      }
      return true;
    }
    
    // light source
    // should add a 'graphics' entry to IBPS, for this and other related entries
  public:
    orsa::Cache<bool> isLightSource;
    
  public:
    //! all the bodies with the same value of nonInteractingGroupID do not interact with each other
    //! example: particles with small mass, that don't see each other, but that all interact with sun and planets
    // orsa::Cache<unsigned int> nonInteractingGroupID;
    //! simpler (and faster): one single group for all non-interacting particles (cannot think of any case where more than one group is really needed)
    //! defaults to false: iteracts with all other bodies
    bool nonInteractingGroup;
    
  };
  
} // namespace orsa

#endif // _ORSA_BODY_
