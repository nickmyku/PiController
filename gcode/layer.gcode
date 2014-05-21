;gcode to test add new layer parameters
;with the z axis dropping .2mm and the v axis rising .35mm the new layer seems to cover 90% of the print bed
;there may be issues with the new layer of powder draging the previous layer with it when the wiper moves to the positive extreme
;fimly bonding the part to the print bed should solve this problem
;test completed by Nicholas Mykulowycz on May 20th 2014

G1 X0.000 Y0.000
G1 X5.000 Y5.000 L100.000
G1 Z-0.20 V0.350
M10
G1 X0.000 Y0.000
