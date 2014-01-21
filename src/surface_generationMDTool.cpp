#include "surface_generationMDTool.h"

#include "mapHandler.h"

namespace CGoGN
{

class Surface_Render_Plugin;

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

void Surface_GenerationMDTool_Plugin::initializeCages(const QString& view, const QString& map)
{

    MapHandler<PFP2>* mh_selected = static_cast<MapHandler<PFP2>*>(m_schnapps->getMap(map));
    PFP2::MAP* selectedMap = mh_selected->getMap();

    VertexAttribute<PFP2::VEC3> position = selectedMap->getAttribute<PFP2::VEC3, VERTEX>("position");
    VertexAttribute <PFP2::VEC4> color = selectedMap->getAttribute<PFP2::VEC4, VERTEX>("color");
    if(!color.isValid())
    {
        color = selectedMap->addAttribute<PFP2::VEC4, VERTEX>("color");
        mh_selected->registerAttribute(color);
    }

    VertexAttribute<VCage> vCagesObject = selectedMap->getAttribute<VCage, VERTEX>("VCages");
    if(!vCagesObject.isValid())
    {
        vCagesObject = selectedMap->addAttribute<VCage, VERTEX>("VCages");
    }

    createCages(selectedMap);

    MapHandlerGen* mhg_map = m_schnapps->getMap("Cages");
    MapHandler<PFP2>* mh_map = static_cast<MapHandler<PFP2>*>(mhg_map);
    PFP2::MAP* cages = mh_map->getMap();

    VertexAttribute <VCage> vCagesCage = cages->getAttribute<VCage, VERTEX>("VCages");
    if(!vCagesCage.isValid())
    {
        exit -1;
    }

    TraversorV<PFP2::MAP> trav(*selectedMap);
    TraversorV<PFP2::MAP> currentTrav(*cages);
    int id;
    for(Dart d = trav.begin(); d != trav.end(); d = trav.next())
    {
        if(isInCage(position[d], m_cages[0]))
        {
            color[d] = Geom::Vec4f(1.,0.2,0.2,1.);
            vCagesObject[d].addNewCage();
            id=-1;
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

    mh_selected->updateBB(position);
    mh_selected->notifyAttributeModification(position);
    mh_selected->notifyConnectivityModification();
    m_positionVBO->updateData(position);
    m_colorVBO->updateData(color);
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
        exit -1;
    }

    Geom::BoundingBox<PFP2::VEC3> bb = Algo::Geometry::computeBoundingBox<PFP2>(*object, positionObject);
    PFP2::VEC3 min = bb.min();
    PFP2::VEC3 max = bb.max();

    Algo::Surface::Modelisation::swapVectorMax(min, max);

    VertexAttribute<PFP2::VEC3> position = map->getAttribute<PFP2::VEC3, VERTEX>("position") ;
    if(!position.isValid())
    {
        position = map->addAttribute<PFP2::VEC3, VERTEX>("position");
        mh_map->registerAttribute(position);
    }

    VertexAttribute <VCage> vCages = map->getAttribute<VCage, VERTEX>("VCages");
    if(!vCages.isValid())
    {
        vCages = map->addAttribute<VCage, VERTEX>("VCages");
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

    //Identification des cages :
    //0 en bas à gauche, 1 en bas à droite, 2 en haut à gauche, 3 en haut à droite

    m_cages.push_back(std::vector<Dart>());
    position[d0] = (min+max)/2;
    vCages[d0].addId(0);
    vCages[d0].addId(1);
    vCages[d0].addId(2);
    vCages[d0].addId(3);
    m_cages.back().push_back(d0);

    d0 = map->phi1(d0);
    position[d0] = PFP2::VEC3(min[0] + (min[0]+max[0])/5, (min[1]+max[1])/2, min[2]);
    vCages[d0].addId(0);
    vCages[d0].addId(2);
    m_cages.back().push_back(d0);

    d0 = map->phi1(d0);
    position[d0] = PFP2::VEC3(min[0], max[1] + (min[1]+max[1])/5, min[2]);
    vCages[d0].addId(2);
    m_cages.back().push_back(d0);

    d0 = map->phi1(d0);
    position[d0] = PFP2::VEC3((min[0]+max[0])/2, max[1] + (min[1]+max[1])/5, min[2]);
    vCages[d0].addId(2);
    vCages[d0].addId(3);
    m_cages.back().push_back(d0);


    m_cages.push_back(std::vector<Dart>());
    d2 = map->phi1(d2);
    position[d2] = PFP2::VEC3((min[0]+max[10])/2, min[1] - (min[1]+max[1])/5, min[2]);
    vCages[d2].addId(0);
    vCages[d2].addId(1);
    m_cages.back().push_back(d2);

    d2 = map->phi1(d2);
    position[d2] = PFP2::VEC3(min[0] + (min[0]+max[0])/5, min[1] - (min[1]+max[1])/5, min[2]);
    vCages[d2].addId(0);
    m_cages.back().push_back(d2);


    m_cages.push_back(std::vector<Dart>());
    d3 = map->phi1(d3);
    position[d3] = PFP2::VEC3(max[0] - (min[0]+max[0])/5, (min[1]+max[1])/2, min[2]);
    vCages[d3].addId(1);
    vCages[d3].addId(3);
    m_cages.back().push_back(d3);

    d3 = map->phi1(d3);
    position[d3] = PFP2::VEC3(max[0] - (min[0]+max[0])/5, min[1] - (min[1]+max[1])/5, min[2]);
    vCages[d3].addId(1);
    m_cages.back().push_back(d3);

    m_cages.push_back(std::vector<Dart>());
    d1 = map->phi<11>(d1);
    position[d1] = PFP2::VEC3(max[0] - (min[0]+max[0])/5, max[1] + (min[1]+max[1])/5, min[2]);
    vCages[d1].addId(3);
    m_cages.back().push_back(d1);

    mh_map->updateBB(position);
    mh_map->notifyAttributeModification(position);
    mh_map->notifyConnectivityModification();
}

//Check whether a point is in the given cage
bool Surface_GenerationMDTool_Plugin::isInCage(PFP2::VEC3 point, std::vector<Dart> cage)
{
    return true;
}

#ifndef DEBUG
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_Plugin, Surface_GenerationMDTool_Plugin)
#else
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_PluginD, Surface_GenerationMDTool_Plugin)
#endif

} // namespace SCHNApps

} // namespace CGoGN
