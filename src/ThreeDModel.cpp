///////////////////////////////////////////////////
//
//  Hamish Carr
//  September, 2020
//
//  ------------------------
//  TexturedObject.cpp
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

// include the header file
#include "ThreeDModel.h"

// include the C++ standard libraries we want
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

// include the Cartesian 3- vector class
#include "Cartesian3.h"

#define MAXIMUM_LINE_LENGTH 1024

// constructor will initialise to safe values
ThreeDModel::ThreeDModel()
    { // TexturedObject()
    // force arrays to size 0
    vertices.resize(0);
    normals.resize(0);
    textureCoords.resize(0);
    } // TexturedObject()

// read routine returns true on success, failure otherwise
std::vector<ThreeDModel> ThreeDModel::ReadObjectStreamMaterial(std::istream &geometryStream, std::istream &materialStream)
    { // ReadObjectStreamMaterial()
    std::vector<ThreeDModel> r;

    // First we read the material file
    std::vector<Material*> ms = Material::readMaterials(materialStream);

    Material *m = nullptr;

    ThreeDModel t;
    // create a read buffer
    char readBuffer[MAXIMUM_LINE_LENGTH];


    //Vertex data is shared between everyone
    std::vector<Cartesian3> vertices;
    std::vector<Cartesian3> normals;
    std::vector<Cartesian3> textureCoords;

    // the rest of this is a loop reading lines & adding them in appropriate places
    while (true)
        { // not eof
        // character to read
        char firstChar = static_cast<char>(geometryStream.get());

        // check for eof() in case we've run out
        if (geometryStream.eof())
            break;

        // otherwise, switch on the character we read
        switch (firstChar)
            { // switch on first character
            case '#':       // comment line
                // read and discard the line
                geometryStream.getline(readBuffer, MAXIMUM_LINE_LENGTH);
                break;

            case 'v':       // vertex data of some type
                { // some sort of vertex data
                // retrieve another character
                char secondChar = static_cast<char>(geometryStream.get());

                // bail if we ran out of file
                if (geometryStream.eof())
                    break;

                // now use the second character to choose branch
                switch (secondChar)
                    { // switch on second character
                    case ' ':       // space - indicates a vertex
                        { // vertex read
                        Cartesian3 vertex;
                        geometryStream >> vertex;
                        vertices.push_back(vertex);
                        break;
                        } // vertex read
                    case 'n':       // n indicates normal vector
                        { // normal read
                        Cartesian3 normal;
                        geometryStream >> normal;
                        normals.push_back(normal.unit());
                        break;
                        } // normal read
                    case 't':       // t indicates texture coords
                        { // tex coord
                        Cartesian3 texCoord;
                        float u;
                        float v;
                        geometryStream >> u;
                        geometryStream >> v;
                        texCoord.x = u;
                        texCoord.y = v;
                        textureCoords.push_back(texCoord);
                        break;
                        } // tex coord
                    default:
                        break;
                    } // switch on second character
                break;
                } // some sort of vertex data

            case 'f':       // face data
                { // face
                // a face can have an arbitrary number of vertices, which is a pain
                // so we will create a separate buffer to read from
                geometryStream.getline(readBuffer, MAXIMUM_LINE_LENGTH);

                // turn into a C++ string
                std::string lineString = std::string(readBuffer);

                // create a string stream
                std::stringstream lineParse(lineString);

                // create vectors for the IDs (with different names from the master arrays)
                std::vector<unsigned int> faceVertexSet;
                std::vector<unsigned int> faceNormalSet;
                std::vector<unsigned int> faceTexCoordSet;

                // now loop through the line
                while (!lineParse.eof())
                    { // lineParse isn't done
                    // the triple of vertex, normal, tex coord IDs
                    unsigned int vertexID;
                    unsigned int normalID;
                    unsigned int texCoordID;

                    // try reading them in, breaking if we hit eof
                    lineParse >> vertexID;
                    // retrieve & discard a slash
                    lineParse.get();
                    // check eof
                    if (lineParse.eof())
                        break;

                    // and the tex coord
                    lineParse >> texCoordID;
                    lineParse.get();
                    if (lineParse.eof())
                        break;

                    // read normal likewise
                    lineParse >> normalID;

                    // if we got this far, we presumably have three valid numbers, so add them
                    // but notice that .obj uses 1-based numbering, where our arrays use 0-based
                    faceVertexSet.push_back(vertexID-1);
                    faceNormalSet.push_back(normalID-1);
                    faceTexCoordSet.push_back(texCoordID-1);
                    } // lineParse isn't done

                // as long as the face has at least three vertices, add to the master list
                if (faceVertexSet.size() > 2)
                    { // at least 3
                    t.faceVertices.push_back(faceVertexSet);
                    t.faceNormals.push_back(faceNormalSet);
                    t.faceTexCoords.push_back(faceTexCoordSet);
                    } // at least 3

                break;
                } // face

            case 'u':{ //usemtl
                std::string token;
                //we discard until we find space
                geometryStream >> token;
                //then get the name of the material;
                geometryStream >> token;
                for (unsigned int i = 0 ;i < ms.size();i++)
                {
                    if(ms.at(i)->name == token){
                        if(m == nullptr){
                            m = ms.at(i);
                            t.material = m;
                            break;
                        }else{
                            t.vertices.assign(vertices.begin(),vertices.end());
                            t.normals.assign(normals.begin(),normals.end());
                            t.textureCoords.assign(textureCoords.begin(),textureCoords.end());
                            r.push_back(t);
                            t = ThreeDModel();
                            m = ms.at(i);
                            t.material = m;
                        }
                    }
                }
                }
            } // switch on first character

        } // not eof

    t.material = m;
    t.vertices.assign(vertices.begin(),vertices.end());
    t.normals.assign(normals.begin(),normals.end());
    t.textureCoords.assign(textureCoords.begin(),textureCoords.end());
    r.push_back(t);
    return r;

    } // ReadObjectStreamMaterial()


