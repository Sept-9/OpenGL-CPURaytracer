//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
//
//  September, 2022
//
//  ------------------------
//  Cartesian3.h
//  ------------------------
//  
//  A minimal class for a point in Cartesian space
//  
///////////////////////////////////////////////////

#ifndef CARTESIAN3_H
#define CARTESIAN3_H

#include <iostream>
#include <limits>
#include <iomanip>
#include <cmath> 

// the class - we will rely on POD for sending to GPU
class Cartesian3
    { // Cartesian3
    public:
    // the coordinates
    float x, y, z;

    // constructors
    inline Cartesian3() :x(0.0), y(0.0), z(0.0)
    {}

    inline Cartesian3(float X, float Y, float Z):x(X), y(Y), z(Z)
    {}

    inline Cartesian3(const Cartesian3 &other) : x(other.x), y(other.y), z(other.z)
    {}
    
    // equality operator
    inline bool operator ==(const Cartesian3& other) const { 
        return (abs(x - other.x) < std::numeric_limits<float>::epsilon() && abs(y - other.y) < std::numeric_limits<float>::epsilon() && abs(z - other.z) < std::numeric_limits<float>::epsilon());
    }

    // addition operator
    inline Cartesian3 operator +(const Cartesian3 &other) const { 
        Cartesian3 returnVal(x + other.x, y + other.y, z + other.z);
        return returnVal;
    }

    // subtraction operator
    inline Cartesian3 operator -(const Cartesian3 &other) const {
        Cartesian3 returnVal(x - other.x, y - other.y, z - other.z);
        return returnVal;
    }
    
    // multiplication operator
    inline Cartesian3 operator *(float factor) const {
        Cartesian3 returnVal(x * factor, y * factor, z * factor);
        return returnVal;
    }

    // division operator
    inline Cartesian3 operator /(float factor) const {
        Cartesian3 returnVal(x / factor, y / factor, z / factor);
        return returnVal;
    }

    // dot product routine
    inline float dot(const Cartesian3& other) const
    {
        float returnVal = x * other.x + y * other.y + z * other.z;
        return returnVal;
    }

    // cross product routine
    Cartesian3 cross(const Cartesian3 &other) const { 
        Cartesian3 returnVal(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
        return returnVal;
    }


    // Hadamard product
    Cartesian3 multiplication(const Cartesian3& other)const {
        Cartesian3 returnVal(x * other.x , y * other.y, z * other.z);
        return returnVal;
    }
    
    // routine to find the length
    float length() const {
        return sqrt(x * x + y * y + z * z);
    }
    
    // normalisation routine
    inline Cartesian3 unit() const { 
        float length = sqrt(x * x + y * y + z * z);
        Cartesian3 returnVal(x / length, y / length, z / length);
        return returnVal;
    }
  

    }; // Cartesian3

inline Cartesian3 operator *(float factor, const Cartesian3 &right) { 
    // scalar multiplication is commutative, so flip & return
    return right * factor;
}

// stream input
inline std::istream & operator >> (std::istream &inStream, Cartesian3 &value) { // stream output
    inStream >> value.x >> value.y >> value.z;
    return inStream;
}

// stream output
inline std::ostream & operator << (std::ostream &outStream, const Cartesian3 &value) { // stream output
    outStream << std::setprecision(4) << value.x << " " << std::setprecision(4) << value.y << " " << std::setprecision(4) << value.z;
    return outStream;
}
        
#endif
