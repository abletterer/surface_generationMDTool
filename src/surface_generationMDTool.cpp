#include "surface_generationMDTool.h"

#include "mapHandler.h"

#include "coordinates.h"

namespace CGoGN
{

namespace SCHNApps
{

bool Surface_GenerationMDTool_Plugin::enable()
{
    m_dockTab = new Surface_GenerationMDTool_DockTab(m_schnapps, this);
    m_schnapps->addPluginDockTab(this, m_dockTab, "Surface_GenerationMultiDimensionalTool");

    m_colorPerVertexShader = new CGoGN::Utils::ShaderColorPerVertex();
    registerShader(m_colorPerVertexShader);

    m_positionVBO = new Utils::VBO();
    m_colorVBO = new Utils::VBO();

    connect(m_dockTab->button_initializeCages, SIGNAL(clicked()), this, SLOT(initializeCages()));

    m_toDraw = false;

    return true;
}

void Surface_GenerationMDTool_Plugin::disable()
{
    delete m_colorPerVertexShader;
    delete m_positionVBO;
    delete m_colorVBO;
}

void Surface_GenerationMDTool_Plugin::drawMap(View *view, MapHandlerGen *map)
{
    if(m_toDraw)
    {
        //If VBO are initialized
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
        glEnable(GL_POLYGON_OFFSET_FILL);
        m_colorPerVertexShader->setAttributePosition(m_positionVBO);
        m_colorPerVertexShader->setAttributeColor(m_colorVBO);
        m_colorPerVertexShader->setOpacity(1.);
        map->draw(m_colorPerVertexShader, CGoGN::Algo::Render::GL2::TRIANGLES);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void Surface_GenerationMDTool_Plugin::initializeCages()
{
    MapHandlerGen* mhg_selected = m_schnapps->getSelectedMap();
    m_cages.clear();
    if(mhg_selected)
    {
        MapHandler<PFP2>* mh_selected = static_cast<MapHandler<PFP2>*>(mhg_selected);
        PFP2::MAP* selectedMap = mh_selected->getMap();

        VertexAttribute<PFP2::VEC3> position = selectedMap->getAttribute<PFP2::VEC3, VERTEX>("position");
        VertexAttribute <PFP2::VEC4> color = selectedMap->getAttribute<PFP2::VEC4, VERTEX>("color") ;
        if(!color.isValid())
            color = selectedMap->addAttribute<PFP2::VEC4, VERTEX>("color") ;

        Geom::BoundingBox<PFP2::VEC3> bb = Algo::Geometry::computeBoundingBox<PFP2>(*selectedMap, position);
        m_cages.reserve(4);

        m_cages.push_back(std::vector<PFP2::VEC3>(2));
        m_cages.back()[0] = bb.min();
        m_cages.back()[1][0] = (bb.min()[0]+bb.max()[0])/2;
        m_cages.back()[1][1] = (bb.min()[1]+bb.max()[1])/2;
        m_cages.back()[1][2] = bb.max()[2];

        m_cages.push_back(std::vector<PFP2::VEC3>(2));
        m_cages.back()[0] = (bb.min()[0]+bb.max()[0])/2;
        m_cages.back()[0][1] = bb.min()[1];
        m_cages.back()[0][2] = bb.min()[2];
        m_cages.back()[1][0] = bb.max()[0];
        m_cages.back()[1][1] = (bb.min()[1]+bb.max()[1])/2;
        m_cages.back()[1][2] = bb.max()[2];

        m_cages.push_back(std::vector<PFP2::VEC3>(2));
        m_cages.back()[0] = bb.min()[0];
        m_cages.back()[0][1] = (bb.min()[1]+bb.max()[1])/2;
        m_cages.back()[0][2] = bb.min()[2];
        m_cages.back()[1][0] = (bb.min()[0]+bb.max()[0])/2;
        m_cages.back()[1][1] = bb.max()[1];
        m_cages.back()[1][2] = bb.max()[2];

        m_cages.push_back(std::vector<PFP2::VEC3>(2));
        m_cages.back()[0] = (bb.min()[0]+bb.max()[0])/2;
        m_cages.back()[0][1] = (bb.min()[1]+bb.max()[1])/2;
        m_cages.back()[0][2] = bb.min()[2];
        m_cages.back()[1] = bb.max();

        TraversorV<PFP2::MAP> trav(*selectedMap);
        for(Dart d = trav.begin(); d != trav.end(); d = trav.next())
        {
            if(isInCage(position[d], m_cages[0]))
            {
                color[d] = Geom::Vec4f(1.,0.2,0.2,1.);
                computePointMVC(m_cages[0], d);
            }
            else if(isInCage(position[d], m_cages[1]))
            {
                color[d] = Geom::Vec4f(0.2,1.,0.2,1.);
                computePointMVC(m_cages[1], d);
            }
            else if(isInCage(position[d], m_cages[2]))
            {
                color[d] = Geom::Vec4f(0.2,0.2,1.,1.);
                computePointMVC(m_cages[2], d);
            }
            else if(isInCage(position[d], m_cages[3]))
            {
                color[d] = Geom::Vec4f(0.2,1.,1.,1.);
                computePointMVC(m_cages[3], d);
            }
        }

        m_positionVBO->updateData(position);
        m_colorVBO->updateData(color);
        m_toDraw = true;
        m_schnapps->getSelectedView()->updateGL();
    }
}

//Check whether a point is in the given cage
bool Surface_GenerationMDTool_Plugin::isInCage(PFP2::VEC3 point, std::vector<PFP2::VEC3> cage)
{
    return cage[0][0]<=point[0] && cage[0][1]<=point[1] && cage[0][2]<=point[2]
       &&  cage[1][0]>=point[0] && cage[1][1]>=point[1] && cage[1][2]>=point[2];
}

/*
  * Fonction qui calcule les coordonnées MVC d'un point par rapport à une cage
  */
void Surface_GenerationMDTool_Plugin::computePointMVC(const std::vector<PFP2::VEC3>& cage, Dart point)
{
    if(cage.size()==2)
    {
        PFP2::REAL c;
        PFP2::REAL sumMVC(0);

        VertexAttribute<PFP2::VEC3> position = selectedMap->getAttribute<PFP2::VEC3, VERTEX>("position");
        if(!position.isValid())
        {
            CGoGNout << "Position attribute chosen isn't valid" << CGoGNendl;
            return;
        }

        VertexAttribute<Coordinates> coordinates = selectedMap->getAttribute<Coordinates, VERTEX>("coordinates");
        if(!coordinates.isValid())
        {
            coordinates = selectedMap->addAttribute<Coordinates, VERTEX>("coordinates");
        }

        int compt = 0;
        float x = cage[0][0], y = cage[0][1], z = cage[0][2];
        std::vector<PFP2::REAL> coord;

        for(unsigned int i=0; i<8; ++i)
        {
            switch(i)
            {
            case 1 :
                z = cage[1][2];
                break;
            case 2 :
                x = cage[1][0];
                break;
            case 3 :
                z = cage[0][2];
                break;
            case 4 :
                x = cage[0][0];
                y = cage[1][1];
                break;
            case 5 :
                x = cage[1][0];
                break;
            case 6 :
                z = cage[1][2];
                break;
            case 7 :
                x = cage[0][0];
                break;
            default:
                break;
            }

            c = computeMVC(PFP2::VEC3(x,y,z), point, cage, position);
            coord.push_back(c);
            coordinates[point].setCoordinate(cage[0], cage[1], c, i);
            sumMVC += c;
            ++compt;
        }

        for(int i = coordinates.begin(); i != coordinates.end(); coordinates.next(i))
        {
            coordinates[i].setCoordinate(cage[0], cage[1], coordinates[i].getCoordinate(cage[0], cage[1])/sumMVC);
        }
    }
}

PFP2::REAL Surface_GenerationMDTool_Plugin::computeMVC(const PFP2::VEC3& pt, Dart vertex, const PFP2::VEC3& prec, const PFP2::VEC3& suiv, const VertexAttribute<PFP2::VEC3>& position)
{
    PFP2::REAL r = (position[vertex]-pt).norm();

    PFP2::REAL sumU(0.);
    Dart it = vertex;
    do
    {
        PFP2::VEC3 vi = position[it];
        PFP2::VEC3 vj = position[prec];
        PFP2::VEC3 vk = position[suiv];

        PFP2::REAL Bjk = Geom::angle((vj-pt),(vk-pt));
        PFP2::REAL Bij = Geom::angle((vi-pt),(vj-pt));
        PFP2::REAL Bki = Geom::angle((vk-pt),(vi-pt));

        PFP2::VEC3 ei = (vi-pt)/((vi-pt).norm());
        PFP2::VEC3 ej = (vj-pt)/((vj-pt).norm());
        PFP2::VEC3 ek = (vk-pt)/((vk-pt).norm());

        PFP2::VEC3 eiej = ei^ej;
        PFP2::VEC3 ejek = ej^ek;
        PFP2::VEC3 ekei = ek^ei;

        PFP2::VEC3 nij = eiej/(eiej.norm());
        PFP2::VEC3 njk = ejek/(ejek.norm());
        PFP2::VEC3 nki = ekei/(ekei.norm());

        PFP2::REAL ui= (Bjk + (Bij*(nij*njk)) + (Bki*(nki*njk)))/(2.0f*ei*njk);

        sumU+=ui;

        it = cage->phi<21>(it);
    }
    while(it!=vertex);

    return (1.0f/r)*sumU;
}


#ifndef DEBUG
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_Plugin, Surface_GenerationMDTool_Plugin)
#else
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_PluginD, Surface_GenerationMDTool_Plugin)
#endif

} // namespace SCHNApps

} // namespace CGoGN