// read routine object true on success, NULL otherwise
std::vector<ThreeDModel> ThreeDModel::ReadObjectStream(std::istream &geometryStream)
    { // ReadObjectStream()
    
    ThreeDModel t;
    t.material = nullptr;
    // create a read buffer
    char readBuffer[MAXIMUM_LINE_LENGTH];
    
    // the rest of this is a loop reading lines & adding them in appropriate places
    while (true)
        { // not eof
        // character to read
        char firstChar = char(geometryStream.get());
        
        // check for eof() in case we've run out
        if (geometryStream.eof())
            break;

        // otherwise, switch on the character we read
        switch (firstChar)
            { // switch on first character
            case '#':       // comment line
                // read and discard the line
                geometryStream.getline(readBuffer, MAXIMUM_LINE_LENGTH);
                break;
                
            case 'v':       // vertex data of some type
                { // some sort of vertex data
                // retrieve another character
                char secondChar = char(geometryStream.get());
                
                // bail if we ran out of file
                if (geometryStream.eof())
                    break;

                // now use the second character to choose branch
                switch (secondChar)
                    { // switch on second character
                    case ' ':       // space - indicates a vertex
                        { // vertex read
                        Cartesian3 vertex;
                        geometryStream >> vertex;
                        t.vertices.push_back(vertex);
                        break;
                        } // vertex read
                    case 'n':       // n indicates normal vector
                        { // normal read
                        Cartesian3 normal;
                        geometryStream >> normal;
                        t.normals.push_back(normal);
                        break;
                        } // normal read
                    case 't':       // t indicates texture coords
                        { // tex coord
                        Cartesian3 texCoord;
                        geometryStream >> texCoord;
                        t.textureCoords.push_back(texCoord);
                        break;                  
                        } // tex coord
                    default:
                        break;
                    } // switch on second character 
                break;
                } // some sort of vertex data
                
            case 'f':       // face data
                { // face
                // a face can have an arbitrary number of vertices, which is a pain
                // so we will create a separate buffer to read from
                geometryStream.getline(readBuffer, MAXIMUM_LINE_LENGTH);

                // turn into a C++ string
                std::string lineString = std::string(readBuffer);

                // create a string stream
                std::stringstream lineParse(lineString); 

                // create vectors for the IDs (with different names from the master arrays)
                std::vector<unsigned int> faceVertexSet;
                std::vector<unsigned int> faceNormalSet;
                std::vector<unsigned int> faceTexCoordSet;
                
                // now loop through the line
                while (!lineParse.eof())
                    { // lineParse isn't done
                    // the triple of vertex, normal, tex coord IDs
                    unsigned int vertexID;
                    unsigned int normalID;
                    unsigned int texCoordID;

                    // try reading them in, breaking if we hit eof
                    lineParse >> vertexID;
                    // retrieve & discard a slash
                    lineParse.get();
                    // check eof
                    if (lineParse.eof())
                        break;
                    
                    // and the tex coord
                    lineParse >> texCoordID;
                    lineParse.get();
                    if (lineParse.eof())
                        break;
                        
                    // read normal likewise
                    lineParse >> normalID;
                        
                    // if we got this far, we presumably have three valid numbers, so add them
                    // but notice that .obj uses 1-based numbering, where our arrays use 0-based
                    faceVertexSet.push_back(vertexID-1);
                    faceNormalSet.push_back(normalID-1);
                    faceTexCoordSet.push_back(texCoordID-1);
                    } // lineParse isn't done

                // as long as the face has at least three vertices, add to the master list
                if (faceVertexSet.size() > 2)
                    { // at least 3
                    t.faceVertices.push_back(faceVertexSet);
                    t.faceNormals.push_back(faceNormalSet);
                    t.faceTexCoords.push_back(faceTexCoordSet);
                    } // at least 3
                
                break;
                } // face
            // default processing: do nothing
            default:
                break;

            } // switch on first character

        } // not eof

    // return a success code
    std::vector<ThreeDModel> r;
    r.push_back(t);
    return r;
    } // ReadObjectStream()

