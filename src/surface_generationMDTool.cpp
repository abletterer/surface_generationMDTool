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

void Surface_GenerationMDTool_Plugin::initializeObject(const QString& view, int x, int y)
{
    MapHandlerGen* mhg_map = m_schnapps->addMap("Model", 2);
    MapHandler<PFP2>* mh_map = static_cast<MapHandler<PFP2>*>(mhg_map);
    PFP2::MAP* map = mh_map->getMap();

    VertexAttribute<PFP2::VEC3> position = map->getAttribute<PFP2::VEC3, VERTEX>("position");
    if(!position.isValid())
    {
        position = map->addAttribute<PFP2::VEC3, VERTEX>("position");
    }

    Algo::Surface::Tilings::Square::Grid<PFP2> grid(*map, x, y);
    grid.embedIntoGrid(position, x, y);

    mh_map->updateBB(position);
    mh_map->notifyAttributeModification(position);
    mh_map->notifyConnectivityModification();

    map->enableQuickTraversal<VERTEX>();

    if(view==m_schnapps->getSelectedView()->getName())
    {
        m_schnapps->getSelectedView()->updateGL();
    }

}

void Surface_GenerationMDTool_Plugin::initializeCages(const QString& view, int x, int y, const QString& model)
{
    MapHandlerGen* mhg_selected = m_schnapps->getMap(model);

    if(!mhg_selected)
    {
        mhg_selected = m_schnapps->getMap("Model");
    }

    if(mhg_selected)
    {
        MapHandler<PFP2>* mh_selected = static_cast<MapHandler<PFP2>*>(mhg_selected);
        PFP2::MAP* selectedMap = mh_selected->getMap();

        createCages(selectedMap, x, y);

        if(view==m_schnapps->getSelectedView()->getName())
        {
            m_schnapps->getSelectedView()->updateGL();
        }
    }
}

void Surface_GenerationMDTool_Plugin::createCages(PFP2::MAP* object, int x, int y)
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

    //Identification des cages :
    //0 en bas à gauche, 1 en bas à droite, 2 en haut à gauche, 3 en haut à droite

    Geom::BoundingBox<PFP2::VEC3> bb = Algo::Geometry::computeBoundingBox<PFP2>(*object, positionObject);
    PFP2::VEC3 min = bb.min();
    PFP2::VEC3 max = bb.max();

    Algo::Surface::Modelisation::swapVectorMax(min, max);

    min += (max-min)/2.f;
    max -= (max-min)/2.f;

    Algo::Surface::Tilings::Square::Grid<PFP2> grid(*map, x, y);
    grid.embedIntoGrid(position, x, y);

    PFP2::MATRIX44 mat;
    mat.identity();

    mat.setSubVectorV<3>(0, 3, PFP2::VEC3(0.,0., 0.));

    mat(0, 0) = (max[0]-min[0])/x;
    mat(1, 1) = (max[1]-min[1])/y;

    grid.transform(position, mat);

    map->enableQuickTraversal<FACE>();
    map->enableQuickTraversal<VERTEX>();

    map->initAllOrbitsEmbedding<FACE>();

    mh_map->updateBB(position);
    mh_map->notifyAttributeModification(position);
    mh_map->notifyConnectivityModification();
}

#ifndef DEBUG
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_Plugin, Surface_GenerationMDTool_Plugin)
#else
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_PluginD, Surface_GenerationMDTool_Plugin)
#endif

} // namespace SCHNApps

} // namespace CGoGN
