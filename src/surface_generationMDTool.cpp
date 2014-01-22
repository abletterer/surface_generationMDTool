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

    VertexAttribute<VCage> vCageObject = selectedMap->getAttribute<VCage, VERTEX>("VCage");
    if(!vCageObject.isValid())
    {
        vCageObject = selectedMap->addAttribute<VCage, VERTEX>("VCage");
    }

    createCages(selectedMap);

    MapHandlerGen* mhg_map = m_schnapps->getMap("Cages");
    MapHandler<PFP2>* mh_map = static_cast<MapHandler<PFP2>*>(mhg_map);
    PFP2::MAP* cage = mh_map->getMap();

    VertexAttribute<PFP2::VEC3> positionCage = cage->getAttribute<PFP2::VEC3, VERTEX>("position");
    VertexAttribute<VCage> vCageCage = cage->getAttribute<VCage, VERTEX>("VCage");
    if(!vCageCage.isValid())
    {
        exit(-1);
    }

    TraversorV<PFP2::MAP> trav(*selectedMap);
    for(Dart d = trav.begin(); d != trav.end(); d = trav.next())
    {
        position[d][2] = positionCage[m_cages[0][0]][2];
        if(isInCage(position[d], m_cages[0], cage))
        {
            color[d] = Geom::Vec4f(1.,0.2,0.2,1.);
            vCageObject[d].addVertex(m_cages[0][0]);
            vCageObject[d].addVertex(cage->phi1(m_cages[0][0]));
            vCageObject[d].addVertex(cage->phi1(m_cages[0][1]));
            vCageObject[d].addVertex(m_cages[0][1]);
        }
        if(isInCage(position[d], m_cages[1], cage))
        {
            color[d] = Geom::Vec4f(0.2,1.,0.2,1.);
            vCageObject[d].addVertex(m_cages[1][0]);
            vCageObject[d].addVertex(cage->phi1(m_cages[1][0]));
            vCageObject[d].addVertex(cage->phi1(m_cages[1][1]));
            vCageObject[d].addVertex(m_cages[1][1]);
        }
        if(isInCage(position[d], m_cages[2], cage))
        {
            color[d] = Geom::Vec4f(0.2,0.2,1.,1.);
            vCageObject[d].addVertex(m_cages[2][0]);
            vCageObject[d].addVertex(cage->phi1(m_cages[2][0]));
            vCageObject[d].addVertex(cage->phi1(m_cages[2][1]));
            vCageObject[d].addVertex(m_cages[2][1]);
        }
        if(isInCage(position[d], m_cages[3], cage))
        {
            color[d] = Geom::Vec4f(0.2,1.,1.,1.);
            vCageObject[d].addVertex(m_cages[3][0]);
            vCageObject[d].addVertex(cage->phi1(m_cages[3][0]));
            vCageObject[d].addVertex(cage->phi1(m_cages[3][1]));
            vCageObject[d].addVertex(m_cages[3][1]);
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
        exit(-1);
    }

    VertexAttribute<PFP2::VEC3> position = map->getAttribute<PFP2::VEC3, VERTEX>("position") ;
    if(!position.isValid())
    {
        position = map->addAttribute<PFP2::VEC3, VERTEX>("position");
        mh_map->registerAttribute(position);
    }

    VertexAttribute <VCage> vCage = map->getAttribute<VCage, VERTEX>("VCage");
    if(!vCage.isValid())
    {
        vCage = map->addAttribute<VCage, VERTEX>("VCage");
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

    Geom::BoundingBox<PFP2::VEC3> bb = Algo::Geometry::computeBoundingBox<PFP2>(*object, positionObject);
    PFP2::VEC3 min = bb.min();
    PFP2::VEC3 max = bb.max();

    Algo::Surface::Modelisation::swapVectorMax(min, max);

    m_cages.push_back(std::deque<Dart>());
    position[d0] = PFP2::VEC3((max[0]+min[0])/2, (max[1]+min[1])/2, min[2]);

    d0 = map->phi1(d0);
    position[d0] = PFP2::VEC3(min[0] - (max[0]-min[0])/5, (max[1]+min[1])/2, min[2]);
    m_cages.back().push_back(d0);   //min

    d0 = map->phi1(d0);
    position[d0] = PFP2::VEC3(min[0] - (max[0]-min[0])/5, max[1] + (max[1]-min[1])/5, min[2]);

    d0 = map->phi1(d0);
    position[d0] = PFP2::VEC3((max[0]+min[0])/2, max[1] + (max[1]-min[1])/5, min[2]);
    m_cages.back().push_back(d0);  //max


    m_cages.push_back(std::deque<Dart>());
    m_cages.back().push_back(d2);   //max
    d2 = map->phi1(d2);
    position[d2] = PFP2::VEC3((max[0]+min[0])/2, min[1] - (max[1]-min[1])/5, min[2]);

    d2 = map->phi1(d2);
    position[d2] = PFP2::VEC3(min[0] - (max[0]-min[0])/5, min[1] - (max[1]-min[1])/5, min[2]);
    m_cages.back().push_front(d2);   //min


    m_cages.push_back(std::deque<Dart>());
    d3 = map->phi1(d3);
    position[d3] = PFP2::VEC3(max[0] + (max[0]-min[0])/5, (max[1]+min[1])/2, min[2]);
    m_cages.back().push_back(d3);   //max

    d3 = map->phi1(d3);
    position[d3] = PFP2::VEC3(max[0] + (max[0]-min[0])/5, min[1] - (max[1]-min[1])/5, min[2]);
    d3 = map->phi1(d3);
    m_cages.back().push_front(d3);   //min

    m_cages.push_back(std::deque<Dart>());
    m_cages.back().push_back(d1);   //min
    d1 = map->phi<11>(d1);
    position[d1] = PFP2::VEC3(max[0] + (max[0]-min[0])/5, max[1] + (max[1]-min[1])/5, min[2]);
    m_cages.back().push_back(d1);   //max

    map->splitFace(d0,map->phi<11>(d0));
    map->splitFace(d1,map->phi<11>(d1));
    map->splitFace(d2,map->phi<11>(d2));
    map->splitFace(d3,map->phi<11>(d3));

    mh_map->updateBB(position);
    mh_map->notifyAttributeModification(position);
    mh_map->notifyConnectivityModification();
}

//Check whether a point is in the given cage
//Returns the id of the cage
bool Surface_GenerationMDTool_Plugin::isInCage(PFP2::VEC3 point, std::deque<Dart> cage, PFP2::MAP* map)
{
    bool res = false;

    VertexAttribute<PFP2::VEC3> position = map->getAttribute<PFP2::VEC3, VERTEX>("position");

    if(position[cage[0]][0] <= point[0] && position[cage[0]][1] <= point[1]
            && position[cage[1]][0] >= point[0] && position[cage[1]][1] >= point[1])
    {
        return true;
    }

    return res;
}

#ifndef DEBUG
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_Plugin, Surface_GenerationMDTool_Plugin)
#else
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_PluginD, Surface_GenerationMDTool_Plugin)
#endif

} // namespace SCHNApps

} // namespace CGoGN
