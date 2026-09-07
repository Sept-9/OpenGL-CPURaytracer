///////////////////////////////////////////////////
//
//  Hamish Carr
//  September, 2020
//
//  ------------------------
//  TexturedObject.h
//  ------------------------
//  
//  Base code for rendering assignments.
//
//  Minimalist (non-optimised) code for reading and 
//  rendering an object file
//  
//  We will make some hard assumptions about input file
//  quality. We will not check for manifoldness or 
//  normal direction, &c.  And if it doesn't work on 
//  all object files, that's fine.
//
//  While I could set it up to use QImage for textures,
//  I want this code to be reusable without Qt, so I 
//  shall make a hard assumption that textures are in 
//  ASCII PPM and use my own code to read them
//  
///////////////////////////////////////////////////

// include guard for TexturedObject
#ifndef TEXTURED_OBJECT_H
#define TEXTURED_OBJECT_H

// include the C++ standard libraries we need for the header
#include <vector>
#include <iostream>

// include the unit with Cartesian 3-vectors
#include "Cartesian3.h"

// the image class for a texture
#include "RGBAImage.h" 
// material class
#include "Material.h"

//trying not to break includes
class RenderParameters;
#include "RenderParameters.h"

class ThreeDModel
    { // class
    public:
    // vector of vertices
    std::vector<Cartesian3> vertices;

    // vector of normals
    std::vector<Cartesian3> normals;
    
    // vector of texture coordinates (stored as triple to simplify code)
    std::vector<Cartesian3> textureCoords;

    // vector of faces
    std::vector<std::vector<unsigned int> > faceVertices;

    // corresponding vector of normals
    std::vector<std::vector<unsigned int> > faceNormals;
    
    // corresponding vector of texture coordinates
    std::vector<std::vector<unsigned int> > faceTexCoords;

    //Material that it might have
    Material *material;

    // constructor will initialise to safe values
    ThreeDModel();
    
    // read routine returns true on success, failure otherwise
    static std::vector<ThreeDModel> ReadObjectStream(std::istream &geometryStream);

    // read routine returns true on success, failure otherwise
    static std::vector<ThreeDModel> ReadObjectStreamMaterial(std::istream &geometryStream, std::istream &materialStream);

    // write routine
    void WriteObjectStream(std::ostream &geometryStream);



    }; // class TexturedObject

// end of include guard for TexturedObject
#endif
