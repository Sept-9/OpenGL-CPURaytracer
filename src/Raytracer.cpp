//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5892M Advanced Rendering
//  User Interface for Coursework
////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <thread>
#include <random>
#include <omp.h>
#include <algorithm>
// include the header file
#include "Raytracer.h"

#define N_THREADS 16
#define N_LOOPS 600
#define N_BOUNCES 5
#define TERMINATION_FACTOR 0.35f
#define M_PI 3.14159265358979323846

// constructor
Raytracer::Raytracer(std::vector<ThreeDModel>* newTexturedObject, RenderParameters* newRenderParameters) : texturedObjects(newTexturedObject),
renderParameters(newRenderParameters),
raytraceScene(texturedObjects, renderParameters)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    restartRaytrace = false;
    raytracingRunning = false;
}

Raytracer::~Raytracer()
{
    // all of our pointers are to data owned by another class
    // so we have no responsibility for destruction
}

// called every time the widget is resized
void Raytracer::resize(int w, int h)
{ // RaytraceRenderWidget::resizeGL()
    // resize the render image

    // frameBuffer.clear(RGBAValue(125.0f, 125.0f, 125.0f, 255.0f));
    frameBuffer.Resize(w, h);
} // RaytraceRenderWidget::resizeGL()

void Raytracer::stopRaytracer()
{
    restartRaytrace = true;
    while (raytracingRunning)
    {
        std::chrono::milliseconds timespan(10);
        std::this_thread::sleep_for(timespan);
    }
    restartRaytrace = false;
}
Ray Raytracer::calculateRay(int pixelx, int pixely, bool perspective)
{
    float width = this->frameBuffer.width;
    float height = this->frameBuffer.height;
    float aspect = float(width) / float(height);
    float top = renderParameters->near * tan(renderParameters->fov / 2.0f);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;
    // aa
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    float jitterX = distribution(generator);  // [0, 1) 
    float jitterY = distribution(generator);
    float offsetX = renderParameters->aaEnabled ? jitterX : 0.5f;
    float offsetY = renderParameters->aaEnabled ? jitterY : 0.5f;

    float ndcX = left + (right - left) * ((pixelx + offsetX) / width);
    float ndcY = bottom + (top - bottom) * ((pixely + offsetY) / height);

    if (perspective)
    {
        Cartesian3 dirCDC = Cartesian3(ndcX, ndcY, renderParameters->near).unit();
        // no need to use camera ratation matrix because assuming camera in (0,0,0)
        // move camera transformation to the mvp part(done the inverse transformation)
        return Ray(Cartesian3(0, 0, 0), dirCDC, Ray::Type::primary);
    }
    else
    {

        float l = aspect > 1.0 ? -aspect : -1;
        float r = aspect > 1.0 ? aspect : 1;
        float t = aspect > 1.0 ? 1 : 1.0f / aspect;
        float b = aspect > 1.0 ? -1 : -1.0f / aspect;

        float x = l + (r - l) * ((pixelx + 0.5f) / width);
        float y = b + (t - b) * ((pixely + 0.5f) / height);

        Cartesian3 originCameraSpace = Cartesian3(x, y, 0);
        Cartesian3 dirCameraSpace = Cartesian3(0, 0, 1);

        // Matrix4 cameraRotation = renderParameters->CameraArcball.GetRotation();
        // Cartesian3 originWorldSpace = cameraRotation * originCameraSpace + renderParameters->CameraPosition;
        // Cartesian3 dirWorldSpace = cameraRotation * dirCameraSpace;

        // return Ray(originWorldSpace, dirWorldSpace, Ray::Type::primary);
        return Ray(originCameraSpace, dirCameraSpace, Ray::Type::primary);
    }
}
inline float linear_from_srgb(std::uint8_t aValue) noexcept
{
    float const fvalue = float(aValue) / 255.f;

    if (fvalue < 0.04045f)
        return (1.f / 12.92f) * fvalue;

    return std::pow((1.f / 1.055f) * (fvalue + 0.055f), 2.4f);
}

