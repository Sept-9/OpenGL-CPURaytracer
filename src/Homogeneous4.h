//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
//
//  September, 2022
//
//  ------------------------
//  Homogeneous4.cpp
//  ------------------------
//  
//  A minimal class for a 3D point in homogeneous coordinates
//  
///////////////////////////////////////////////////

#ifndef HOMOGENEOUS4_H
#define HOMOGENEOUS4_H

#include <iostream>
#include "Cartesian3.h"
#include <iomanip>

// the class - we will rely on POD for sending to GPU
class Homogeneous4
    { // Homogeneous4
    public:
    // the coordinates
    float x, y, z, w;

    // constructors
    inline Homogeneous4()
        : x(0.0),y(0.0), z(0.0), w(0.0){}
    inline Homogeneous4(float X, float Y, float Z, float W = 1.0) 
        : x(X), y(Y), z(Z), w(W){}
    inline Homogeneous4(const Cartesian3& other) :
        x(other.x), y(other.y), z(other.z), w(1) {}
    inline Homogeneous4(const Homogeneous4& other) :
        x(other.x), y(other.y), z(other.z), w(other.w) {}
    
    // routine to get a point by perspective division
    inline Cartesian3 Point() const { 
        Cartesian3 returnVal(x / w, y / w, z / w);
        return returnVal;
    } 

    // routine to get a vector by dropping w (assumed to be 0)
    inline Cartesian3 Vector() const { 
        Cartesian3 returnVal(x, y, z);
        return returnVal;
    }

    inline Homogeneous4 modulate(Homogeneous4 b) {
        return Homogeneous4(x * b.x, y * b.y, z * b.z, w * b.w);
    } 

    // addition operator
    inline Homogeneous4 operator +(const Homogeneous4 &other) const { 
        Homogeneous4 returnVal(x + other.x, y + other.y, z + other.z, w + other.w);
        return returnVal;
    }

    // subtraction operator
    inline Homogeneous4 operator -(const Homogeneous4 &other) const {
        Homogeneous4 returnVal(x - other.x, y - other.y, z - other.z, w - other.w);
        return returnVal;
    }
    
    // multiplication operator
    inline Homogeneous4 operator *(float factor) const {
        Homogeneous4 returnVal(x * factor, y * factor, z * factor, w * factor);
        return returnVal;
    }

    // division operator
    inline Homogeneous4 operator /(float factor) const {
        Homogeneous4 returnVal(x / factor, y / factor, z / factor, w / factor);
        return returnVal;
    }

    // operator that allows us to use array indexing instead of variable names
    inline float& operator [] (const int index)
    { // operator []
    // use default to catch out of range indices
    // we could throw an exception, but will just return the 0th element instead
        switch (index)
        { // switch on index
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
            // actually the error case
        default:
            return x;
        } // switch on index
    } // operator []

// operator that allows us to use array indexing instead of variable names
    inline const float& operator [] (const int index) const
    { // operator []
    // use default to catch out of range indices
    // we could throw an exception, but will just return the 0th element instead
        switch (index)
        { // switch on index
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
            // actually the error case
        default:
            return x;
        } // switch on index
    } // operator []



    }; // Homogeneous4

// multiplication operator
inline Homogeneous4 operator *(float factor, const Homogeneous4 &right) {
    // scalar multiplication is commutative, so flip & return
    return right * factor;
}

// stream input
inline std::istream & operator >> (std::istream &inStream, Homogeneous4 &value) {
    inStream >> value.x >> value.y >> value.z >> value.w;
    return inStream;
}

// stream output
inline std::ostream & operator << (std::ostream &outStream, const Homogeneous4 &value) {
    outStream << std::setprecision(4) << value.x << " " << std::setprecision(4) << value.y << " " << std::setprecision(4) << value.z << " " << std::setprecision(4) << value.w;
    return outStream;
}
        
#endif
