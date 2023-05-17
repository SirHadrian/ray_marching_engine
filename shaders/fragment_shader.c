#version 460 core
layout(location = 0) out vec4 FragColor;

// =========================================================================================================
// Uniforms
// =========================================================================================================

uniform float u_time;
uniform vec2 u_resolution;
uniform vec2 u_mouse;

// =========================================================================================================
// Global constants
// =========================================================================================================

#define FC gl_FragCoord
#define R u_resolution
#define T u_time
#define M u_mouse

#define MAX_MARCHING_STEPS 100.
#define PRECISION .001
#define MAX_DEPTH 100.

#define PI 3.14159265359

// =========================================================================================================
// Scene custom objects
// =========================================================================================================

struct Ray {
  vec3 ro;
  vec3 rd;
};

struct Light {
  vec3 position;
  vec3 direction;
  vec3 color;
  float intensity;
};

struct Material {
  vec3 ambientColor;  // k_a * i_a
  vec3 diffuseColor;  // k_d * i_d
  vec3 specularColor; // k_s * i_s
  float alpha;        // shininess
};

struct Mesh {
  float sdf;         // signed distance value from SDF or geometry
  Material material; // material of object
};

// =========================================================================================================
// Custom materials
// =========================================================================================================

Material gold() {
  vec3 aCol = 0.5 * vec3(0.7, 0.5, 0);
  vec3 dCol = 0.6 * vec3(0.7, 0.7, 0);
  vec3 sCol = 0.6 * vec3(1, 1, 1);
  float alpha = 5.;

  return Material(aCol, dCol, sCol, alpha);
}

Material silver() {
  vec3 aCol = 0.4 * vec3(0.8);
  vec3 dCol = 0.5 * vec3(0.7);
  vec3 sCol = 0.6 * vec3(1, 1, 1);
  float alpha = 5.;

  return Material(aCol, dCol, sCol, alpha);
}

Material background() {
  vec3 aCol = vec3(.3, .5, .9);
  vec3 dCol = vec3(0.);
  vec3 sCol = vec3(0.);
  float alpha = 1.;

  return Material(aCol, dCol, sCol, alpha);
}

Material checkerboard(vec3 p) {
  vec3 aCol = vec3(0.8 * mod(floor(p.x) + floor(p.z), 2.0)) * 0.3;
  vec3 dCol = vec3(0.1);
  vec3 sCol = vec3(0.);
  float alpha = 1.;

  return Material(aCol, dCol, sCol, alpha);
}

// =========================================================================================================
// Translations
// =========================================================================================================

//  Rotation matrix around the X axis.
mat3 rotateX(float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return mat3(vec3(1, 0, 0), vec3(0, c, -s), vec3(0, s, c));
}

// Rotation matrix around the Y axis.
mat3 rotateY(float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return mat3(vec3(c, 0, s), vec3(0, 1, 0), vec3(-s, 0, c));
}

// Rotation matrix around the Z axis.
mat3 rotateZ(float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return mat3(vec3(c, -s, 0), vec3(s, c, 0), vec3(0, 0, 1));
}

// Rotate around a circular path
mat2 rotate2d(float theta) {
  float s = sin(theta), c = cos(theta);
  return mat2(c, -s, s, c);
}

// Identity matrix.
mat3 identity() { return mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)); }

// =========================================================================================================
// SDFs
// =========================================================================================================

float planeSdf(vec3 point, vec3 orientation, float distanceFromOrigin) {
  return dot(point, orientation) + distanceFromOrigin;
}

float sphereSdf(vec3 point, vec3 offset, float radius) {
  return length(point - offset) - radius;
}

// =========================================================================================================
// Mesh operations
// =========================================================================================================

Mesh minMesh(Mesh a, Mesh b) {
  if (a.sdf < b.sdf)
    return a;
  return b;
}

float opUnion(float d1, float d2) { return min(d1, d2); }

float opSmoothUnion(float d1, float d2, float k) {
  float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
  return mix(d2, d1, h) - k * h * (1.0 - h);
}

float opIntersection(float d1, float d2) { return max(d1, d2); }

float opSmoothIntersection(float d1, float d2, float k) {
  float h = clamp(0.5 - 0.5 * (d2 - d1) / k, 0.0, 1.0);
  return mix(d2, d1, h) + k * h * (1.0 - h);
}

float opSubtraction(float d1, float d2) { return max(-d1, d2); }

float opSmoothSubtraction(float d1, float d2, float k) {
  float h = clamp(0.5 - 0.5 * (d2 + d1) / k, 0.0, 1.0);
  return mix(d2, -d1, h) + k * h * (1.0 - h);
}

float opSubtraction2(float d1, float d2) { return max(d1, -d2); }

float opSmoothSubtraction2(float d1, float d2, float k) {
  float h = clamp(0.5 - 0.5 * (d2 + d1) / k, 0.0, 1.0);
  return mix(d1, -d2, h) + k * h * (1.0 - h);
}

