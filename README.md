# CENG477 HW1 — Ray Tracer

Due: Thursday, 5 November 2026, 23:59. Run with `make && ./raytracer scene.json`.

## What we are building

We are the camera. It sits at `Position`, looks along `Gaze`, and `Up` tells it which way is up.
At `NearDistance` in front of it is the image plane, bounded by `NearPlane` (left, right, bottom, top)
and cut into `Width × Height` pixels.

For every pixel we shoot one ray from the camera through the center of that pixel. `Gaze` is only
the direction of the middle of the image; each ray leans toward its own pixel.

The ray goes straight until it hits the closest surface (sphere, triangle, mesh, plane or cylinder).
If it hits nothing, the pixel gets `BackgroundColor`.

If it hits something, we ask two questions at the hit point:

1. **How much light arrives here?** A point light delivers `Intensity / d²`, where `d` is the
   distance from the light to the point. It only counts if nothing blocks the way: we send a
   *shadow ray* from the point toward the light. The start point is pushed out along the normal by
   `ShadowRayEpsilon` so the ray does not hit the surface it starts on.
2. **What does the material do with that light?**
   - **Ambient**: a constant amount of light everywhere, even in shadow.
   - **Diffuse**: matte brightness. Strongest when the light hits the surface head-on.
   - **Specular**: the shiny highlight (Blinn-Phong). It uses the half vector between "toward the
     light" and "toward the camera". `PhongExponent` sets how sharp the highlight is.
   - **Mirror**: send a new ray in the reflected direction, trace it the same way, and add its color
     scaled by `MirrorReflectance`. Stop after `MaxRecursionDepth` bounces. A reflected ray that hits
     nothing adds black, not the background.

```
color = ka·Ia + Σ over lights [ kd·E·max(0, n·l) + ks·E·max(0, n·h)^p ] + km·(color of reflected ray)
E = I / d²     l = to light     v = to camera     h = normalize(l + v)     n = surface normal
```

The sum is that pixel's RGB color. After all pixels are done, the PPM writer clamps values to 255,
rounds them and writes the image. A scene with several cameras gives several images.

## Status

| Done | Started | Not started |
|---|---|---|
| JSON parser, scene parser, PPM writer (clamps and rounds), `Vector3D`, `Color`, `Ray`, Makefile | `camera.h` (pixel → ray not finished), empty `Surface` classes in `main.cpp` | intersections, shading, render loop |

## Work split

Every file has one owner, and only the owner edits it. That is what keeps us out of merge conflicts.

### Step 0, together (~1 hour, before splitting)

Write `surface.h`: the `Surface` base class and the hit information an intersection returns
(distance `t`, hit point, unit normal, material ID). This is the only contract between the two halves.
Move the empty shape classes out of `main.cpp` while doing it.

After Step 0, the shared files (`vector3d.h`, `color.h`, `ray.h`, `surface.h`) change only through a
small separate commit, pushed right away, after telling the other person.

### Oktay: camera and light

Files: `camera.h`, `main.cpp`, `tracer.h/.cpp`

1. Finish the camera: build `u, v, w` (`v = Up`, `w = -Gaze`, `u = v × w`), compute pixel size from
   `NearPlane` and resolution, and return the ray through a pixel's center.
2. Render loop in `main.cpp`: for each camera, for each pixel, trace and store the color.
3. Closest hit: test the ray against every surface and keep the smallest positive `t`.
4. Shading: ambient, diffuse and Blinn-Phong specular with `I / d²`.
5. Shadow rays with `ShadowRayEpsilon`.
6. Mirror reflection with `MaxRecursionDepth`.
7. If renders are slow, split image rows across threads (`std::thread`).

Lights and materials are read straight from `parser::SceneConfig`.

### Ezgi: geometry

Files: `sphere.*`, `plane.*`, `triangle.*`, `mesh.*`, `cylinder.*`, `scene.*`

1. Sphere first, and push it as soon as it works: Oktay needs one working shape to test the camera
   and shading (`simple.json` has a sphere and 3 triangles).
2. Plane.
3. Triangle. Compute the normal once at load time; the counter-clockwise vertex order gives its direction.
4. Mesh: a list of triangles that share one material.
5. Cylinder: side and two caps; keep the smallest positive `t`; the normal is radial on the side and
   `±Axis` on the caps.
6. `scene.*`: build the list of surfaces from `parser::SceneConfig`. Vertex IDs start at 1.
7. Speed: a bounding box per mesh, so a ray that misses the box skips all its triangles.
   `killeroo.json` has 92k triangles and `dragon_lowres.json` 12k. If a box per mesh is not enough,
   add a BVH.

### Both, at the end

- Render every sample scene and compare with `samples_scenes/outputs`.
- Read each other's code until you can explain every line. Either of us can be interviewed about any of it.
- Package and test on an inek machine (the grader's steps, a failure costs 10 points):
  `tar -czf raytracer.tar.gz *.cpp *.h Makefile`, then `tar -xf raytracer.tar.gz && make && ./raytracer scene.json`.
- Optional bonus: write an interesting scene JSON.

### Timeline

| Day | Goal |
|---|---|
| Mon 2 Nov | Step 0 |
| Tue 3 Nov | Camera + sphere: first image of `simple.json` |
| Wed 4 Nov | All shapes and all shading terms |
| Thu 5 Nov | Speed, comparing against the sample outputs, packaging, submit |
