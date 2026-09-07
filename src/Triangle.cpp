#include "Triangle.h"
#include <math.h>
#include <algorithm>
#define M_PI 3.14159265358979323846

Triangle::Triangle()
{
    triangle_id = -1;
    shared_material = nullptr;
}

void Triangle::validate(int id)
{
    triangle_id = id;
}

bool Triangle::isValid()
{
    return triangle_id != -1;
}
float Triangle::intersect(Ray r)
{
    Cartesian3 p = verts[0].Point();
    Cartesian3 q = verts[1].Point();
    Cartesian3 PointR = verts[2].Point();
    Cartesian3 u = q - p;
    Cartesian3 v = PointR - p;
    Cartesian3 n = u.cross(v);
    Cartesian3 s = r.origin;
    Cartesian3 l = r.direction;
    float denom = l.dot(n);
    if (fabs(denom) == 0.0f)
    {
        return -1.0f;
    }
    float t = (p - s).dot(n) / denom;
    if (t < 0.0f)
    {
        return -1.0f;
    }
    Cartesian3 o = s + l * t;
    Cartesian3 n_norm = n.unit();
    Cartesian3 u_norm = u.unit();
    Cartesian3 w_norm = n_norm.cross(u_norm).unit();
    Cartesian3 o_line = o - p;
    float o_u = o_line.dot(u_norm);
    float o_w = o_line.dot(w_norm);
    // p_u p_w will be all 0
    Cartesian3 q_line = q - p;
    float q_u = q_line.dot(u_norm);
    float q_w = q_line.dot(w_norm);
    Cartesian3 r_line = PointR - p;
    float r_u = r_line.dot(u_norm);
    float r_w = r_line.dot(w_norm);
    // half-plane test
    auto sign = [](float p_u, float p_w, float q_u, float q_w, float r_u, float r_w)
    {
        return (p_u - r_u) * (q_w - r_w) - (q_u - r_u) * (p_w - r_w);
    };

    float d1 = sign(o_u, o_w, 0.0f, 0.0f, q_u, q_w);
    float d2 = sign(o_u, o_w, q_u, q_w, r_u, r_w);
    float d3 = sign(o_u, o_w, r_u, r_w, 0.0f, 0.0f);

    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    if (!(has_neg && has_pos))
    {
        return t;
    }
    return -1.0f;
}
Cartesian3 Triangle::baricentric(Cartesian3 o)
{
    // TODO: Input is the intersection between the ray and the triangle.
    // o = origin + direction*t;
    Cartesian3 bc;
    Cartesian3 p = verts[0].Point();
    Cartesian3 q = verts[1].Point();
    Cartesian3 r = verts[2].Point();
    Cartesian3 v0 = q - p;
    Cartesian3 v1 = r - p;
    Cartesian3 v2 = o - p;
    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);
    float denom = d00 * d11 - d01 * d01;
    float beta = (d11 * d20 - d01 * d21) / denom;
    float gamma = (d00 * d21 - d01 * d20) / denom;
    float alpha = 1.0f - beta - gamma;
    bc = Cartesian3(alpha, beta, gamma);
    return bc;
}
Homogeneous4 Triangle::BlinnPhongLight(Homogeneous4 lightPosition, Homogeneous4 lightColor, Cartesian3 o)
{
    Cartesian3 bari = baricentric(o);
    Cartesian3 n = (normals[0].Vector() * bari.x + normals[1].Vector() * bari.y + normals[2].Vector() * bari.z).unit();
    Cartesian3 vl = (lightPosition.Point() - o).unit();
    Cartesian3 vv = (Cartesian3(0, 0, 0) - o).unit();
    Cartesian3 h = (vl + vv).unit();
    float cosTheta = std::clamp(n.dot(vl), 0.0f, 1.0f);
    float spFactor = std::clamp(n.dot(h), 0.0f, 1.0f);
    Cartesian3 TempS = Cartesian3(0, 0, 0);
    float Sp = pow(spFactor, shared_material->shininess);
    TempS = shared_material->specular * Sp * cosTheta * (shared_material->shininess + 2) / (2 * M_PI);
    Cartesian3 TempA = shared_material->ambient;
    Cartesian3 TempD = shared_material->diffuse * cosTheta;
    Cartesian3 TempE = shared_material->emissive;
    Homogeneous4 Total = Homogeneous4(lightColor.x * (TempS.x + TempD.x + TempA.x) + TempE.x,
                                      lightColor.y * (TempS.y + TempD.y + TempA.y) + TempE.y,
                                      lightColor.z * (TempS.z + TempD.z + TempA.z) + TempE.z, 1.0f);
    return Total;
}
Homogeneous4 Triangle::ShadowLight(Homogeneous4 lightColor)
{
    Cartesian3 TempA = shared_material->ambient;
    Cartesian3 TempE = shared_material->emissive;
    Homogeneous4 Total = Homogeneous4(lightColor.x * (TempA.x) + TempE.x,
                                      lightColor.y * (TempA.y) + TempE.y,
                                      lightColor.z * (TempA.z) + TempE.z, 1.0f);
    return Total;
}