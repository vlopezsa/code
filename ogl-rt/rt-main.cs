#version 430

uniform float winWidth;
uniform float winHeight;

layout (rgba32f, binding=0) uniform image2D destTex;
layout (local_size_x = 16, local_size_y = 16) in;

struct ray
{
    vec3 orig;
    vec3 dir;
};

struct sphere
{
    vec3 center;
    float radius;
};

struct hit
{
    float t;
    vec3 p;
    vec3 normal;
};


sphere sphereList[2] = {
    {vec3(0.0,    0.0, -1.0),  0.5},
    {vec3(0.0, -100.5, -1.0), 100.0}
    };

vec3 pointAlongRay(ray r, float t)
{
    return r.orig + t * r.dir;
}

vec4 bgColor(ray r)
{
    vec3 udir = normalize(r.dir);

    float t = 0.5 * (udir.y + 1.0);

    vec3 color = (1.0 - t) * vec3(1.0, 1.0, 1.0) + 
             t * vec3(0.5, 0.7, 1.0);

    return vec4(color.xyz, 1.0);
}

bool hitSphere(sphere s, ray r, float t_min, float t_max, out hit rec)
{
    vec3 oc = r.orig - s.center;

    float a = dot(r.dir, r.dir);
    float b = dot(oc, r.dir);
    float c = dot(oc, oc) - (s.radius * s.radius);
    float d = b * b -  a * c;

    if (d > 0)
    {
        float temp = (-b - sqrt(d)) / a;
        if(temp<t_max && temp>t_min)
        {
            rec.t = temp;
            
            vec3 p = pointAlongRay(r, temp);
            rec.normal = (p - s.center) / s.radius;

            rec.p = p;
            return true;
        }
        temp = (-b + sqrt(d)) / a;
        if (temp < t_max && temp > t_min)
        {
            rec.t = temp;

            vec3 p = pointAlongRay(r, temp);
            rec.normal = (p - s.center) / s.radius;

            rec.p = p;
            return true;
        }
    }

    return false;
}

bool hitable(ray r, float t_min, float t_max, out hit rec)
{
    hit trec;
    bool fhit = false;

    float closest = t_max;

    for(int i=0; i<sphereList.length(); i++)
    {
        if(hitSphere(sphereList[i], r, t_min, closest, trec)==true)
        {
            fhit = true;
            closest = trec.t;
            rec = trec;
        }
    }

    return fhit;
}

vec4 render(ray r)
{
    vec4 color;
    hit rec;

    bool n = hitable(r, 0.0, 100000000.0, rec);

    if (n == true)
    {
        vec3 c = rec.normal + vec3(1.0, 1.0, 1.0);
        color = vec4(0.5 * c, 1.0);
    }
    else
        color = bgColor(r);

    return color;
}

void main() {
    vec2 storePos = vec2(gl_GlobalInvocationID.xy);

    float u, v, x, y;

    ray r;

    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    r.orig = vec3(0.0, 0.0, 0.0);

    x = gl_GlobalInvocationID.x;
    y = gl_GlobalInvocationID.y;

    u = (x + 0.5) / winWidth;
    v = (y + 0.25) / winHeight;
    r.dir = vec3(-2.0, -1.0, -1.0) + u * vec3(4.0, 0.0, 0.0) + v * vec3(0.0, 2.0, 0.0);
    color += render(r);

    u = (x + 0.25) / winWidth;
    v = (y + 0.5) / winHeight;
    r.dir = vec3(-2.0, -1.0, -1.0) + u * vec3(4.0, 0.0, 0.0) + v * vec3(0.0, 2.0, 0.0);
    color += render(r);

    u = (x + 0.75) / winWidth;
    v = (y + 0.5) / winHeight;
    r.dir = vec3(-2.0, -1.0, -1.0) + u * vec3(4.0, 0.0, 0.0) + v * vec3(0.0, 2.0, 0.0);
    color += render(r);

    u = (x + 0.5) / winWidth;
    v = (y + 0.75) / winHeight;
    r.dir = vec3(-2.0, -1.0, -1.0) + u * vec3(4.0, 0.0, 0.0) + v * vec3(0.0, 2.0, 0.0);
    color += render(r);

    color /= 4.0;

    imageStore(destTex, ivec2(storePos), color);
}