inline std::uint8_t linear_to_srgb(float aValue) noexcept
{
    if (aValue < 0.0031308f)
        return std::uint8_t(255.f * 12.92f * aValue + 0.5f);
    return std::uint8_t(255.f * (1.055f * std::pow(aValue, 1.f / 2.4f) - 0.055f) + 0.5f);
}
Ray Raytracer::reflectRay(Ray r, Cartesian3 normal, Cartesian3 hitPoint)
{
    if (r.direction.dot(normal) > 0) {
        normal = Cartesian3(-normal.x, -normal.y, -normal.z);
    }
    float dotProduct = r.direction.dot(normal);
    Cartesian3 reflectDir = r.direction - normal * (2.0f * dotProduct);
    return Ray(hitPoint + 0.01f * normal, reflectDir.unit(), Ray::Type::primary); // ori maybe need to offset a little bit
}
Ray  Raytracer::refractRay(Ray r,
    Cartesian3 normal,
    Cartesian3 hitPoint,
    float ior1,
    float ior2,
    bool& isInner,
    float& fr)
{
    Cartesian3 I = r.direction.unit();
    Cartesian3 N = normal.unit();
    isInner = false;
    float cosi = -I.dot(N);
    if (cosi < 0.0f)
    {
        cosi = -cosi;
        N = Cartesian3(-N.x, -N.y, -N.z);
        float temp = ior1;
        ior1 = ior2;
        ior2 = temp;
    }
    float eta = ior1 / ior2;
    float k = 1.0f - eta * eta * (1.0f - cosi * cosi);
    fr = fresnelSchlick(cosi, ior1, ior2);

    if (k < 0.0f)
    {
        isInner = true;
        return Ray(hitPoint, Cartesian3(0, 0, 0), Ray::primary);
    }
    k = std::max(0.0f, k);
    Cartesian3 refractDir = eta * I + (eta * cosi - std::sqrt(k)) * N;
    refractDir = refractDir.unit();
    const float eps = 1e-4f;

    Cartesian3 newOrigin = hitPoint - N * eps;
    return Ray(newOrigin, refractDir, Ray::primary);
}
float Raytracer::fresnelSchlick(float cosTheta, float ior1, float ior2) {
    float F0 = pow((ior1 - ior2) / (ior1 + ior2), 2);
    float F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    return F;
}

