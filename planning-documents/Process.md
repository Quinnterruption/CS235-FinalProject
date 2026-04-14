The rendering pipeline will take in values from an obj file

It will create an object based off the vertices and faces. These objects will be stored in memory

---
After pulling in all necessary assets/objects, transformations and rotations will be calculated

Each object will calculate its projected coordinates only if it is actually in front of the screen

Next, a triangle rasterization algorithm will "draw" any pixels that are inside the object's projected coordinates.
This will loop through all the projected triangles of an object.

Finally, after all the points are stored in the buffer, it will be drawn to the screen using 
CreateDIBSection, BitBlt, StretchBlt, and more.