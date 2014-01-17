#include "surface_generationMDTool.h"

#include "mapHandler.h"

namespace CGoGN
{

class Surface_Render_Plugin;

namespace SCHNApps
{

bool Surface_GenerationMDTool_Plugin::enable()
{
    m_selectedMap = m_schnapps->getSelectedMap();

    m_dockTab = new Surface_GenerationMDTool_DockTab(m_schnapps, this);
    m_schnapps->addPluginDockTab(this, m_dockTab, "Surface_GenerationMultiDimensionalTool");

    m_colorPerVertexShader = new CGoGN::Utils::ShaderColorPerVertex();
    registerShader(m_colorPerVertexShader);

    m_positionVBO = new Utils::VBO();
    m_colorVBO = new Utils::VBO();

    connect(m_schnapps, SIGNAL(selectedMapChanged(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selectedMapChanged(MapHandlerGen*, MapHandlerGen*)));

    m_toDraw = false;

    return true;
}

void Surface_GenerationMDTool_Plugin::disable()
{
    delete m_colorPerVertexShader;
    delete m_positionVBO;
    delete m_colorVBO;

    disconnect(m_schnapps, SIGNAL(selectedMapChanged(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selectedMapChanged(MapHandlerGen*, MapHandlerGen*)));
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

void Surface_GenerationMDTool_Plugin::selectedMapChanged(MapHandlerGen *prev, MapHandlerGen *cur)
{
    m_selectedMap = cur;
}

void Surface_GenerationMDTool_Plugin::initializeCages(const QString& view, const QString& map)
{
    MapHandler<PFP2>* mh_selected = static_cast<MapHandler<PFP2>*>(m_schnapps->getMap(map));
    PFP2::MAP* selectedMap = mh_selected->getMap();

    VertexAttribute<PFP2::VEC3> position = selectedMap->getAttribute<PFP2::VEC3, VERTEX>("position");
    VertexAttribute <PFP2::VEC4> color = selectedMap->getAttribute<PFP2::VEC4, VERTEX>("color") ;
    if(!color.isValid())
    {
        color = selectedMap->addAttribute<PFP2::VEC4, VERTEX>("color");
        mh_selected->registerAttribute(color);
    }

    Geom::BoundingBox<PFP2::VEC3> bb = Algo::Geometry::computeBoundingBox<PFP2>(*selectedMap, position);
    m_cages.reserve(4);

    m_cages.push_back(std::vector<PFP2::VEC3>(2));
    m_cages.back()[0] = bb.min();
    m_cages.back()[0][2] = bb.max()[2];
    m_cages.back()[1][0] = (bb.min()[0]+bb.max()[0])/2;
    m_cages.back()[1][1] = (bb.min()[1]+bb.max()[1])/2;
    m_cages.back()[1][2] = bb.max()[2];

    m_cages.push_back(std::vector<PFP2::VEC3>(2));
    m_cages.back()[0] = (bb.min()[0]+bb.max()[0])/2;
    m_cages.back()[0][1] = bb.min()[1];
    m_cages.back()[0][2] = bb.max()[2];
    m_cages.back()[1][0] = bb.max()[0];
    m_cages.back()[1][1] = (bb.min()[1]+bb.max()[1])/2;
    m_cages.back()[1][2] = bb.max()[2];

    m_cages.push_back(std::vector<PFP2::VEC3>(2));
    m_cages.back()[0] = bb.min()[0];
    m_cages.back()[0][1] = (bb.min()[1]+bb.max()[1])/2;
    m_cages.back()[0][2] = bb.max()[2];
    m_cages.back()[1][0] = (bb.min()[0]+bb.max()[0])/2;
    m_cages.back()[1][1] = bb.max()[1];
    m_cages.back()[1][2] = bb.max()[2];

    m_cages.push_back(std::vector<PFP2::VEC3>(2));
    m_cages.back()[0] = (bb.min()[0]+bb.max()[0])/2;
    m_cages.back()[0][1] = (bb.min()[1]+bb.max()[1])/2;
    m_cages.back()[0][2] = bb.max()[2];
    m_cages.back()[1] = bb.max();

    TraversorV<PFP2::MAP> trav(*selectedMap);
    for(Dart d = trav.begin(); d != trav.end(); d = trav.next())
    {
        if(isInCage(position[d], m_cages[0]))
        {
            color[d] = Geom::Vec4f(1.,0.2,0.2,1.);
        }
        else if(isInCage(position[d], m_cages[1]))
        {
            color[d] = Geom::Vec4f(0.2,1.,0.2,1.);
        }
        else if(isInCage(position[d], m_cages[2]))
        {
            color[d] = Geom::Vec4f(0.2,0.2,1.,1.);
        }
        else if(isInCage(position[d], m_cages[3]))
        {
            color[d] = Geom::Vec4f(0.2,1.,1.,1.);
        }
    }

    m_positionVBO->updateData(position);
    m_colorVBO->updateData(color);
    m_toDraw = true;

    createCages();

    if(view==m_schnapps->getSelectedView()->getName())
    {
        m_schnapps->getSelectedView()->updateGL();
    }
}

void Surface_GenerationMDTool_Plugin::createCages()
{
    MapHandlerGen* mhg_map = m_schnapps->addMap("Cages", 2);
    MapHandler<PFP2>* mh_map = static_cast<MapHandler<PFP2>*>(mhg_map);
    PFP2::MAP* map = mh_map->getMap();

    VertexAttribute<PFP2::VEC3> position = map->getAttribute<PFP2::VEC3, VERTEX>("position") ;
    if(!position.isValid())
    {
        position = map->addAttribute<PFP2::VEC3, VERTEX>("position");
        mh_map->registerAttribute(position);
    }

    Dart d0 = map->newFace(4);
    Dart d1 = map->newFace(4);
    Dart d2 = map->newFace(4);
    Dart d3 = map->newFace(4);

    map->sewFaces(d0,d1);
    d0 = map->phi1(d0);
    map->sewFaces(d0,d2);
    d2 = map->phi1(d2);
    map->sewFaces(d2,d3);
    d3 = map->phi1(d3);
    map->sewFaces(d3,map->phi_1(d1));

    position[d0] = m_cages[0][1];
    d0 = map->phi1(d0);
    position[d0] = m_cages[2][0];
    d0 = map->phi1(d0);
    position[d0] = PFP2::VEC3(m_cages[2][0][0], m_cages[2][1][1], m_cages[2][1][2]);
    d0 = map->phi1(d0);
    position[d0] = m_cages[2][1];
    d0 = map->phi1(d0);

    d2 = map->phi1(d2);
    position[d2] = m_cages[1][0];
    d2 = map->phi1(d2);
    position[d2] = m_cages[0][0];

    d3 = map->phi1(d3);
    position[d3] = m_cages[1][1];
    d3 = map->phi1(d3);
    position[d3] = PFP2::VEC3(m_cages[1][1][0], m_cages[1][0][1], m_cages[1][1][2]);

    d1 = map->phi<11>(d1);
    position[d1] = m_cages[3][1];

    mh_map->updateBB(position);
    mh_map->notifyAttributeModification(position);
    mh_map->notifyConnectivityModification();
}

//Check whether a point is in the given cage
bool Surface_GenerationMDTool_Plugin::isInCage(PFP2::VEC3 point, std::vector<PFP2::VEC3> cage)
{
    return cage[0][0]<=point[0] && cage[0][1]<=point[1]
       &&  cage[1][0]>=point[0] && cage[1][1]>=point[1];
}

#ifndef DEBUG
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_Plugin, Surface_GenerationMDTool_Plugin)
#else
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_PluginD, Surface_GenerationMDTool_Plugin)
#endif

} // namespace SCHNApps

} // namespace CGoGN