Homogeneous4 Raytracer::TraceAndShadeWithRay(Ray CameraRay, int bounceNum, float power, bool isNEE)
{
    if (power < 0.01f || bounceNum < 0)
    {
        return Homogeneous4(0, 0, 0);
    }
    //if (bounceNum < N_BOUNCES - 3) // 前4次必定追踪
    //{
    //    float continueProbability = 0.4f;
    //    static thread_local std::mt19937 gen(std::random_device{}());
    //    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    //    if (dist(gen) > continueProbability)
    //        return Homogeneous4(0, 0, 0);

    //    power /= continueProbability; // 补偿概率
    //}
    Scene::CollisionInfo cInfo = raytraceScene.closestTriangle(CameraRay);
    if (cInfo.t > 0)
    {
        Cartesian3 bari = cInfo.tri.baricentric(CameraRay.origin + CameraRay.direction * cInfo.t);
        Cartesian3 normOut = (cInfo.tri.normals[0].Vector() * bari.x + cInfo.tri.normals[1].Vector() * bari.y + cInfo.tri.normals[2].Vector() * bari.z).unit();
        Homogeneous4 reflectColor = Cartesian3(0, 0, 0);
        Homogeneous4 refractColor = Cartesian3(0, 0, 0);
        if (cInfo.tri.shared_material->isLight())
        {
            if (isNEE) {
                return Homogeneous4(cInfo.tri.shared_material->emissive.x,
                    cInfo.tri.shared_material->emissive.y,
                    cInfo.tri.shared_material->emissive.z);
            }
            else {
                return Homogeneous4(0, 0, 0);
            }
            /*return Homogeneous4(cInfo.tri.shared_material->emissive.x,
                cInfo.tri.shared_material->emissive.y,
                cInfo.tri.shared_material->emissive.z);*/
        }
        if (renderParameters->refractionEnabled && cInfo.tri.shared_material->transparency > 0) {
            bool isInner = false;
            float ior1 = 1;
            float ior2 = cInfo.tri.shared_material->indexOfRefraction;
            float fr = 0;
            Ray refaRay = refractRay(CameraRay, normOut, CameraRay.origin + CameraRay.direction * cInfo.t, ior1, ior2, isInner, fr);
            if (isInner) {
                fr = 1;
            }
            Ray reflRay = reflectRay(CameraRay, normOut, CameraRay.origin + CameraRay.direction * cInfo.t);
            Cartesian3 I = CameraRay.direction.unit();
            Cartesian3 N = normOut.unit();
            float cosi = -I.dot(N);
            Homogeneous4 absorption(1.0f, 1.0f, 1.0f);
            if (cosi < 0) {
                float distance = cInfo.t * CameraRay.direction.length(); // 当前段的传播距离
                Cartesian3 diffuse = cInfo.tri.shared_material->diffuse;

                // 对每个颜色通道分别计算吸收
                absorption = Homogeneous4(
                    exp(-diffuse.x * distance),
                    exp(-diffuse.y * distance),
                    exp(-diffuse.z * distance)
                );
                //absorption = Homogeneous4(0.3000, 0.3000, 0.2000);
                //std::cout << absorption << std::endl;
            }
            refractColor = isInner ? Homogeneous4(0, 0, 0) : TraceAndShadeWithRay(refaRay, bounceNum - 1, power * (1 - fr));
            reflectColor = TraceAndShadeWithRay(reflRay, bounceNum - 1, power * fr);
            Homogeneous4 finalColor = refractColor * (1 - fr) + reflectColor * fr;
            finalColor = finalColor = Homogeneous4(
                finalColor.x * absorption.x,
                finalColor.y * absorption.y,
                finalColor.z * absorption.z
            );
            return finalColor;
        }
        else {
            if (renderParameters->reflectionEnabled && bounceNum > 0)
            {
                Ray refRay = reflectRay(CameraRay, normOut, CameraRay.origin + CameraRay.direction * cInfo.t);
                reflectColor = TraceAndShadeWithRay(refRay, bounceNum - 1, power * cInfo.tri.shared_material->reflectivity);
            }
        }
        if (renderParameters->interpolationRendering)
        {
            return Homogeneous4(
                abs(normOut.x),
                abs(normOut.y),
                abs(normOut.z));
        }
        if (renderParameters->phongEnabled)
        {
            Cartesian3 allLight = Cartesian3(0, 0, 0);
            bool inShadow = false;
            for (auto l : renderParameters->lights)
            {
                Homogeneous4 lp = raytraceScene.getModelview() * l->GetPositionCenter();
                Homogeneous4 lpSample = raytraceScene.getModelview() * l->GetPosition();
                if (renderParameters->shadowsEnabled)
                {
                    Cartesian3 o = CameraRay.origin + CameraRay.direction * cInfo.t;
                    Cartesian3 ShadowRayDir = (lp.Point() - o).unit();
                    Ray ShadowRay = Ray(o + normOut * 0.01f, ShadowRayDir, Ray::Type::secondary);
                    Scene::CollisionInfo shadowHit = raytraceScene.closestTriangle(ShadowRay);
                    inShadow = false;
                    if (shadowHit.t > 0.0f && !shadowHit.tri.shared_material->isLight() && shadowHit.t < 1.0f)
                    {
                        inShadow = true;
                    }
                }
                if (inShadow && !renderParameters->refractionEnabled)
                {
                    allLight = cInfo.tri.ShadowLight(l->GetColor()).Vector() + allLight;
                }
                else if (renderParameters->monteCarloEnabled) {
                    allLight = MonteCarloPathTracing(l, l->GetColor(), CameraRay.origin + CameraRay.direction * cInfo.t, cInfo.tri, bounceNum, CameraRay.ray_type == Ray::Type::secondary, lpSample).Vector() + allLight;
                }
                else
                {
                    allLight = cInfo.tri.BlinnPhongLight(lp, l->GetColor(), CameraRay.origin + CameraRay.direction * cInfo.t).Vector() + allLight;
                }
            }
            allLight.x = std::clamp(allLight.x, 0.0f, 1.0f);
            allLight.y = std::clamp(allLight.y, 0.0f, 1.0f);
            allLight.z = std::clamp(allLight.z, 0.0f, 1.0f);
            if (renderParameters->reflectionEnabled)
            {
                float refl = cInfo.tri.shared_material->reflectivity;
                return Homogeneous4(refl * reflectColor.x + (1 - refl) * allLight.x,
                    refl * reflectColor.y + (1 - refl) * allLight.y,
                    refl * reflectColor.z + (1 - refl) * allLight.z);
            }
            else
                return Homogeneous4(allLight.x,
                    allLight.y,
                    allLight.z);
        }
        else
            return Homogeneous4(255.0f, 255.0f, 255.0f);
    }
    return Homogeneous4(0.0f, 0.0f, 0.0f);
}
Homogeneous4 Raytracer::MonteCarloPathTracing(Light* l, Homogeneous4 lightColor, Cartesian3 o, Triangle& tri, int bounceNum, bool isSecondary, Homogeneous4 lpp) {
    // define sample number
    int nSamples = 5;
    int nlightSamples = 5;
    Cartesian3 bari = tri.baricentric(o);
    Cartesian3 n = (tri.normals[0].Vector() * bari.x + tri.normals[1].Vector() * bari.y + tri.normals[2].Vector() * bari.z).unit();
    Cartesian3 vl = Cartesian3(0, 0, 0);
    Cartesian3 vv = (Cartesian3(0, 0, 0) - o).unit();
    Cartesian3 directLight = Cartesian3(0, 0, 0);
    float lightPdf = 1.0f;
    for (int i = 0; i < nlightSamples; i++) {
        Cartesian3 lp = raytraceScene.getModelview() * l->GetPosition().Point();
        //Cartesian3 lp = lpp.Vector();
        Cartesian3 ori = o + n * 0.001f;
        Cartesian3 toLight = lp - ori;
        Cartesian3 vl = toLight.unit();
        Ray dirLight = Ray(ori, toLight, Ray::Type::secondary);
        Scene::CollisionInfo shadowHit = raytraceScene.closestTriangle(dirLight);
        Homogeneous4 transmittance = TraceShadowRayWithRefraction(dirLight, 1000);
        if (transmittance.Vector() == Cartesian3(0, 0, 0))
        //if (shadowHit.t > 0.0f && !shadowHit.tri.shared_material->isLight() && shadowHit.t < 1.0f)
        {
            directLight = directLight;
        }
        else {
            // bling phong brdf
            Cartesian3 h = (vl + vv).unit();
            float cosTheta = std::clamp(n.dot(vl), 0.0f, 1.0f);
            float spFactor = std::clamp(n.dot(h), 0.0f, 1.0f);
            Cartesian3 TempS = Cartesian3(0, 0, 0);
            float Sp = pow(spFactor, tri.shared_material->shininess);
            TempS = tri.shared_material->specular * Sp * cosTheta * (tri.shared_material->shininess + 2) / (2 * M_PI);
            Cartesian3 TempD = tri.shared_material->diffuse * cosTheta;
            Cartesian3 brdf = TempD + TempS;
            Cartesian3 contribution = lightColor.Vector().multiplication( transmittance.Vector().multiplication(TempS + TempD))/ lightPdf;
            //Cartesian3 contribution = lightColor.Vector().multiplication((TempS + TempD)) / lightPdf;
            directLight = directLight + contribution;
        }
    }
    directLight = directLight / (float)nlightSamples;




    Cartesian3 TempE = tri.shared_material->emissive;
    // change ambient part

    Cartesian3 indirectLight = Cartesian3(0, 0, 0);
    Cartesian3 diffuse = tri.shared_material->diffuse;
    //Cartesian3 fr = tri.shared_material->diffuse / M_PI;
    //float cosThetaMonte = std::max(0.0f, n.dot(monteRay.direction));
    //float pdf = cosThetaMonte / M_PI;
    //indirectLight = indirectLight + fr * Li * cosTheta / pdf = indirectLight + Li * diffuse
    if (isSecondary) {
        /*if (tri.shared_material->transparency > 0) {
            for (int i = 0; i < 3; i++) {
                Ray monteRay = MonteCarloRay(o, n);
                Cartesian3 Li = TraceAndShadeWithRay(monteRay, bounceNum - 1, 1.0f, false).Vector();

                indirectLight = indirectLight + Cartesian3(Li.x * diffuse.x, Li.y * diffuse.y, Li.z * diffuse.z);
            }
            indirectLight = indirectLight / 3.0f;
        }
        else {
            Ray monteRay = MonteCarloRay(o, n);
            Cartesian3 Li = TraceAndShadeWithRay(monteRay, bounceNum - 1, 1.0f, false).Vector();
            indirectLight = Cartesian3(Li.x * diffuse.x, Li.y * diffuse.y, Li.z * diffuse.z);
        }*/
        Ray monteRay = MonteCarloRay(o, n);
        Cartesian3 Li = TraceAndShadeWithRay(monteRay, bounceNum - 1, 1.0f, false).Vector();
        indirectLight = Li.multiplication(diffuse);
    }
    else {
        for (int i = 0; i < nSamples; i++) {
            Ray monteRay = MonteCarloRay(o, n);
            Cartesian3 Li = TraceAndShadeWithRay(monteRay, bounceNum - 1, 1.0f, false).Vector();

            indirectLight = indirectLight + Li.multiplication(diffuse);
        }
        indirectLight = indirectLight / float(nSamples);
    }
    //indirectLight = tri.shared_material->ambient;
    Homogeneous4 Total = Homogeneous4(indirectLight + directLight + TempE);
    return Total;
}
// cos-weighted
Ray Raytracer::MonteCarloRay(Cartesian3 hitPoint, Cartesian3 normal) {
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

    float r1 = distribution(generator);
    float r2 = distribution(generator);

    float theta = acos(sqrt(r1));
    float phi = 2.0f * M_PI * r2;

    float x = sin(theta) * cos(phi);
    float y = sin(theta) * sin(phi);
    float z = cos(theta);

    Cartesian3 up = (fabs(normal.y) < 0.999f) ? Cartesian3(0, 1, 0) : Cartesian3(1, 0, 0);
    Cartesian3 tangent = up.cross(normal).unit();
    Cartesian3 bitangent = normal.cross(tangent);

    Cartesian3 direction = (tangent * x + bitangent * y + normal * z).unit();

    return Ray(hitPoint + normal * 0.001f, direction, Ray::Type::secondary);
}
Homogeneous4 Raytracer::TraceShadowRayWithRefraction(Ray shadowRay, float maxDistance)
{
    Homogeneous4 transmittance(1.0f, 1.0f, 1.0f);
    float traveledDistance = 0.0f;
    Ray currentRay = shadowRay;
    int maxRefractions = 8; // Limit refraction bounces for shadow rays

    for (int depth = 0; depth < maxRefractions; depth++)
    {
        Scene::CollisionInfo hit = raytraceScene.closestTriangle(currentRay);

        if (hit.t <= 0.0f)
        {
            break;
        }
        traveledDistance += hit.t;
        if (traveledDistance >= maxDistance)
        {
            break;
        }
        if (hit.tri.shared_material->isLight())
        {
            return transmittance;
        }
        if (hit.tri.shared_material->transparency <= 0.001f)
        {
            return Homogeneous4(0.0f, 0.0f, 0.0f);
        }

        Cartesian3 hitPoint = currentRay.origin + currentRay.direction * hit.t;
        Cartesian3 bari = hit.tri.baricentric(hitPoint);
        Cartesian3 normal = (hit.tri.normals[0].Vector() * bari.x +
            hit.tri.normals[1].Vector() * bari.y +
            hit.tri.normals[2].Vector() * bari.z).unit();

        bool isInner = false;
        float ior1 = 1.0f;
        float ior2 = hit.tri.shared_material->indexOfRefraction;
        float fr = 0.0f;
        Ray refractedRay = refractRay(currentRay, normal, hitPoint, ior1, ior2, isInner, fr);
        if (isInner)
        {
            return Homogeneous4(0.0f, 0.0f, 0.0f);
        }
        Cartesian3 I = currentRay.direction.unit();
        Cartesian3 N = normal.unit();
        float cosi = -I.dot(N);
        if (cosi < 0)
        {
            float distance = hit.t;
            Cartesian3 absorbColor = hit.tri.shared_material->diffuse;
            Homogeneous4 absorption(
                exp(-absorbColor.x * distance),
                exp(-absorbColor.y * distance),
                exp(-absorbColor.z * distance)
            );
            transmittance = Homogeneous4(
                transmittance.x * absorption.x,
                transmittance.y * absorption.y,
                transmittance.z * absorption.z
            );
        }

        float transmission = 1.0f - fr;
        transmittance = Homogeneous4(
            transmittance.x * transmission,
            transmittance.y * transmission,
            transmittance.z * transmission
        );

        currentRay = refractedRay;

        if (transmittance.x < 0.001f && transmittance.y < 0.001f && transmittance.z < 0.001f)
        {
            return Homogeneous4(0.0f, 0.0f, 0.0f);
        }
    }

    return transmittance;
}
void Raytracer::RaytraceThread()
{
    for (int j = 0; j < frameBuffer.height; j++)
    {

#pragma omp parallel for schedule(dynamic)

        for (int i = 0; i < frameBuffer.width; i++)
        {
            // Ray CameraRay = calculateRay(i,j,ture)
            Ray CameraRay = calculateRay(i, j, !renderParameters->orthoProjection);
            Homogeneous4 result = TraceAndShadeWithRay(CameraRay, N_BOUNCES, 1.0f, true);
            frameBuffer[j][i] = RGBAValue(linear_to_srgb(result.x), linear_to_srgb(result.y), linear_to_srgb(result.z), 255);
        }
        if (restartRaytrace)
        {
            raytracingRunning = false;
            return;
        }
    }
    raytracingRunning = false;
    // Tutorial code here!
}

