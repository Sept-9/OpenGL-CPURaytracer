#include "Scene.h"
#include <limits>
#include <float.h>
Scene::Scene(std::vector<ThreeDModel> *texobjs, RenderParameters *renderp)
{
    objects = texobjs;
    rp = renderp;

    Cartesian3 ambient = Cartesian3(0.5f, 0.5f, 0.5f);
    Cartesian3 diffuse = Cartesian3(0.5f, 0.5f, 0.5f);
    Cartesian3 specular = Cartesian3(0.5f, 0.5f, 0.5f);
    Cartesian3 emissive = Cartesian3(0, 0, 0);
    float shininess = 1.0f;

    default_mat = new Material(ambient, diffuse, specular, emissive, shininess);
}

Matrix4 Scene::getModelview()
{

    return rp->getViewMatrix() * rp->getModelMatrix();
}

Scene::CollisionInfo Scene::closestTriangle(Ray r)
{
    // TODO: method to find the closest triangle!
    Scene::CollisionInfo ci;
    ci.t = FLT_MAX; // this is just so it compiles warning free
    for (int i = 0; i < int(triangles.size()); i++)
    {
        float t = triangles[i].intersect(r);
        if (t != -1.0f && t < ci.t)
        {
            ci.t = t;
            ci.tri = triangles[i];
        }
    }
    if (ci.t == FLT_MAX)
        ci.t = -1.0f;
    return ci;
}
// updateScene will build the scene like we would do for
// rasterization. So very similar to "Render", but instead
// of calling glVertex3f, and instead of GL_TRIANGLES, we
// create triangles. We however need to transform things
// to VCS to raytrace, as there is no transform phase to do that
// for us.
void Scene::updateScene()
{
    triangles.clear();
    int triID = 0;
    // We go through all the objects to construct the scene
    for (int i = 0; i < int(objects->size()); i++)
    {
        typedef unsigned int uint;
        ThreeDModel obj = objects->at(uint(i));
        // Scale defaults to the zoom setting

        // This object may have a material. But if it does not, lets use from sliders.

        // loop through the faces: note that they may not be triangles, which complicates life
        for (unsigned int face = 0; face < obj.faceVertices.size(); face++)
        { // per face
            // on each face, treat it as a triangle fan starting with the first vertex on the face
            for (unsigned int triangle = 0; triangle < obj.faceVertices[face].size() - 2; triangle++)
            { // per triangle
                // now do a loop over three vertices
                Triangle t;
                for (unsigned int vertex = 0; vertex < 3; vertex++)
                { // per vertex
                    // we always use the face's vertex 0
                    uint faceVertex = 0;
                    // so if it isn't 0, we want to add the triangle base ID
                    if (vertex != 0)
                        faceVertex = triangle + vertex;

                    // this is our vertex before any transformations. (world space)
                    Homogeneous4 v = Homogeneous4(
                        obj.vertices[obj.faceVertices[face][faceVertex]].x,
                        obj.vertices[obj.faceVertices[face][faceVertex]].y,
                        obj.vertices[obj.faceVertices[face][faceVertex]].z);
                    // order of transformations
                    //- sliders
                    //- arcball
                    //- center

                    v = getModelview() * v;
                    t.verts[vertex] = v;

                    Homogeneous4 n = Homogeneous4(
                        obj.normals[obj.faceNormals[face][faceVertex]].x,
                        obj.normals[obj.faceNormals[face][faceVertex]].y,
                        obj.normals[obj.faceNormals[face][faceVertex]].z,
                        0.0f);

                    n = getModelview() * n;
                    t.normals[vertex] = n;

                    Cartesian3 tex = Cartesian3(
                        obj.textureCoords[obj.faceTexCoords[face][faceVertex]].x,
                        obj.textureCoords[obj.faceTexCoords[face][faceVertex]].y,
                        0.0f);
                    t.uvs[vertex] = tex;

                    t.colors[vertex] = Cartesian3(0.7f, 0.7f, 0.7f);

                } // per vertex
                t.validate(triID++);
                if (obj.material == nullptr)
                {
                    t.shared_material = default_mat;
                }
                else
                {
                    t.shared_material = obj.material;
                }
                triangles.push_back(t);
            } // per triangle
        } // per face
    } // per object
}
