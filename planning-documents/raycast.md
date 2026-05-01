Take in coordinates of the click -> store in a pair<int, int>

for all wireFrames, check if the click at wireFrame.getMidpoint().z intersects with the wireFrame 
this will essentially reverse project the click to be at wireFrame.getMidpoint().z

(sphere can use spherical collision detection)(all others will use a minimum size bounding box)