// =========================================================================================================
// Scene
// =========================================================================================================

Mesh scene(vec3 point) {

  float dist = sin(T) * .5 + .5 + .1;
  Mesh sphere1 = Mesh(sphereSdf(point, vec3(0., 0., 0.), dist), gold());

  // Mesh sphere2 = Mesh(sphereSdf(point, vec3(.5, 0., 0.), .3), gold());

  Mesh plane = Mesh(planeSdf(point, vec3(0., 1., 0.), 1.), checkerboard(point));

  // plane.sdf += perlinNoise(point.xz);

  const int MESH_NUMB = 2;

  // Mesh sphere = Mesh(opSmoothUnion(sphere1.sdf, sphere2.sdf, .1), silver());

  // Mesh mesh_list[MESH_NUMB] = {sphere, plane};

  Mesh mesh_list[MESH_NUMB] = {sphere1, plane};

  Mesh closest_object = Mesh(MAX_DEPTH, background());
  for (int i = 0; i < MESH_NUMB; i++) {
    closest_object = minMesh(closest_object, mesh_list[i]);
  }

  return closest_object;
}

// =========================================================================================================
// Raymarch algorithm
// =========================================================================================================

Mesh rayMarch(Ray ray) {

  float marched = 0.;
  float dist_scene = 0.;

  Mesh closest_object = Mesh(MAX_DEPTH, background());

  for (int i = 0; i < MAX_MARCHING_STEPS; i++) {

    closest_object = scene(ray.ro + marched * ray.rd);

    dist_scene = closest_object.sdf;

    marched += dist_scene;

    if (abs(dist_scene) < PRECISION || marched > MAX_DEPTH)
      break;
  }
  closest_object.sdf = marched;

  return closest_object;
}

// =========================================================================================================
// Surface normal
// =========================================================================================================

// vec3 getNormal(vec3 point) {
//
//   const float EPSILON = 0.001;
//
//   vec2 e = vec2(EPSILON, 0.0);
//   vec3 n = vec3(scene(point).sdf) - vec3(scene(point - e.xyy).sdf,
//                                          scene(point - e.yxy).sdf,
//                                          scene(point - e.yyx).sdf);
//   return normalize(n);
// }

vec3 getSurfaceNormal(vec3 p) {

  const float d0 = scene(p).sdf;
  const vec2 epsilon = vec2(.0001, 0.);
  vec3 d1 = vec3(scene(p - epsilon.xyy).sdf, scene(p - epsilon.yxy).sdf,
                 scene(p - epsilon.yyx).sdf);

  return normalize(d0 - d1);
}

// =========================================================================================================
// Shadows
// =========================================================================================================

// float softShadow(vec3 ro, vec3 rd, float mint, float tmax) {
//   float res = 1.0;
//   float t = mint;
//
//   for (int i = 0; i < 16; i++) {
//     float h = scene(ro + rd * t).sdf;
//     res = min(res, 8.0 * h / t);
//     t += clamp(h, 0.02, 0.10);
//     if (h < 0.001 || t > tmax)
//       break;
//   }
//
//   return clamp(res, 0.0, 1.0);
// }

float softShadow(vec3 ro, vec3 rd, float mint, float maxt, float w) {
  float res = 1.0;
  float t = mint;
  for (int i = 0; i < 256 && t < maxt; i++) {
    float h = scene(ro + t * rd).sdf;
    res = min(res, h / (w * t));
    t += clamp(h, 0.005, 0.50);
    if (res < -1.0 || t > maxt)
      break;
  }
  res = max(res, -1.0);
  return 0.25 * (1.0 + res) * (1.0 + res) * (2.0 - res);
}

// =========================================================================================================
// Ambient oclusion
// =========================================================================================================

float ambientOcclusion(vec3 p, vec3 normal) {
  float occ = 0.0;
  float weight = 1.0;
  for (int i = 0; i < 8; i++) {
    float len = 0.01 + 0.02 * float(i * i);
    float dist = scene(p + normal * len).sdf;
    occ += (len - dist) * weight;
    weight *= 0.85;
  }
  return 1.0 - clamp(0.6 * occ, 0.0, 1.0);
}

// =========================================================================================================
// Lighting
// =========================================================================================================

