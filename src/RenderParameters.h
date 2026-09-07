
/////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5892M Advanced Rendering
//  User Interface for Coursework
//
//  September, 2020
//
//  -----------------------------
//  Render Parameters
//  -----------------------------
//
//  This is part of the "model" in the MVC paradigm
//  We separate out the render parameters from the object rendered
//
/////////////////////////////////////////////////////////////////

// include guard
#ifndef RENDER_PARAMETERS_H
#define RENDER_PARAMETERS_H

#include "Matrix4.h"
#include "Light.h"
#include <vector>
#include "ArcBall.h"

//here not to break the includes
class ThreeDModel;
#include "ThreeDModel.h"
#include <numbers>
#include <cstddef> 

#define BIT_FW 1
#define BIT_BACK 2
#define BIT_RIGHT 4
#define BIT_LEFT 8
#define BIT_UP 16
#define BIT_DOWN 32
#define BIT_LEFTMOUSE 64
#define BIT_RIGHTMOUSE 128

// class for the render parameters
class RenderParameters
    { // class RenderParameters
    public:


    // we store x & y translations

    std::vector<Light*> lights;

    Matrix4 ViewMatrix;
    

    // and the booleans
    bool interpolationRendering;
    bool phongEnabled;
    bool fresnelRendering;
    bool shadowsEnabled;
    bool reflectionEnabled;
    bool refractionEnabled;
    bool monteCarloEnabled;
    bool centreObject;
    bool orthoProjection;
    bool aaEnabled;
    
    Cartesian3 ModelPosition;
    ArcBall ModelArcball;
    Cartesian3 CameraPosition;
    ArcBall CameraArcball;
    float speed;
    float fov;
    float near;
    float far;

    Matrix4 getModelMatrix();
    Matrix4 getViewMatrix();
    Matrix4 getProjectionMatrix(float window_w, float window_h);

    void computeMatricesFromInputs(float deltaTime, std::byte movementKeys);
    void findLights(std::vector<ThreeDModel> objects);
    void printSettings();
    

    // constructor
    RenderParameters()
        :
        ModelPosition(0.0, 0.0, 2.0),
        CameraPosition(0.0, 0.0, 0.0),
        interpolationRendering(false),
        phongEnabled(false),
        fresnelRendering(false),
        shadowsEnabled(false),
        reflectionEnabled(false),
        refractionEnabled(false),
        monteCarloEnabled(false),
        centreObject(false),
        orthoProjection(false),
        aaEnabled(false),
        speed (0.1f),
        near(0.1f),
        far(500),
        ModelArcball(Quaternion(0,1,0,0)),
        CameraArcball(Quaternion(0, 0, 0, 1))
        { // constructor
            fov = 80 * (std::numbers::pi_v<float> / 180.0f); //radians
        } // constructor

    // accessor for scaledXTranslate

    ~RenderParameters(){
        for (unsigned int i = 0;i<lights.size();i++) {
            delete(lights.at(i));
        }
    }
    
    }; // class RenderParameters

// now define some macros for bounds on parameters
#define TRANSLATE_MIN -1.0f
#define TRANSLATE_MAX 1.0f

#define ZOOM_SCALE_LOG_MIN -2.0f
#define ZOOM_SCALE_LOG_MAX 2.0f
#define ZOOM_SCALE_MIN 0.01f
#define ZOOM_SCALE_MAX 100.0f


// this is to scale to/from integer values
#define PARAMETER_SCALING 100


// end of include guard
#endif
