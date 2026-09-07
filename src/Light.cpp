//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5892M Advanced Rendering
//  User Interface for Coursework
////////////////////////////////////////////////////////////////////////

#include "Light.h"
#include <math.h>
#include <random>

Light::Light(LightType type,Homogeneous4 color,Homogeneous4 pos, Homogeneous4 dir, Homogeneous4 tan1, Homogeneous4 tan2)
{
    this->type = type;
    this->lightPosition = pos;
    this->lightDirection = dir;
    this->tangent1 = tan1;
    this->tangent2 = tan2;
    this->lightColor = color;
    enabled = false;
}
Homogeneous4 Light::GetPositionCenter()
{
    return lightPosition;
}

Homogeneous4 Light::GetPosition()
{
    if(type == LightType::Directional){
        return lightDirection;
    }
    //if its an area light lets do a random sampling for the position
    else if(type== LightType::Area)
    {
        static thread_local std::mt19937 generator(std::random_device{}());
        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
        float r1 = distribution(generator);
        float r2 = distribution(generator);
        //float u = (-0.5f+(rand()/ float(RAND_MAX)));
        //float v = (-0.5f+(rand()/ float(RAND_MAX)));
        float u = (-0.5f + r1);
        float v = (-0.5f + r2);
        Homogeneous4 pos(lightPosition);
        pos = pos + u*tangent1;
        pos = pos + v*tangent2;
        return pos;
    }
    else {
        //for point light, we can also do a bit of montecarlo to vary the position given a "size"
        //and a random between 0 and size.
        float pi = float(2 * acos(0.0));
        //So we generate two angles for polar coordinates
        float theta = (pi*2.0f)*(rand()/ float(RAND_MAX));
        float phi = (pi*2.0f)*(rand()/ float(RAND_MAX));
        float r = 0.01f*(rand()/ float(RAND_MAX));


        //back to cartesian.
        float x = r*cos(phi)*sin(theta);
        float y = r*sin(phi)*sin(theta);
        float z = r*cos(theta);

        Cartesian3 out = lightPosition.Point() + Cartesian3(x,y,z);

        return out;
    }
}
