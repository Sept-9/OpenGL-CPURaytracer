#include "RenderParameters.h"
using namespace std;

void RenderParameters::findLights(std::vector<ThreeDModel> objects)
{
    for(ThreeDModel obj: objects)
    {
        //find objects that have a "light" material
        if(obj.material->isLight())
        {
            //if the object has exactly 2 triangles, its a rectangular area light.
            if(obj.faceVertices.size()== 2)
            {
                //we find one of the corner vertices not in the diagonal, and from there we have everything we need.
                //we assume the lights will be triangles, not quads.
                for (unsigned int i = 0; i < 3; i++)
                {
                    unsigned int vid = obj.faceVertices[0][i];
                    bool found = false;
                    for(unsigned int j = 0; j < 3; j++)
                    {
                        if(vid == obj.faceVertices[1][j])
                        {
                            found = true;
                            break;
                        }
                    }
                    if(!found)
                    {
                        unsigned int id1 = obj.faceVertices[0][i];

                        unsigned int id2 = obj.faceVertices[0][(i + 1) % 3];

                        unsigned int id3 = obj.faceVertices[0][(i + 2) % 3];

                        Cartesian3 v1 = obj.vertices[id1];

                        Cartesian3 v2 = obj.vertices[id2];

                        Cartesian3 v3 = obj.vertices[id3];

                        Cartesian3 vecA = v2 - v1;

                        Cartesian3 vecB = v3 - v1;

                        Homogeneous4 t1(vecA.x, vecA.y, vecA.z, 0.0f);

                        Homogeneous4 t2(vecB.x, vecB.y, vecB.z, 0.0f);

                        Homogeneous4 color = obj.material->emissive;

                        Homogeneous4 pos = v1 + (vecA / 2) + (vecB / 2);

                        Homogeneous4 normal = obj.normals[obj.faceNormals[0][0]];

                        Light* l = new Light(Light::Area, color, pos, normal, t1, t2);

                        l->enabled = true;

                        lights.push_back(l);

                        break;
                    }
                }
            }
            else
            {
                Cartesian3 center = Cartesian3(0,0,0);
                for (unsigned int i = 0; i < obj.vertices.size(); i++)
                {
                    center = center + obj.vertices[i];
                }
                center = center / float(obj.vertices.size());

                float maxDist = 0;
                for (unsigned int i = 0; i < obj.vertices.size(); i++)
                {
                    maxDist = std::max(maxDist,(obj.vertices[i] - center).length());
                }
                Light *l = new Light(Light::Point,obj.material->emissive,center,Homogeneous4(),Homogeneous4(),Homogeneous4());
                l->enabled = true;
                lights.push_back(l);
            }
        }
    }
}


