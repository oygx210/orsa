#ifndef _ORSA_MATRIX_
#define _ORSA_MATRIX_

#include <orsa/cache.h>
#include <orsa/debug.h>
#include <orsa/double.h>
#include <orsa/vector.h>

#include <osg/Matrixd>

namespace orsa {
  
  class Matrix {
  public:
    Matrix();
    Matrix(const Matrix &);
    Matrix(const double m11, 
	   const double m12,
	   const double m13,
	   const double m21,
	   const double m22,
	   const double m23,
	   const double m31,
	   const double m32,
	   const double m33);
    
  public:
    void set(const double m11, 
	     const double m12,
	     const double m13,
	     const double m21,
	     const double m22,
	     const double m23,
	     const double m31,
	     const double m32,
	     const double m33); 

    void get(double & m11, 
	     double & m12,
	     double & m13,
	     double & m21,
	     double & m22,
	     double & m23,
	     double & m31,
	     double & m32,
	     double & m33) const;

  public:
    const double & getM11() const { return m11; }
    const double & getM12() const { return m12; }
    const double & getM13() const { return m13; }
    const double & getM21() const { return m21; }
    const double & getM22() const { return m22; }
    const double & getM23() const { return m23; }
    const double & getM31() const { return m31; }
    const double & getM32() const { return m32; }
    const double & getM33() const { return m33; }
    
  public:
    osg::Matrixd getMatrixd() const;
    
  public:
    double determinant() const;
 
  public:
    static Matrix identity();
    
  public:
    static bool invert(const Matrix & src, Matrix & inverse);
    static void transpose(const Matrix & src, Matrix & transposed);
    
  public:
    static void OpenGLMatrix(const Matrix & src, double opengl_matrix[16]);
    
    // operators
    Matrix & operator += (const Matrix &);
    Matrix & operator -= (const Matrix &);
    Matrix & operator *= (const double &);
    Matrix & operator /= (const double &);
    
    // sign
    Matrix operator + () const;
    Matrix operator - () const;
    
  public:
    Matrix rotX(const double & alpha);
    Matrix rotY(const double & alpha);
    Matrix rotZ(const double & alpha);
    
  public:
    static Matrix axisRotation(const Vector & axis, const double & alpha);
    
  public:
    Matrix operator + (const Matrix &) const;
    Matrix operator - (const Matrix &) const;
    Matrix operator * (const Matrix &) const;
    
    /* 
       friend Matrix operator + (const Matrix &, const Matrix &);
       friend Matrix operator - (const Matrix &, const Matrix &);
       friend Matrix operator * (const Matrix &, const Matrix &);
       friend Vector operator * (const Matrix &, const Vector &);
       friend Vector operator * (const Vector &, const Matrix &);
    */
    
    // friend Matrix operator * (const double  , const Matrix &);
    // friend Matrix operator * (const Matrix &, const double  );
    // friend Vector operator * (const Matrix &, const Vector &);
    // friend Vector operator * (const Vector &, const Matrix &);
    
  public:
    // util...
    // void print() const;
    
  protected:
    double m11,m12,m13,m21,m22,m23,m31,m32,m33;
  };
  
  // Matrix operator * (const double &, const Matrix &);
  // Matrix operator * (const Matrix &, const double &);
  // Vector operator * (const Matrix &, const Vector &);
  // Vector operator * (const Vector &, const Matrix &);
  
  inline Matrix operator * (const double & f, const Matrix & m) {
    Matrix q(m);
    q *= f;
    return q;
  }
  
  inline Matrix operator * (const Matrix & m, const double & f) {
    Matrix q(m);
    q *= f;
    return q;
  }

  inline Vector operator * (const Matrix & m, const Vector & v) {
    return Vector (m.getM11()*v.getX()+m.getM12()*v.getY()+m.getM13()*v.getZ(),
		   m.getM21()*v.getX()+m.getM22()*v.getY()+m.getM23()*v.getZ(),
		   m.getM31()*v.getX()+m.getM32()*v.getY()+m.getM33()*v.getZ());
  }
  
  inline Vector operator * (const Vector & v, const Matrix & m) {
    return Vector (v.getX()*m.getM11()+v.getY()*m.getM21()+v.getZ()*m.getM31(),
		   v.getX()*m.getM12()+v.getY()*m.getM22()+v.getZ()*m.getM32(),
		   v.getX()*m.getM13()+v.getY()*m.getM23()+v.getZ()*m.getM33());
  }
  
  /* 
     Matrix operator + (const Matrix &, const Matrix &);
     Matrix operator - (const Matrix &, const Matrix &);
     Matrix operator * (const double  , const Matrix &);
     Matrix operator * (const Matrix &, const double  );
     Matrix operator * (const Matrix &, const Matrix &);
     Vector operator * (const Matrix &, const Vector &);
     Vector operator * (const Vector &, const Matrix &);
  */
  
} // namespace orsa

#endif // _ORSA_MATRIX_