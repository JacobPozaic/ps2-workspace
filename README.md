# This is a collections of playstation 2 development projects I have been playing around with.

## Threading:

This project is an attempt to reverse engineer the threading system calls to properly document their use, operation, and exceptions.  It is unfortunate that the PS2SDK and the PS2 hardware manuals do not seem to explain threading operations to any degree, and online documentation appears to be non-existant.

It is my intention with this project to create a small library that wraps the threading system calls.  This library will provide in-depth documentation on how to properly use multithreaded functionality, and provide some more useful functionality to control thread use.

## OBJLoader:

This project is a library I am developing to load wavefront obj files from any storage medium connected to the PS2 so that meshes do not have to be hard-coded into the ELF executable.  

I plan to add the ability to select what properties of the OBJ should not be included to save on memory and parsing time.  I also plan to make an obj pre-processor that will strip comments and features not supported by PS2, and might go sofar as to generate custom binary mesh file types that can be parsed much faster and more effeciently.

## Demo1:

This project is an experiment with the rendering pipeline built into PS2SDK, and the integration of IOP modules.  As the project stands it shows a cube rotating and bouncing around in 3D space, the d-pad on the controler can move the camera's position to a moderate degree, the camera can also be set to follow or stop following the cube by pressing the cross button.

I plan to create an asyncronous event system (using the threading library I am working on) to handle controller input.  I would also like to increase the complexity of the rendered envitoment, and will be creating a character that can navigate more fluidly with conventional camera control.  Long term I plan to extend the project into a basic game.
	
# Things you should probably know about these projects:
projects are licenced under Academic Free License version 2.0
The full license can be found in the LICENSE file

Some portions of code are from examples or other projects related to:
PS2SDK - https://github.com/ps2dev/ps2sdk
GSKIT - https://github.com/ps2dev/gsKit
Open PS2 Loader - https://github.com/ifcaro/Open-PS2-Loader

Some projects may include incomplete notice of authors, licensing, ect. as this is not a
product for distribution, just some code I have been playing with.