# TODO

* Rotate objects
* mesh spawning
```
// mesh spawn
case GLFW_KEY_1:
    ctx->mesh_ctx.push(UnitCube{});
    break;
```
* shading
* aspect scaling
* rename mesh entity to mesh instance?

## Bugs

* prevent segfault on transforming no selection

## For Writeup

* VBO is binded once for each mesh and not once for each instance