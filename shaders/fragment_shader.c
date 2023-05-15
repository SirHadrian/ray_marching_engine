#version 460 core
layout(location = 0) out vec4 FragColor;

uniform float u_time;
uniform vec2 u_resolution;
uniform vec2 u_mouse;

#define FC gl_FragCoord
#define R u_resolution
#define T u_time
#define M u_mouse

#define MAX_MARCHING_STEPS 100.
#define PRECISION 0.01
#define MAX_DEPTH 50.

struct Ray {
  vec3 ro;
  vec3 rd;
};

struct Object {
  float sdf;
  vec3 color;
};

struct Light {
  vec3 position;
  vec3 direction;
  vec3 color;
  float intensity;
};

float planeSdf(vec3 point, vec3 orientation, float distanceFromOrigin) {
  return dot(point, orientation) + distanceFromOrigin;
}

float sphereSdf(vec3 point, vec3 offset, float radius) {
  return length(point - offset) - radius;
}

Object minObject(Object a, Object b) {
  if (a.sdf < b.sdf)
    return a;
  return b;
}

Object scene(vec3 point) {

  Object sphere =
      Object(sphereSdf(point, vec3(0., 0., -1.), .1), vec3(1., 0., 0.));
  Object plane =
      Object(planeSdf(point, vec3(0., 1., 0.), 1.),
             vec3(.2 + .4 * mod(floor(point.x) + floor(point.z), 2.)));
  Object sphere2 =
      Object(sphereSdf(point, vec3(.5, 0., -.5), .1), vec3(0., 1., 0.));
  const int OBJS_NUM = 3;

  Object objs[OBJS_NUM] = {sphere, sphere2, plane};

  Object closest_object = Object(MAX_DEPTH, vec3(1.));
  for (int i = 0; i < OBJS_NUM; i++) {
    closest_object = minObject(closest_object, objs[i]);
  }

  return closest_object;
}

Object rayMarch(Ray ray) {

  float marched = 0.;
  float dist_scene = 0.;
  float steps = 0.;

  Object closest_object = Object(MAX_DEPTH, vec3(1.));

  while (steps < MAX_MARCHING_STEPS) {

    closest_object = scene(ray.ro + marched * ray.rd);

    dist_scene = closest_object.sdf;

    marched += dist_scene;

    if (dist_scene < PRECISION || marched > MAX_DEPTH)
      break;

    steps += 1.;
  }

  closest_object.sdf = marched;
  return closest_object;
}

vec3 getNormal(vec3 p) {

  const float EPSILON = 0.001;

  vec2 e = vec2(EPSILON, 0.0);
  vec3 n = vec3(scene(p).sdf) - vec3(scene(p - e.xyy).sdf, scene(p - e.yxy).sdf,
                                     scene(p - e.yyx).sdf);
  return normalize(n);
}

float softShadow(vec3 ro, vec3 rd, float mint, float tmax) {
  float res = 1.0;
  float t = mint;

  for (int i = 0; i < 16; i++) {
    float h = scene(ro + rd * t).sdf;
    res = min(res, 8.0 * h / t);
    t += clamp(h, 0.02, 0.10);
    if (h < 0.001 || t > tmax)
      break;
  }

  return clamp(res, 0.0, 1.0);
}

vec3 phongReflection(vec3 point, vec3 surface_normal, Ray ray,
                     vec3 object_color, Light light) {

  // ambient
  float k_a = 0.6;
  vec3 i_a = object_color;
  vec3 ambient = k_a * i_a;

  // diffuse
  float k_d = 0.5;
  float dotLN = clamp(dot(light.direction, surface_normal), 0., 1.);
  vec3 i_d = light.color;
  vec3 diffuse = k_d * dotLN * i_d;

  // specular
  float k_s = 0.6;
  float dotRV =
      clamp(dot(reflect(light.direction, surface_normal), ray.rd), 0., 1.);
  vec3 i_s = vec3(1, 1, 1);
  float alpha = 10.;
  vec3 specular = k_s * pow(dotRV, alpha) * i_s;

  // Shadows
  float softShadow =
      clamp(softShadow(point, light.direction, 0.02, 2.5), 0.1, 1.0);

  return (ambient + specular) + diffuse * softShadow;
}

vec3 light(vec3 point, vec3 object_color, Ray ray) {

  vec3 surface_normal = getNormal(point);

  // light #1
  vec3 light_position1 = vec3(1., 9., 5.);
  vec3 light_direction1 = normalize(light_position1 - point);
  vec3 light_color1 = vec3(1., 1., 1.);
  float light_intensity1 = 0.6;

  Light light1 =
      Light(light_position1, light_direction1, light_color1, light_intensity1);

  // light #2
  vec3 light_position2 = vec3(9., 3., -5.);
  vec3 light_direction2 = normalize(light_position2 - point);
  vec3 light_color2 = vec3(0., 0., 0.);
  float light_intensity2 = 0.7;

  Light light2 =
      Light(light_position2, light_direction2, light_color2, light_intensity2);

  vec3 color = vec3(0.);

  color += light1.intensity *
           phongReflection(point, surface_normal, ray, object_color, light1);

  color += light2.intensity *
           phongReflection(point, surface_normal, ray, object_color, light2);

  return color;
}

vec3 render(vec2 uv) {

  vec3 background = vec3(.5);

  vec3 ro = vec3(0., 0., 1.);
  vec3 rd = normalize(vec3(uv, -1.));

  Ray ray = Ray(ro, rd);

  Object closest_object = rayMarch(ray);
  float dist = closest_object.sdf;
  vec3 object_color = closest_object.color;

  if (dist < MAX_DEPTH) {
    vec3 point = ray.ro + dist * ray.rd;

    vec3 light = light(point, object_color, ray);

    // Fog
    return mix(light, background,
               1. - exp(-.001 * closest_object.sdf * closest_object.sdf));
  }

  // Fog height
  return background - max(.9 * ray.rd.y, 0.);
}

void main() {

  vec2 uv = (FC.xy - .5 * R.xy) / R.y;

  vec3 color = vec3(0.);
  color = render(uv);

  // Gamma correction
  color = pow(color, vec3(.4545));

  FragColor = vec4(color, 1.);
}
