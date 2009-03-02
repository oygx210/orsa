#include <orsa/double.h>
#include <orsa/cache.h>

#include <vector>

using namespace orsa;

/* 
   mpz_class orsa::factorial(const mpz_class & i) {
   if (i <= 1) {
   return 1;
   }
   return (i*factorial(i-1));
   }
*/

mpz_class orsa::factorial(const mpz_class & i) {
  // ORSA_DEBUG("f: %Zi",i.get_mpz_t());
  static std::vector< orsa::Cache<mpz_class> > _factorial_table;
  const unsigned long int index = i.get_ui();
  static mpz_class _mpz_one("1");
  if (i <= _mpz_one) {
    return _mpz_one;
  } else if (_factorial_table.size() > i) {
    if (!_factorial_table[index].isSet()) {
      _factorial_table[index].set(i*factorial(i-_mpz_one));
    }
    return _factorial_table[index].get();
  } else {
    _factorial_table.resize(index+1);
    _factorial_table[index].set(i*factorial(i-_mpz_one));
    return _factorial_table[index].get();
  }
}

/* 
   mpz_class orsa::bi_factorial(const mpz_class & i) {
   ORSA_DEBUG("bf: %Zi",i.get_mpz_t());
   if (i <= mpz_class("1")) {
   return mpz_class("1");
   }
   return (i*bi_factorial(i-mpz_class("2")));
   }
*/

mpz_class orsa::bi_factorial(const mpz_class & i) {
  // ORSA_DEBUG("bf: %Zi",i.get_mpz_t());
  static std::vector< orsa::Cache<mpz_class> > _bi_factorial_table;
  const unsigned long int index = i.get_ui();
  static mpz_class _mpz_one("1");
  static mpz_class _mpz_two("2");
  if (i <= _mpz_one) {
    return _mpz_one;
  } else if (_bi_factorial_table.size() > i) {
    if (!_bi_factorial_table[index].isSet()) {
      _bi_factorial_table[index].set(i*bi_factorial(i-_mpz_two));
    }
    return _bi_factorial_table[index].get();
  } else {
    _bi_factorial_table.resize(index+1);
    _bi_factorial_table[index].set(i*bi_factorial(i-_mpz_two));
    return _bi_factorial_table[index].get();
  }
}

mpz_class orsa::binomial(const mpz_class & n, const mpz_class & k) {
  const mpz_class retVal = ( (factorial(n)) / 
			     (factorial(k)*factorial(n-k)) );

  /* 
     ORSA_DEBUG("binomial(%Zi,%Zi) = %Zi",
     n.get_mpz_t(),
     k.get_mpz_t(),
     retVal.get_mpz_t());
  */
  
  return retVal;
}

int orsa::power_sign(const mpz_class & l) {
  if ((l%2)==1) {
    return -1;
  } else {
    return  1;
  }
}

double orsa::int_pow(const double    & x, 
		     const mpz_class & p) {
  // ORSA_DEBUG("int_pow(%f,%Zi)",x(),p.get_mpz_t());
  if (p ==  2) return x*x;
  if (p ==  1) return x;
  if (p ==  0) return 1;
  if (p == -1) return 1/x;
  if (fabs(x) < epsilon()) {
    return 0;
  }
  double _pow = x;
  const mpz_class max_k = abs(p);
  for (mpz_class k=1; k < max_k; ++k) {
    _pow *= x;
  }
  if (p < 0) _pow = 1/_pow;
  return _pow;
}

/* 
   const double & orsa::epsilon() {
   // ORSA_DEBUG("epsilon power: %Zi",mpz_class(-0.28125*mpf_get_default_prec()).get_mpz_t());
   // approx 15 digits (base 10) every 64 bits... is this correct?
   // return int_pow(double("10.0"),mpz_class(-15.0*(mpf_get_default_prec()/64.0))); 
   // ...or a bit better: 16 digits for 64 bits:
   // return int_pow(double("10.0"),mpz_class(-(int)mpf_get_default_prec()/4)); 
   // ...or more... (0.3125 = 1/3.2 -> 20 digits per 64 bits)
   // return int_pow(double("10.0"),mpz_class(-0.3125*mpf_get_default_prec())); 
   // ...or simply 0.3, which seems to be the one working better...
   // return 
   // int_pow(double("10.0"),mpz_class(-0.3*mpf_get_default_prec())); 
   // ... OK, 0.28125 = 18/64 works great.
   static double _eps;
   static unsigned int old_prec = 0;
   if (old_prec != mpf_get_default_prec()) {
   _eps = int_pow(double("10.0"),mpz_class(-0.28125*mpf_get_default_prec()),false); 
   old_prec = mpf_get_default_prec();
   } 
   return _eps;
   }
*/

const double & orsa::epsilon() {
  static double _eps = __DBL_EPSILON__; /* 2.2204460492503131e-16 */
  return _eps;
}

const double & orsa::pi() {
  static double _pi = 3.14159265358979323846;
  return _pi;
}

const double & orsa::halfpi() {
  static double _halfpi = 0.5*orsa::pi();
  return _halfpi;
}

const double & orsa::twopi() {
  static double _twopi = 2*orsa::pi();
  return _twopi;
}

const double & orsa::pisquared() {
  static double _pisquared = orsa::pi()*orsa::pi();
  return _pisquared;
}

const double & orsa::radToDeg() {
  static double _radToDeg = 180/orsa::pi();
  return _radToDeg;
}

const double & orsa::degToRad() {
  static double _degToRad = orsa::pi()/180;
  return _degToRad;
}

const double & orsa::radToArcmin() {
  static double _radToArcmin = 180*60/orsa::pi();
  return _radToArcmin;
}

const double & orsa::arcminToRad() {
  static double _arcminToRad = orsa::pi()/(180*60);
  return _arcminToRad;
}

const double & orsa::radToArcsec() {
  static double _radToArcsec = 180*3600/orsa::pi();
  return _radToArcsec;
}

const double & orsa::arcsecToRad() {
  static double _arcsecToRad = orsa::pi()/(180*3600);
  return _arcsecToRad;
}

int orsa::kronecker(const mpz_class & i,
		    const mpz_class & j) {
  if (i==j) {
    return 1;
  } else {
    return 0;
  }
}

double orsa::pochhammer(const double & a, const mpz_class & n) {
  if (n == 0) {
    return 1;
  }
  if (n == 1) {
    return a;
  }
  double _result = 1;
  mpz_class _k = 0;
  if (n > 0) {
    do { 
      // ORSA_DEBUG("PH+: k: %Zi n: %Zi",_k.get_mpz_t(),n.get_mpz_t());
      _result *= (a+_k.get_d());
      ++_k;
    } while (_k != n);
  } else {
    do { 
      // ORSA_DEBUG("PH-: k: %Zi n: %Zi",_k.get_mpz_t(),n.get_mpz_t());
      _result *= (a+_k.get_d());
      --_k;
    } while (_k != n);
  }
  return _result;
}