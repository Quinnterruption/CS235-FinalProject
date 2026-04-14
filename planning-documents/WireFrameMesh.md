meshes could be stored as a list of points and a list of points to connect

for a simple square, it could be
```
points = {{0,0}, {1,0}, {1,1}, {0,1}}
connections = {{0,1}, {1,2}, {2,3}, {3,0}}
```
to render as two triangles
```
connections = {{0,1}, {1,2}, {2,3}, {3,0}, {0,2}}
```
alternatively, each mesh could store a set of triangles

the rendering method will draw all triangles and discard overlapping points plus/minus a threshold
```
square = {triangle1, triangle2}

triangle1 = {{0,0}, {1,0}, {0,1}}
triangle2 = {{1,0}, {0,1}, {1,1}}
```
the program would render the two triangles and only draw the line between {1,0} and {0,1} once.

for a cube, it could be stored in an obj format
```
cube = {tri1, tri2, tri3, tri4, ..., tri11, tri12}

vert1 = {0, 0, 0}
vert2 = {0, 1, 0}
vert3 = {0, 1, 1}
etc.
tri1 = {vert1, vert2, vert3}
etc.
```