vec3 phongLight(vec3 point, Ray ray, Material object_material, Light light) {

  vec3 surface_normal = getSurfaceNormal(point);

  // ambient
  float k_a = 0.6;
  vec3 ambient = k_a * object_material.ambientColor;

  // diffuse
  float k_d = 0.5;
  float dotLN = clamp(dot(light.direction, surface_normal), 0., 1.);
  vec3 diffuse = k_d * dotLN * object_material.diffuseColor;

  // specular
  float k_s = 0.6;
  float dotRV =
      clamp(dot(reflect(light.direction, surface_normal), ray.rd), 0., 1.);
  vec3 specular =
      k_s * pow(dotRV, object_material.alpha) * object_material.specularColor;

  // Shadows
  float soft_shadow =
      clamp(softShadow(point, light.direction, 0.02, 5.0, .3), 0.0, 1.0);

  // Raymarching simple shadow
  // Ray shadow_ray = Ray(point + surface_normal * .02, light.direction);
  // float dist = rayMarch(shadow_ray).sdf;
  //
  // if (dist < length(light.position - point))
  //   return ambient;

  // Ambient occlusion
  float ambient_occlusion = ambientOcclusion(point, surface_normal);

  // Light reflect from objects
  vec3 reflect_back = .05 * object_material.ambientColor *
                      clamp(dot(surface_normal, light.direction), 0., 1.);

  return (reflect_back + ambient) * ambient_occlusion +
         (specular * ambient_occlusion + diffuse) * soft_shadow;
}

// =========================================================================================================
// Scene lights
// =========================================================================================================

vec3 sceneLights(vec3 point, Material object_material, Ray ray) {

  vec3 color = vec3(0.);

  // light #1
  vec3 light_position1 = vec3(sin(T * 3.) + 1., 5., cos(T * 3.) + 0.);
  vec3 light_direction1 = normalize(light_position1 - point);
  vec3 light_color1 = vec3(1., 1., 1.);
  float light_intensity1 = 0.9;

  Light light1 =
      Light(light_position1, light_direction1, light_color1, light_intensity1);

  // light #2
  vec3 light_position2 = vec3(5., 3., -3.);
  vec3 light_direction2 = normalize(light_position2 - point);
  vec3 light_color2 = vec3(1., 1., 1.);
  float light_intensity2 = 0.7;

  Light light2 =
      Light(light_position2, light_direction2, light_color2, light_intensity2);

  // Add lights
  const int LIGHTS_NUMBER = 1;
  // Light lights[LIGHTS_NUMBER] = {light1, light2};
  Light lights[LIGHTS_NUMBER] = {light1};

  for (int i = 0; i < LIGHTS_NUMBER; i++) {

    color += lights[i].intensity *
             phongLight(point, ray, object_material, lights[i]) *
             lights[i].color;
  }

  return color;
}

// =========================================================================================================
// Camera translations
// =========================================================================================================

mat3 camera(vec3 ro, vec3 lookAt) {
  vec3 cd = normalize(lookAt - ro);                 // camera direction
  vec3 cr = normalize(cross(vec3(0., 1., 0.), cd)); // camera right
  vec3 cu = normalize(cross(cd, cr));               // camera up

  return mat3(-cr, cu, -cd);
}

// =========================================================================================================
// Render objects and lights
// =========================================================================================================

vec3 render(vec2 uv, vec2 mp) {

  vec3 background = background().ambientColor;

  vec3 ro = vec3(0., .5, 3.);
  vec3 lookAt = vec3(0., 0., 0.);

  // Make camera to center on lookAt point
  vec3 rd = camera(ro, lookAt) * normalize(vec3(uv, -1.0));
  // Look around with mouse
  rd *= rotateY(mp.x) * rotateX(mp.y);

  Ray ray = Ray(ro, rd);

  // Shoot the rays and get hit scene object
  Mesh closest_object = rayMarch(ray);

  // If the closest_object sdf is smaller than the MAX_DEPTH then we hit a scene
  // object else we hit the "background object".
  if (closest_object.sdf < MAX_DEPTH) {

    // Get the point where the ray hit the surface of an object
    vec3 point = ray.ro + closest_object.sdf * ray.rd;

    vec3 light = sceneLights(point, closest_object.material, ray);

    // Fog
    return mix(light, background,
               1. - exp(-.001 * closest_object.sdf * closest_object.sdf));
  }

  // Fog height
  return background - max(.9 * ray.rd.y, 0.);
}

// =========================================================================================================
// Anti-Aliasing AAx4
// =========================================================================================================

vec2 offsetUV(vec2 offset) { return (2. * (FC.xy + offset) - R.xy) / R.y; }

vec3 AAx4(vec2 mp) {
  vec4 offset = vec4(.125, -.125, .375, -.375);
  vec3 aax4 = render(offsetUV(offset.xz), mp) +
              render(offsetUV(offset.yw), mp) +
              render(offsetUV(offset.wx), mp) + render(offsetUV(offset.zy), mp);
  return aax4 / 4.;
}

void main() {

  // vec2 uv = FC.xy / R.xy;
  // uv -= .5;
  // uv.x *= R.x / R.y;

  vec2 mp = M.xy / R.xy;
  mp.xy = 1. - mp.xy;
  mp -= .5;
  mp.x *= R.x / R.y;

  vec3 color = vec3(0.);
  // color = AAx4(mp);
  color = render(offsetUV(vec2(0.)), mp);

  // Gamma correction
  color = pow(color, vec3(.4545));

  FragColor = vec4(color, 1.);
}
