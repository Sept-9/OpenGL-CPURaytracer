//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5892M Advanced Rendering
//  User Interface for Coursework
//
//  September, 2022
//
//  ------------------------
//  Light.h
//  ------------------------
//
//  A class for different types of lights.
//
///////////////////////////////////////////////////

#ifndef LIGHT_H
#define LIGHT_H

#include "Homogeneous4.h"

class Light
{

public:

enum LightType{
    Point,
    Area,
    Directional
};

private:
    Homogeneous4 lightPosition;
    Homogeneous4 lightDirection;
    Homogeneous4 lightColor;
    Homogeneous4 tangent1;
    Homogeneous4 tangent2;
    LightType type;


public:
    Light(LightType type,Homogeneous4 color,Homogeneous4 pos, Homogeneous4 dir, Homogeneous4 tan1, Homogeneous4 tan2);
    Homogeneous4 GetPosition();
    Homogeneous4 GetPositionCenter();

    bool enabled;

    inline Homogeneous4 GetColor(){return lightColor;}
    inline LightType GetType() { return type; }
    inline float GetArea() {
        Cartesian3 edge1 = tangent1.Vector();
        Cartesian3 edge2 = tangent2.Vector();
        float area = edge1.cross(edge2).length();
        return area;
    }
};

#endif // LIGHT_H
