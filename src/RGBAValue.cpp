//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5892M Advanced Rendering
//  User Interface for Coursework
//
//  September, 2020
//
//  ------------------------
//  RGBAValue.h
//  ------------------------
//  
//  A minimal class for a colour in RGB space
//  
///////////////////////////////////////////////////

#include "RGBAValue.h"

// default constructor
RGBAValue::RGBAValue()
    :
    red(0),
    green(0),
    blue(0),
    alpha(0)
    { // constructor
    } // constructor

// value constructor
RGBAValue::RGBAValue(unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha)
    :
    red(Red),
    green(Green),
    blue(Blue),
    alpha(Alpha)
    { // constructor
    } // constructor

// value constructor with floats
// values outside 0.0-255.0 are clamped
RGBAValue::RGBAValue(float Red, float Green, float Blue, float Alpha)
    { // constructor
    if (Red > 255.0f) Red = 255.0f;
    if (Red < 0.0f) Red = 0.0f;

    red = static_cast<unsigned char>(Red);

    if (Green > 255.0f) Green = 255.0f;
    if (Green < 0.0f) Green = 0.0f;
    green = static_cast<unsigned char>(Green);

    if (Blue > 255.0f) Blue = 255.0f;
    if (Blue < 0.0f) Blue = 0.0f;
    blue = static_cast<unsigned char>(Blue);

    if (Alpha > 255.0f) Alpha = 255.0f;
    if (Alpha < 0.0f) Alpha = 0.0f;
    alpha = static_cast<unsigned char>(Alpha);
    } // constructor

// copy constructor
RGBAValue::RGBAValue(const RGBAValue &other)
    :
    red(other.red),
    green(other.green),
    blue(other.blue),
    alpha(other.alpha)
    { // copy constructor
    } // copy constructor

// convenience routines for scalar multiplication and addition
RGBAValue operator *(float scalar, const RGBAValue &colour)
    { // operator * ()
    // feed the new values into a constructor & return
    return RGBAValue(   scalar * float(colour.red),
                        scalar * float(colour.green),
                        scalar * float(colour.blue),
                        scalar * float(colour.alpha));
    } // operator * ()

RGBAValue operator +(const RGBAValue &left, const RGBAValue &right)
    { // operator + ()
    // compute new values, then use constructor to scale & clamp
    return RGBAValue(   (float(left.red)   +  float(right.red)),
                        (float( left.green) + float( right.green)),
                        (float(left.blue)  + float(right.blue)),
                        (float(left.alpha) + float(right.alpha)));
    } // operator + ()

// colour modulation routine:
// NB: this routine scales each component by 1/255.0, multiplies then inverts
RGBAValue RGBAValue::modulate(const RGBAValue &right) const
    { // modulate()
    float leftRed = float(red) / 255.0f, leftGreen = float(green) / 255.0f;
    float leftBlue = float(blue) / 255.0f, leftAlpha = float(alpha) / 255.0f;
    float rightRed = float(right.red) / 255.0f, rightGreen = float(right.green) / 255.0f;
    float rightBlue = float(right.blue) / 255.0f, rightAlpha = float(right.alpha) / 255.0f;

    // modulate them & convert back to 0..255
    return RGBAValue( 255.0f * leftRed * rightRed,255.0f * leftGreen * rightGreen, 255.0f * leftBlue * rightBlue, 255.0f * leftAlpha * rightAlpha);
    } // modulate()

// stream input
// WARNING - always sets alpha to 255
std::istream & operator >> (std::istream &inStream, RGBAValue &value)
    { // stream input
    // we want to read integers, not characters, so we create a temporary variable
    int component;
    inStream >> component;
    value.red = static_cast<unsigned char>(component);
    inStream >> component;
    value.green =static_cast<unsigned char>(component);
    inStream >> component;
    value.blue = static_cast<unsigned char>(component);
    return inStream;
    } // stream input

// stream output
// WARNING - does not output alpha
std::ostream & operator << (std::ostream &outStream, const RGBAValue &value)
    { // stream output
    // we want to output as integers, not characters, so:
    int component = 0;
    component = value.red;
    outStream << component << " ";
    component = value.green;
    outStream << component << " ";
    component = value.blue;
    outStream << component;
    return outStream;
    } // stream output