// write routine
void ThreeDModel::WriteObjectStream(std::ostream &geometryStream)
    { // WriteObjectStream()
    // output the vertex coordinates
    for (unsigned int vertex = 0; vertex < vertices.size(); vertex++)
        geometryStream << "v  " << std::fixed << vertices[vertex] << std::endl;
    geometryStream << "# " << vertices.size() << " vertices" << std::endl;
    geometryStream << std::endl;

    // and the normal vectors
    for (unsigned int normal = 0; normal < normals.size(); normal++)
        geometryStream << "vn " << std::fixed << normals[normal] << std::endl;
    geometryStream << "# " << normals.size() << " vertex normals" << std::endl;
    geometryStream << std::endl;

    // and the texture coordinates
    for (unsigned int texCoord = 0; texCoord < textureCoords.size(); texCoord++)
        geometryStream << "vt " << std::fixed << textureCoords[texCoord] << std::endl;
    geometryStream << "# " << textureCoords.size() << " texture coords" << std::endl;
    geometryStream << std::endl;

    // and the faces
    for (unsigned int face = 0; face < faceVertices.size(); face++)
        { // per face
        geometryStream << "f ";
        
        // loop through # of vertices
        for (unsigned int vertex = 0; vertex < faceVertices[face].size(); vertex++)
            geometryStream << faceVertices[face][vertex]+1 << "/" << faceTexCoords[face][vertex]+1 << "/" << faceNormals[face][vertex]+1 << " " ;
        
        geometryStream << std::endl;
        } // per face
    geometryStream << "# " << faceVertices.size() << " polygons" << std::endl;
    geometryStream << std::endl;

    } // WriteObjectStream()



