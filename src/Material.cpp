//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5892M Advanced Rendering
//  User Interface for Coursework
////////////////////////////////////////////////////////////////////////

#include "Material.h"
#include <string>
Material::Material(Cartesian3 ambient,Cartesian3 diffuse,Cartesian3 specular,Cartesian3 emissive,float shininess,std::istream &textureStream)
{
    this->ambient = ambient;
    this->diffuse = diffuse;
    this->specular = specular;
    this->emissive = emissive;
    this->shininess = shininess;
    this->reflectivity=0;
    this->indexOfRefraction=1;
    this->transparency=0;
    texture = new RGBAImage();
    texture->ReadPPM(textureStream);
    name = "default";
    setFromFile = false;
}

Material::Material(Cartesian3 ambient,Cartesian3 diffuse,Cartesian3 specular,Cartesian3 emissive,float shininess)
{
    this->ambient = ambient;
    this->diffuse = diffuse;
    this->specular = specular;
    this->emissive = emissive;
    this->shininess = shininess;
    this->reflectivity=0;
    this->indexOfRefraction=1;
    this->transparency=0;
    texture = nullptr;
    name = "default";
    setFromFile = false;
}

Material::Material()
{
    this->ambient =Cartesian3();
    this->diffuse = Cartesian3();
    this->specular = Cartesian3();
    this->emissive = Cartesian3();
    this->shininess = 0.0f;
    this->reflectivity=0;
    this->indexOfRefraction=1;
    this->transparency=0;
    texture = nullptr;
    name = "default";
    setFromFile = false;
}

Material::~Material()
{
    delete texture;
}

std::vector<Material*> Material::readMaterials(std::istream &materialStream)
{

    std::vector<Material*> r;
    Material *m = new Material();

    // First we read the material file
   // std::vector<Material> materials = Material::ReadMtl(materialStream);
    std::string name = "";
    // the rest of this is a loop reading lines & adding them in appropriate places
    while (true)
    {
        // check for eof() in case we've run out
        if (materialStream.eof())
            break;

        std::string token;
        materialStream >> token;
        if(token == "newmtl")
        {
            if(name != "")
            {
                m->setFromFile = true;
                r.push_back(m);
                m = new Material();
            }
            materialStream >> name;
            m->name = name;
        }
        else if(token == "Ka")
        {
            Cartesian3 ambient;
            materialStream >> ambient;
            m->ambient = ambient;
        }
        else if(token == "Kd")
        {
            Cartesian3 diffuse;
            materialStream >> diffuse;
            m->diffuse = diffuse;
        }
        else if(token == "Ks")
        {
            Cartesian3 specular;
            materialStream >> specular;
            m->specular = specular;
        }
        else if(token == "Ke")
        {
            Cartesian3 emissive;
            materialStream >> emissive;
            m->emissive = emissive;
        }
        else if(token == "Ns")
        {
            float shininess;
            materialStream >> shininess;
            m->shininess = shininess;
        }
        else if(token == "N_ior")
        {
            float ior;
            materialStream >> ior;
            m->indexOfRefraction = ior;
        }
        else if(token == "N_mirr")
        {
            float reflect;
            materialStream >> reflect;
            m->reflectivity = reflect;
        }
        else if(token == "N_transp")
        {
            float transp;
            materialStream >> transp;
            m->transparency = transp;
        }
        else if(token == "map_Ka")
        {
            std::string filename = "";
            materialStream >> filename;
            std::ifstream textureFile(filename.c_str());
            if(!textureFile.good()){
                std::cout << "Problem reading texture " << filename << " for the material " << m->name << std::endl;
            }else{
                m->texture = new RGBAImage();
                m->texture->ReadPPM(textureFile);
            }
        }
    } // not eof
    m->setFromFile = true;
    r.push_back(m);
    return r;
}

bool Material::isLight(){
    std::size_t found = name.find("light");
    return found !=std::string::npos;
}
