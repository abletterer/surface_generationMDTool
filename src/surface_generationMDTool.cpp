#include "surface_generationMDTool.h"

#include "mapHandler.h"

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
}

void Surface_GenerationMDTool_Plugin::initializeCages(const QString& view)
{
    MapHandlerGen* mhg_selected = m_schnapps->addMap("Model", 2);
    MapHandler<PFP2>* mh_selected = static_cast<MapHandler<PFP2>*>(mhg_selected);
    PFP2::MAP* selectedMap = mh_selected->getMap();
    Dart d0 = selectedMap->newFace(4);

    VertexAttribute<PFP2::VEC3> position = selectedMap->addAttribute<PFP2::VEC3, VERTEX>("position");

    position[d0] = PFP2::VEC3(0.f,0.f,0.f);
    d0 = selectedMap->phi1(d0);
    position[d0] = PFP2::VEC3(1.f,0.f,0.f);
    d0 = selectedMap->phi1(d0);
    position[d0] = PFP2::VEC3(1.f,1.f,0.f);
    d0 = selectedMap->phi1(d0);
    position[d0] = PFP2::VEC3(0.f,1.f,0.f);

    VertexAttribute <PFP2::VEC4> color = selectedMap->getAttribute<PFP2::VEC4, VERTEX>("color");
    if(!color.isValid())
    {
        color = selectedMap->addAttribute<PFP2::VEC4, VERTEX>("color");
        mh_selected->registerAttribute(color);
    }

    mh_selected->updateBB(position);
    mh_selected->notifyAttributeModification(position);
    mh_selected->notifyConnectivityModification();

    createCages(selectedMap);

    for(int i=0; i<10; ++i)
    {
        Algo::Surface::Modelisation::quadranguleFaces<PFP2, VertexAttribute<PFP2::VEC3> >(*selectedMap, position);
    }

    mh_selected->updateBB(position);
    mh_selected->notifyAttributeModification(position);
    mh_selected->notifyConnectivityModification();

    m_toDraw = true;

    if(view==m_schnapps->getSelectedView()->getName())
    {
        m_schnapps->getSelectedView()->updateGL();
    }
}

void Surface_GenerationMDTool_Plugin::createCages(PFP2::MAP* object)
{
    MapHandlerGen* mhg_map = m_schnapps->addMap("Cages", 2);
    MapHandler<PFP2>* mh_map = static_cast<MapHandler<PFP2>*>(mhg_map);
    PFP2::MAP* map = mh_map->getMap();

    VertexAttribute<PFP2::VEC3> positionObject = object->getAttribute<PFP2::VEC3, VERTEX>("position");
    if(!positionObject.isValid())
    {
        CGoGNout << "Object position attribute not valid" << CGoGNendl;
        exit(-1);
    }

    VertexAttribute<PFP2::VEC3> position = map->getAttribute<PFP2::VEC3, VERTEX>("position") ;
    if(!position.isValid())
    {
        position = map->addAttribute<PFP2::VEC3, VERTEX>("position");
        mh_map->registerAttribute(position);
    }

    Dart d1 = map->newFace(4);
    Dart d0 = map->newFace(4);
    Dart d2 = map->newFace(4);
    Dart d3 = map->newFace(4);

    map->sewFaces(d0,d1);
    d0 = map->phi1(d0);
    map->sewFaces(d0,d2);
    d2 = map->phi1(d2);
    map->sewFaces(d2,d3);
    d3 = map->phi1(d3);
    map->sewFaces(d3,map->phi_1(d1));

    //Identification des cages :
    //0 en bas à gauche, 1 en bas à droite, 2 en haut à gauche, 3 en haut à droite

    Geom::BoundingBox<PFP2::VEC3> bb = Algo::Geometry::computeBoundingBox<PFP2>(*object, positionObject);
    PFP2::VEC3 min = bb.min();
    PFP2::VEC3 max = bb.max();

    Algo::Surface::Modelisation::swapVectorMax(min, max);

    m_cages.push_back(std::deque<Dart>());
    m_cages.back().push_back(d1);   //min
    position[d1] = PFP2::VEC3((max[0]+min[0])/2, (max[1]+min[1])/2, min[2]);
    d1 = map->phi<11>(d1);
    position[d1] = PFP2::VEC3(max[0] + (max[0]-min[0])/20, max[1] + (max[1]-min[1])/20, min[2]);
    m_cages.back().push_back(d1);   //max


    m_cages.push_back(std::deque<Dart>());
    d0 = map->phi1(d0);
    position[d0] = PFP2::VEC3((max[0]+min[0])/2, min[1] - (max[1]-min[1])/20, min[2]);
    m_cages.back().push_back(d0);   //min

    d0 = map->phi1(d0);
    position[d0] = PFP2::VEC3(max[0] + (max[0]-min[0])/20, min[1] - (max[1]-min[1])/20, min[2]);

    d0 = map->phi1(d0);
    position[d0] = PFP2::VEC3(max[0] + (max[0]-min[0])/20, (max[1]+min[1])/2, min[2]);
    m_cages.back().push_back(d0);  //max


    m_cages.push_back(std::deque<Dart>());
    d3 = map->phi1(d3);
    position[d3] = PFP2::VEC3((max[0]+min[0])/2, max[1] + (max[1]-min[1])/20, min[2]);
    m_cages.back().push_back(d3);   //max

    d3 = map->phi1(d3);
    position[d3] = PFP2::VEC3(min[0] - (max[0]-min[0])/20, max[1] + (max[1]-min[1])/20, min[2]);
    d3 = map->phi1(d3);
    m_cages.back().push_front(d3);   //min


    m_cages.push_back(std::deque<Dart>());
    m_cages.back().push_back(d2);   //max
    d2 = map->phi1(d2);
    position[d2] = PFP2::VEC3(min[0] - (max[0]-min[0])/20, (max[1]+min[1])/2, min[2]);

    d2 = map->phi1(d2);
    position[d2] = PFP2::VEC3(min[0] - (max[0]-min[0])/20, min[1] - (max[1]-min[1])/20, min[2]);
    m_cages.back().push_front(d2);   //min

    map->initAllOrbitsEmbedding<FACE>();

    mh_map->updateBB(position);
    mh_map->notifyAttributeModification(position);
    mh_map->notifyConnectivityModification();
}

//Check whether a point is in the given cage
//Returns the id of the cage
bool Surface_GenerationMDTool_Plugin::isInCage(PFP2::VEC3 point, std::deque<Dart> cage, PFP2::MAP* map)
{

    VertexAttribute<PFP2::VEC3> position = map->getAttribute<PFP2::VEC3, VERTEX>("position");

    if(position[cage[0]][0]-FLT_EPSILON < point[0] && position[cage[0]][1]-FLT_EPSILON < point[1]
            && position[cage[1]][0]+FLT_EPSILON > point[0] && position[cage[1]][1]+FLT_EPSILON > point[1])
    {
        return true;
    }

    return false;
}

#ifndef DEBUG
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_Plugin, Surface_GenerationMDTool_Plugin)
#else
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_PluginD, Surface_GenerationMDTool_Plugin)
#endif

} // namespace SCHNApps

} // namespace CGoGN
