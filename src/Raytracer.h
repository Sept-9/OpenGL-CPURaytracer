//////////////////////////////////////////////////////////////////////
//
//	University of Leeds
//	COMP 5892M Advanced Rendering
//	User Interface for Coursework
//
//	September, 2020
//
//  -----------------------------
//  Raytrace Render Widget
//  -----------------------------
//
//	Provides a widget that displays a fixed image
//	Assumes that the image will be edited (somehow) when Render() is called
//
////////////////////////////////////////////////////////////////////////

// include guard
#ifndef RAYTRACE_RENDER_WIDGET_H
#define RAYTRACE_RENDER_WIDGET_H

#include <vector>
#include <mutex>
#include <atomic>

// and include all of our own headers that we need
#include "ThreeDModel.h"
#include "RenderParameters.h"
#include "Scene.h"

class Raytracer
{

private:
	// the geometric object to be rendered
	std::vector<ThreeDModel> *texturedObjects;
	// the render parameters to use
	RenderParameters *renderParameters;
	// An image to use as a framebuffer
	// A friendly Scene representation that we control
	Scene raytraceScene;

public:
	// constructor
	Raytracer(
		// the geometric object to show
		std::vector<ThreeDModel> *newTexturedObject,
		// the render parameters to use
		RenderParameters *newRenderParameters);

	// destructor
	~Raytracer();

	void resize(int w, int h);
	void stopRaytracer();
	RGBAImage frameBuffer;

protected:
	// called every time the widget needs painting

public:
	// routine that generates the image
	void Raytrace();
	// threading stuff
	void RaytraceThread();
	Homogeneous4 TraceAndShadeWithRay(Ray ray, int bounceNum, float power,bool isNEE = false);
	Ray reflectRay(Ray r, Cartesian3 normal, Cartesian3 hitPoint);
	void RaytraceDebug();
	Ray calculateRay(int pixelx, int pixely, bool perspective);
	Ray refractRay(Ray r, Cartesian3 normal, Cartesian3 hitPoint, float ior1, float ior2, bool& isInner,float& fr);
	float fresnelSchlick(float cosTheta, float ior1, float ior2);
	Homogeneous4 MonteCarloPathTracing(Light* l, Homogeneous4 lightColor, Cartesian3 o,Triangle& tri,int bounceNum,bool isSecondary, Homogeneous4 lp = Homogeneous4());
	// 3D Hemisphere
	Ray MonteCarloRay(Cartesian3 hitPoint, Cartesian3 normal);
	Homogeneous4 TraceShadowRayWithRefraction(Ray shadowRay, float maxDistance);
private:
	std::atomic<bool> raytracingRunning;
	std::atomic<bool> restartRaytrace;

}; // class RaytraceRenderWidget

#endif
