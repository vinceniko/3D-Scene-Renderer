# 3D Scene Editor

Vincent Nikolayev  
Professor Silva  
Interactive Computer Graphics  
11/18/20  

![1](images/1.png)

A 3D Scene Editor that can:

* Load triangulated 3D models from .off files
* View the models
  * Two cameras are available
    * One that moves along the x, y, z axes and always points to the origin of world space
    * A trackball camera that moves along a sphere and always points to the origin of world space
* Transform the models
  * Translate
  * Rotate
  * Scale
* Shade the models
  * Flat shading
  * Phong shading
* Draw debug info
  * Wireframe
  * Mesh normals
* Hot-reload shaders
  * Watched shader files are polled from another thread and reloaded automatically

## Installation

This project uses gitmodules for dependency management. After cloning the repo, run `git submodule update --init --recursive` to download the necessary dependencies.

This project uses CMake for building. A brief getting started guide for creating a build with CMake follows:

1. Make a new directory for storing the build
2. `cd` into the directory
3. Run `cmake ..`
4. Run `make`
5. Finally, run the `3DSceneEditor_bin` executable

Refer to `CMakeLists.txt` for the build configuration and necessary dependencies.

## Tasks

### 1.1: Scene Editor

Adding and removing objects to the scene:

![insertion_deletion](images/insertion_deletion.gif)

Directions:

* Key `1` inserts a unit cube at the origin with unit scale
* Key `2` inserts a bumpy cube at the origin with unit scale
* Key `3` inserts a bunny at the origin with unit scale

### 1.2: Model Control

#### Model Transformations

![trans](images/trans.gif)

Directions:

* Clicking on an model with the mouse selects it. The color changes to the inverse of the model's color to indicate that it is selected.
* It is possible to transform the selected model as such (all transformations are relative to the view direction):
  * Translate
    * Key `a` translates the model left, or in the -x direction
    * Key `d` translates the model right, or in the +x direction
    * Key `w` translates the model up, or in the +y direction
    * Key `s` translates the model down, or in the -y direction
    * Key `-` translates the model towards the camera, or in the +z direction
    * Key `=` translates the model away from the camera, or in the -z direction
  * Rotate
    * Key `t` rotates the model along the x axis up
    * Key `y` rotates the model along the x axis down
    * Key `f` rotates the model along the y axis left
    * Key `g` rotates the model along the y axis right
    * Key `h` rotates the model along the z axis left
    * Key `j` rotates the model along the z axis right
  * Scale
    * Key `k` uniformly scales the model up
    * Key `l` uniformly scales the model down

#### Draw and Shading Modes ("Render Setting")

![modes](images/modes.gif)

Directions:

* Cycle through the following available drawing modes with key `m`:
  * Shading only
  * Shading and Wireframe
  * Wireframe only
  * Shading and normals
* Cycle through the following available shading modes with key `p` (be sure to be in a draw mode that supports shading):
  * Phong Shading (technically Blinn-Phong)
  * Flat Shading

### 1.3: Camera Control

#### Free Camera

![free_camera](images/free_camera.gif)

Directions:

* Pan along the x and y axes in world space by clicking and dragging the mouse in the relevant direction
* Move along the z axis in world space by scrolling with the mouse wheel
* Switch to the orthographic projection mode with key `c`

##### Orthographic Projection

![ortho](images/ortho.gif)

Directions:

* The other features work identically in the orthographic projection mode

##### Aspect Ratio

![aspect](images/aspect.gif)

* The aspect ratio is preserved such that the model's scale is not deformed if the window is resized
* Changing the window size preserves all functionality

### 1.5: Trackball Camera

![trackball](images/trackball.gif)

Directions:

* Switch to the Trackball Camera by pressing key `c`.
* The mouse controls are identical to the Free Camera except now the camera moves along a sphere.

Note: Orthographic projection for the trackball camera has an additional zoom feature using the scroll wheel that just scales the view.
