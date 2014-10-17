#ifndef COORDINATES_H
#define COORDINATES_H

#include "Geometry/vector_gen.h"

#include "types.h"

#include <map>

#include <sstream>

namespace CGoGN
{
namespace SCHNApps
{

/*
* Classe définissant l'attribut de sommet Coordinates, définissant les coordonnées associées à un sommet
*/
class Coordinates
{
public:
    Coordinates()
        :   m_coordinates()
    {}

    void setCoordinate(const PFP2::VEC3& min, const PFP2::VEC3& max, const PFP2::REAL& coordinate, int index)
    {
        std::ostringstream key;
        key << min << max

        m_coordinates[QString::fromStdString(key.str())][index] = coordinate;
    }

    PFP2::REAL getCoordinate(const PFP2::VEC3& min, const PFP2::VEC3& max, int index)
    {
        std::ostringstream key;
        key << min << max;

        std::map<QString, std::vector<PFP2::REAL> >::const_iterator elem;
        if((elem = m_coordinates.find(QString::fromStdString(key.str()))) != m_coordinates.end())
        {
            return elem->second[index];
        }
        return NULL;
    }

    void setCoordinate(const QString& key, const PFP2::REAL& coordinate, int index)
    {
        m_coordinates[key][index] = coordinate;
    }

    PFP2::REAL getCoordinate(const QString& key, int index)
    {
        std::map<QString, std::vector<PFP2::REAL> >::const_iterator elem;
        if((elem = m_coordinates.find(key)) != m_coordinates.end())
        {
            return elem->second[index];
        }
        return NULL;
    }


    static std::string CGoGNnameOfType()
    {
        return "Coordinates" ;
    }

private:
    std::map<QString, std::vector<PFP2::REAL> > m_coordinates;
};

} //namespace SCHNApps
} //namespace CGoGN


#endif // COORDINATES_H
