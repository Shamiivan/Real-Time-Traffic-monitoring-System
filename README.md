## Project summary

The project is a simulation of a simplified air traffic control system (ATC) which monitors and coordinates air traffic in a 3D air space.
It was developed using QNX and run on a single core virtual machine.

The 3D airspace is a 100,000 by 100,000 by 25,000 unit rectangle in which planes will operate.
Planes must maintain a minimum separation distance of 1000 units vertically and 3000 units horizontally.

When the program starts, planes enter the airspace and begin moving as their velocity vector dictates.
A periodic thread will check that each plane is not close enough to collide with another plane.
If a potential violation is detected, the velocity vector of one of the two planes will be changed to avoid the collision.

## Tutorial

### Initializing planes

Planes are read from "planes.txt" and each plane has a starting position and a velocity vector, each with XYZ components.
the first element in a row is the unique ID of a plane.
the following 3 numbers are the starting position.
the last 3 numbers are the velocity vector.
A simple, single digit integer should be used as the value will be multiplied by 1000. This is to make the text file more readable.

### Operator commands

Typing "help" will display a list of commands the user can enter into the terminal.

Commands: 1) Display data of all planes
          2) Display data of a specific plane
          3) Change a specific plane's trajectory

### Reading the grid
A 2D grid will be displayed in the terminal periodically with different characters showing the positions of various planes on the X and Y axis.
Different characters are used to denote a plane's general altitude.

"V" meaning low altitude
"#" meaning medium altitude
"^" meaning high altitude
