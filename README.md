# UERoadTools
Work in progress plugin for Unreal Engine to handle procedural road creation tools.

# Features
Below are a list of features supported by this plugin. 

## Road Layout Features

### Layout
Layout basic road shapes using spline component. This acts as the backbone for the road shape.

![Alt Text](Docs/Assets/DrawRoad.gif)

Resolution Control

Adjust the mesh resolution of the road. Control over U and V properties of the mesh resolution are avaiable. 

![Alt Text](Docs/Assets/RoadResolution.gif)


### Lane Control
Add and control lanes. Lanes will affect traffic simulation.

![Alt Text](Docs/Assets/LaneAdd.gif)

### Marking Control
Custom Marking Control for different marking forms.
 
![Alt Text](Docs/Assets/Marking_001.PNG)
![Alt Text](Docs/Assets/Marking_002.PNG)
![Alt Text](Docs/Assets/Marking_003.PNG)

### Terrain Snapping
Snap Road to existing terrain

### Dressing Geneneration
Add edge dressing such as barriers, pylons etc.

### Sidewalk Generation
Control over sidewalk for pedestrian simulation

### Plot Generation
Component to generate land plots either side of the road to allow for building generation

![Alt Text](Docs/Assets/PlotGeneration_001.png)


### Junction Generation

### Manual Junction Editing

Generation of geometry for complex junctions, controlled by the user.



![Alt Text](Docs/Assets/JunctionCenter.gif)

![Alt Text](Docs/Assets/JunctionLanePivot.gif)

### Automatic Junction Generation


## Traffic Simulation and ABaseTrafficVehicle

### Lane Control

### Lane Generation and ALaneSplineComponent

### Signals 

### Traffic Agent Spawning

### Traffic Agent Density Grid and ATrafficDensityGrid

### Traffic Agent Logic


# Planned Features

### Road Tools
* Road Camber and Roll - Support for Elevation across road width
* Road Micro Noise -     Support for adding bumps and random disturbance in road surface
* Lane Splitting  -      Ability to split a lane to form new lanes / half lanes
* Mesh Resolution -      Control Mesh resolution by curvature to reduce mesh complexity on flat and straight surfaces
* Mesh Width along U -   Control Road Width along U so lanes can vary in width when needed 

### Junction Controls
* Road Camber and Roll - Support for Elevation across Junction Width to match connected roads
* Junction Elevation -   Add support for proper elevation across center of junction
* SDF Junction Generation - 
* Stop Line Generation  -  Automatic Stop Line Generation
* Crossing Generation -    Automatic Pedestrian crossing generation
* Junction Center Markings - Add support for markings in junction center for turning lanes
* Junction Center Wear -     Add decal mesh generation for tyre wear for junction lanes
* Automatic Markings    -    Add correct arrows, text based on lane direction and connection
* Editor interface -         Add editor interface for editing turning lanes

