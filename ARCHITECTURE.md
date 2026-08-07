# ofxLatk Architecture

`ofxLatk` is an openFrameworks addon for reading, writing, and rendering Latk files (a format for 3D strokes/animations). 

The architecture is built around a hierarchical data model representing 3D animation, coupled with JSON parsing capabilities.

## Data Model Hierarchy

The core of the addon is a nested set of classes that represent the structure of a Latk file. This structure models traditional 2D animation translated into 3D space:

*   **`Latk`**: The root class representing a complete Latk animation file. 
    *   It manages the overall playback state (`checkInterval()`, `run()`), tracking time/frames, and controls a collection of `LatkLayer`s.
    *   It contains the main `read(string fileName)` and `write(string fileName)` logic, which translates the in-memory hierarchy to and from the Latk JSON format.
*   **`LatkLayer`**: Represents a distinct layer within the animation.
    *   It contains a sequential vector of `LatkFrame`s and tracks the `currentFrame` being played.
*   **`LatkFrame`**: Represents a single snapshot in time for a given layer. 
    *   It holds a collection of `LatkStroke`s that are drawn during this frame.
*   **`LatkStroke`**: The atomic visual element.
    *   Contains a vector of 3D points (`ofVec3f`) that define the path of the stroke.
    *   Stores visual properties such as `strokeColor` and `strokeSize`.
    *   Provides utility functions for manipulating the stroke data, such as `splitStroke()`, `smoothStroke()`, and `refine()`.

## Directory Structure & Components

*   **`src/`**: Contains `ofxLatk.h`, which simply acts as an aggregate include file for an openFrameworks project using the addon.
*   **`libs/ofxLatk/`**: Contains the core logic and classes.
    *   `include/ofxLatk/` and `src/`: Contain the source code for the hierarchical data model (`Latk`, `LatkLayer`, `LatkFrame`, `LatkStroke`).
*   **`libs/jsoncpp/`**: An embedded third-party library for parsing and writing JSON files.

## I/O and Utilities

*   **`LatkJson`**: A wrapper class built on top of `JsonCpp` (and loosely based on `ofxJSON`). It provides easy utility methods to open, parse, and save JSON strings and files from local or remote URLs. `Latk.cpp` uses this to parse the `"grease_pencil"` nested JSON structure into the C++ hierarchy.
*   **`LatkZip`**: A wrapper class utilizing `Poco::Zip` (loosely based on `ofxZipArchive`). It provides utility functions to compress folders or decompress zipped archives. While the `Latk::write` method outputs standard JSON, `LatkZip` is provided to handle `.latk` files which are sometimes zipped JSON archives.

## Rendering Pipeline

During a typical frame update in an openFrameworks application:
1.  `Latk::run()` is called. It checks the elapsed time against the target framerate (`checkInterval()`).
2.  If it is time to advance, it increments the frame on all `LatkLayer`s (`nextFrame()`).
3.  Each `LatkLayer` tells its strokes to run.
4.  The application typically accesses the current points of the `LatkStroke`s in the active frame of each layer to draw them to the screen (either as paths or meshes based on the `drawMesh` flag).
