## Procedural Methods
# LAB 5: L-SYSTEMS
### Gaz Robinson
### School of Design and Informatics
# Introduction
So far, we’ve focused on CPU Vertex manipulation, today we’re moving away a little to look at a different branch of procedural content generation: Procedural Growth and L-Systems!

Get this week’s code from MyLearningSpace and build + run it; you should see a hexagonal outline. 
![image](https://user-images.githubusercontent.com/5627386/137639324-db59db23-8661-47d0-9430-4665eff584bf.png)

The shape is the result of a string being interpreted as though it were the output from an L-System. It is built from one of two actions: Draw a line or turn left.

Your task today is to build up a class for managing and iterating L-Systems, and then to interpret these systems into some sort of output.

If you have any problems, ask in the lab, or contact me (g.robinson@abertay.ac.uk). 
 
# The Code
The code is the same framework that you should be growing familiar with, but there are three new classes: LSystem, LineMesh and CylinderMesh. 
## LSystem
LSystem is small skeleton class that is intended to encapsulate the string rewriting functionality of an LSystem. 
It contains strings to represent the Axiom (starting state) and the current state, as well as a selection of functions that… don’t do anything right now!
Your first task is to fill in these functions and build your own L-System.

You’ll need some sort of structure to represent and store the rules first of all. Right now there’s an “Add Rule” function that takes a Char and a string; so it will probably need to use those somehow!
Hint: If you’ve never used it before, why not take the time today to look at the std::map class?

Then, you’ll need build the Iterate function; this is the method that runs through the string, checking each symbol and applying the rules as necessary. 
It should replace any matching symbols with the appropriate successor substitution.

The IMGUI window has functionality for viewing the current system string and for activating the Iterate function, which will come in handy for testing.
## LineMesh
The LineMesh class provides functionality for drawing lines in 3D as a list of line segments drawn in one go. 
You shouldn’t need to change much in here; simply provide a list of segments and then call the BuildLine function to construct the vertices ready for drawing.
The BuildLine2D function in App1.cpp has an example of its use.
## CylinderMesh
The CylinderMesh class will draw an uncapped (no top or bottom) cylinder according to the initialisation parameters.
The cylinder mesh also has an internal m_Transform variable, which will allow you to store the rotation/positional matrix you might create when building a more complex L-System structure like the 3D tree.
By utilising this mesh, you should be able to create a more complex tree structure.
 
# Tasks
## #1 Build an L-System Class
Follow the guidance from the code overview section above to fill-in the provided skeleton class (or build your own). Test it with an L-System and ensure it provides the expected output.
Wikipedia has a great sample of these.
## #2 Branching Binary Tree
Once your L-System class is functional, build an L-system that makes use of save and restore symbols (typically ‘[‘ and ‘]' respectively) to generate a fractal binary tree.
Remember you’ll need to:
1.	Set up the L-system with the correct rules, symbols and axiom 
2.	Change how the symbols are interpreted within the BuildLine2D function (or build a new function).
![image](https://user-images.githubusercontent.com/5627386/137639535-55179d18-af8b-422d-b0c2-14bc55532394.png)


The usual approach for this system is:  
**A** : draw a line segment (more advanced systems would also draw a leaf!)  
**B** : draw a line segment & move “forward”  
**[**  : Push the current position and rotation AND turn left 45 degrees  
**]**  : Return the position and rotation to the saved values and pop them off the stack, then turn right 45 degrees  

You’re looking to get something like the below. (0, 1, 2, and 3 iterations)
![image](https://user-images.githubusercontent.com/5627386/137639408-588fffc6-0318-4a6c-b7cf-20842e3c1d17.png)

**Note** The L-Systems tend to grow exponentially, so you might want to avoid going beyond 8 or so iterations…

### Bonus task
Have a look for some other L-systems, on Wikipedia or elsewhere, and try implementing one of them!

## #3 3D Tree
2D trees won’t do our terrain a lot of good, so we should look at expanding our system into 3D space.
Use the lecture notes, and the example system here to build an appropriate L-system.

To build the tree from the string, you’ll need to keep track of your position and the sequence of rotations so far.
This will likely require some smart matrix manipulation, so think about planning it out beforehand.

![image](https://user-images.githubusercontent.com/5627386/137639526-84205a6d-9bc8-43b7-9b5d-7f2b01685c68.png)

It’s also recommended you only iterate the L-system rules once or twice during development, so you don’t have to debug an entire tree when it’s not working how you expect it to. 
If it works correctly for three iterations, it should work for more

### Bonus Task – Stochastic L-System
When moving and rotating the branches, move or rotate them a random or pseudo-random amount. Doing this will give you a much more ‘realistic’ looking tree.

### Bonus Task – ‘Tree’-D
Once you have a 3D tree, it’s still not well suited to our terrain because it’s made entirely out of lines!
The next improvement would be to render the tree with something more complex than lines!
As a start, you could replace the line segments with cylinders that you can control the thickness of.
If you build the tree with line segments and matrix transformations, you should be able to swap them out for a different mesh!