// routine that generates the image
void Raytracer::Raytrace()
{ // RaytraceRenderWidget::Raytrace()
    stopRaytracer();
    // To make our lifes easier, lets calculate things on VCS.
    // So we need to process our scene to get a triangle soup in VCS.
    raytraceScene.updateScene();
    frameBuffer.clear(RGBAValue(0.0f, 0.0f, 0.0f, 1.0f));
    std::thread raytracingThread(&Raytracer::RaytraceThread, this);
    raytracingThread.detach();
    raytracingRunning = true;
} // RaytraceRenderWidget::Raytrace()

void Raytracer::RaytraceDebug()
{
    // renderParameters->ModelPosition = Cartesian3(0.0, 0.0, 0.0);
    // renderParameters->CameraPosition = Cartesian3(0.0, 0.0, 0.0);
    // renderParameters->ModelArcball = Quaternion(0, 1, 0, 0);
    // renderParameters->CameraArcball = Quaternion(0, 0, 0, 1);
    // raytraceScene.updateScene();
    // std::cout << std::endl;
    // std::cout << "###############################" << std::endl;
    // std::cout << "#Debugging the first few steps# " << std::endl;
    // std::cout << "###############################" << std::endl;
    // std::cout << std::endl;
    // std::cout << std::endl;
    // std::cout << "#Task 1# " << std::endl;
    // std::cout << std::endl;
    // std::cout << "#No transformations: # " << std::endl;
    // Triangle tri = raytraceScene.triangles[0];
    // std::cout << "v0:" << tri.verts[0] << std::endl;
    // std::cout << "v1:" << tri.verts[1] << std::endl;
    // std::cout << "v2:" << tri.verts[2] << std::endl;
    // std::cout << "#Model at z=2, 1 to left: # " << std::endl;
    // renderParameters->ModelPosition = Cartesian3(-1.0, 0.0, 2.0);
    // raytraceScene.updateScene();
    // tri = raytraceScene.triangles[0];
    // std::cout << "v0:" << tri.verts[0] << std::endl;
    // std::cout << "v1:" << tri.verts[1] << std::endl;
    // std::cout << "v2:" << tri.verts[2] << std::endl;
    // std::cout << "#Just rotate 180 (signals flip): # " << std::endl;
    // renderParameters->ModelPosition = Cartesian3(0.0, 0.0, 0.0);
    // renderParameters->ModelArcball.currentRotation = Quaternion(0, 0, 1, 0);
    // raytraceScene.updateScene();
    // tri = raytraceScene.triangles[0];
    // std::cout << "v0:" << tri.verts[0] << std::endl;
    // std::cout << "v1:" << tri.verts[1] << std::endl;
    // std::cout << "v2:" << tri.verts[2] << std::endl;
    // std::cout << "#Rotate 180 and translate: Did it go left? # " << std::endl;
    // renderParameters->ModelPosition = Cartesian3(-1.0, 0.0, 2.0);
    // renderParameters->ModelArcball.currentRotation = Quaternion(0, 0, 1, 0);
    // raytraceScene.updateScene();
    // tri = raytraceScene.triangles[0];
    // std::cout << "v0:" << tri.verts[0] << std::endl;
    // std::cout << "v1:" << tri.verts[1] << std::endl;
    // std::cout << "v2:" << tri.verts[2] << std::endl;
    // std::cout << std::endl;
    // std::cout << "#Task 2# " << std::endl;
    // std::cout << std::endl;
    // std::cout << "#One ray to each corner!# " << std::endl;
    // Ray r0 = calculateRay(0, 0, true);
    // Ray r1 = calculateRay(0, frameBuffer.height - 1, true);
    // Ray r2 = calculateRay(frameBuffer.width - 1, frameBuffer.height - 1, true);
    // Ray r3 = calculateRay(frameBuffer.width - 1, 0, true);
    // std::cout << "pixel is " << 0 << " " << 0 << " d: " << r0.direction << std::endl;
    // std::cout << "pixel is " << 0 << " " << frameBuffer.height - 1 << " d: " << r1.direction << std::endl;
    // std::cout << "pixel is " << frameBuffer.width - 1 << " " << frameBuffer.height - 1 << " d: " << r2.direction << std::endl;
    // std::cout << "pixel is " << frameBuffer.width - 1 << " " << 0 << " d: " << r3.direction << std::endl;
    // std::cout << "#Rays to the center on a diagonal!# " << std::endl;
    // Ray rc = calculateRay(((frameBuffer.width - 1) / 2.0f) - 1, ((frameBuffer.height - 1) / 2.0f) - 1,
    //                       true);
    // Ray rc1 = calculateRay(((frameBuffer.width - 1) / 2.0f), ((frameBuffer.height - 1) / 2.0f), true);
    // Ray rc2 = calculateRay(((frameBuffer.width - 1) / 2.0f) + 1, ((frameBuffer.height - 1) / 2.0f) + 1,
    //                        true);
    // Ray rc3 = calculateRay(((frameBuffer.width - 1) / 2.0f) + 2, ((frameBuffer.height - 1) / 2.0f) + 2, true);
    // std::cout << "pixel is " << ((frameBuffer.width - 1) / 2.0f) - 1 << " " << ((frameBuffer.height - 1) / 2.0f) - 1 << " d: " << rc.direction << std::endl;
    // std::cout << "pixel is " << (frameBuffer.width - 1) / 2.0f << " " << ((frameBuffer.height - 1) / 2.0f) << " d: " << rc1.direction << std::endl;
    // std::cout << "pixel is " << ((frameBuffer.width - 1) / 2.0f) + 1 << " " << ((frameBuffer.height - 1) / 2.0f) + 1 << " d: " << rc2.direction << std::endl;
    // std::cout << "pixel is " << ((frameBuffer.width - 1) / 2.0f) + 2 << " " << ((frameBuffer.height - 1) / 2.0f) + 2 << " d: " << rc3.direction << std::endl;
    // renderParameters->ModelPosition = Cartesian3(0.0, 0.0, 2.0);
    // renderParameters->CameraPosition = Cartesian3(0.0, 0.0, 0.0);
    // renderParameters->ModelArcball = Quaternion(0, 1, 0, 0);
    // renderParameters->CameraArcball = Quaternion(0, 0, 0, 1);
    // raytraceScene.updateScene();
    // Scene::CollisionInfo ci0 = raytraceScene.closestTriangle(r0);
    // Scene::CollisionInfo ci1 = raytraceScene.closestTriangle(r1);
    // Scene::CollisionInfo ci2 = raytraceScene.closestTriangle(r2);
    // Scene::CollisionInfo ci3 = raytraceScene.closestTriangle(r3);
    // std::cout << "#Do they hit at initial position? (z=2)# " << std::endl;
    // std::cout << ci0.tri.isValid() << "== 0" << std::endl;
    // std::cout << ci1.tri.isValid() << "== 1" << std::endl;
    // std::cout << ci2.tri.isValid() << "== 1" << std::endl;
    // std::cout << ci3.tri.isValid() << "== 1" << std::endl;
}
