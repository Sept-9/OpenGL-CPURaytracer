#include "Ray.h"

Ray::Ray(Cartesian3 og, Cartesian3 dir,Type rayType)
{
    origin = og;
    direction = dir;
    ray_type = rayType;
}