void RenderParameters::computeMatricesFromInputs(float deltaTime, byte movementKeys) {

    Matrix4 cr = CameraArcball.GetRotation();
    Cartesian3 x =  Cartesian3(1, 0, 0);
    Cartesian3 y =  Cartesian3(0, 1, 0);
    Cartesian3 z = Cartesian3(0, 0, 1);
    
    // Move forward
    if ((movementKeys & byte{ BIT_FW }) != byte{ 0 }) {
        if ((movementKeys & byte{ BIT_RIGHTMOUSE }) != byte{ 0 }) {
            ModelPosition = ModelPosition + deltaTime * speed * z;
        }
        if ((movementKeys & byte{ BIT_LEFTMOUSE }) != byte{ 0 }) {
            CameraPosition = CameraPosition + deltaTime * speed * (cr*z).unit();
        }
    }
    // Move backward
    if ((movementKeys & byte{ BIT_BACK }) != byte{ 0 }) {
        if ((movementKeys & byte{ BIT_RIGHTMOUSE }) != byte{ 0 }) {
            ModelPosition = ModelPosition - deltaTime * speed * z;
        }
        if ((movementKeys & byte{ BIT_LEFTMOUSE }) != byte{ 0 }) {
            CameraPosition = CameraPosition - deltaTime * speed * (cr*z).unit();
        }
    }
    // Strafe right
    if ((movementKeys & byte{ BIT_RIGHT }) != byte{ 0 }) {
        if ((movementKeys & byte{ BIT_RIGHTMOUSE }) != byte{ 0 }) {
            ModelPosition = ModelPosition + deltaTime * speed * x;
        }
        if ((movementKeys & byte{ BIT_LEFTMOUSE }) != byte{ 0 }) {
            CameraPosition = CameraPosition + deltaTime * speed *(cr* x).unit();
        }
    }
    // Strafe left
    if ((movementKeys & byte{ BIT_LEFT }) != byte{ 0 }) {
        if ((movementKeys & byte{ BIT_RIGHTMOUSE }) != byte{ 0 }) {
            ModelPosition = ModelPosition - deltaTime * speed * x;
        }
        if ((movementKeys & byte{ BIT_LEFTMOUSE }) != byte{ 0 }) {
            CameraPosition = CameraPosition - deltaTime * speed * (cr*x).unit();

        }
    }
    // Pan up
    if ((movementKeys & byte{ BIT_UP }) != byte{ 0 }) {
        if ((movementKeys & byte{ BIT_RIGHTMOUSE }) != byte{ 0 }) {
            ModelPosition = ModelPosition + deltaTime * speed * y;
        }
        if ((movementKeys & byte{ BIT_LEFTMOUSE }) != byte{ 0 }) {
            CameraPosition = CameraPosition + deltaTime * speed *(cr* y).unit();
        }
    }
    // Pan down
    if ((movementKeys & byte{ BIT_DOWN }) != byte{ 0 }) {
        if ((movementKeys & byte{ BIT_RIGHTMOUSE }) != byte{ 0 }) {
            ModelPosition = ModelPosition - deltaTime * speed * y;
        }
        if ((movementKeys & byte{ BIT_LEFTMOUSE }) != byte{ 0 }) {
            CameraPosition = CameraPosition - deltaTime * speed * (cr*y).unit();

        }
    }
} 

Matrix4 RenderParameters::getModelMatrix() 
{
    Matrix4 TranslateMatrix;
    TranslateMatrix.SetTranslation(ModelPosition);
    Matrix4 RotateMatrix = ModelArcball.GetRotation();
    return TranslateMatrix * RotateMatrix;
}

Matrix4 RenderParameters::getViewMatrix() 
{
    Matrix4 TranslateMatrix;
    TranslateMatrix.SetTranslation(-1*CameraPosition);
    Matrix4 RotateMatrix = CameraArcball.GetRotation().transpose();
    return  RotateMatrix * TranslateMatrix;
}

void RenderParameters::printSettings() 
{
    cout << "Interpolation " << interpolationRendering << endl;
    cout << "Phong " << phongEnabled << endl;
    cout << "Shadow " << shadowsEnabled << endl;
    cout << "Reflection " << reflectionEnabled << endl;
    cout << "Refraction " << refractionEnabled << endl;
    cout << "Fresnel " << fresnelRendering << endl;
    cout << "monteCarloEnabled " << monteCarloEnabled << endl;
    cout << "Ortho " << orthoProjection << endl;
}

Matrix4 RenderParameters::getProjectionMatrix(float window_w, float window_h) 
{
    Matrix4 Projection;
    float aspect =  window_w / window_h;
   
    if(orthoProjection){
        float l = aspect > 1.0 ? -aspect : -1;
        float r = aspect > 1.0 ? aspect : 1;
        float t = aspect > 1.0 ? 1 : 1.0f / aspect;
        float b = aspect > 1.0 ? -1 : -1.0f / aspect;

        Projection[0][0] = 2 / (r - l);
        Projection[1][1] = 2 / (t - b);
        Projection[2][2] = 1 / (far - near);
        Projection[0][3] = -(r + l) / (r - l);
        Projection[1][3] = -(t + b) / (t - b);
        Projection[2][3] = -near / (far - near);
        Projection[3][3] = 1;
    }
    else {       
        float tanfov2 = tanf(fov / 2.0f);
        Projection[0][0] = 1/ (tanfov2 * aspect);
        Projection[1][1] = 1/ (tanfov2);
        Projection[2][2] = far / (far - near);
        Projection[2][3] = -far * near / (far - near);
        Projection[3][2] = 1;
    }

    return Projection;
}