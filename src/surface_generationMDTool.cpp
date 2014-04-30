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

void Surface_GenerationMDTool_Plugin::initializeCages(const QString& view, int nbCagesPerRow, int nbCagesPerColumn,  const QString& model)
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

        createCages(selectedMap, nbCagesPerRow, nbCagesPerColumn);

        if(view==m_schnapps->getSelectedView()->getName())
        {
            m_schnapps->getSelectedView()->updateGL();
        }
    }
}

void Surface_GenerationMDTool_Plugin::createCages(PFP2::MAP* object, int nbCagesPerRow, int nbCagesPerColumn)
{
    MapHandlerGen* mhg_map = m_schnapps->addMap("Cages", 2);
    MapHandler<PFP2>* mh_map = static_cast<MapHandler<PFP2>*>(mhg_map);
    PFP2::MAP* cages = mh_map->getMap();

    MapHandlerGen* mhg_vcages = m_schnapps->addMap("VCages", 2);
    MapHandler<PFP2>* mh_vcages = static_cast<MapHandler<PFP2>*>(mhg_vcages);
    PFP2::MAP* vcages = mh_vcages->getMap();

    VertexAttribute<PFP2::VEC3> positionObject = object->getAttribute<PFP2::VEC3, VERTEX>("position");
    if(!positionObject.isValid())
    {
        CGoGNout << "Object position attribute not valid" << CGoGNendl;
        exit(-1);
    }

    VertexAttribute<PFP2::VEC3> positionCages = cages->getAttribute<PFP2::VEC3, VERTEX>("position") ;
    if(!positionCages.isValid())
    {
        positionCages = cages->addAttribute<PFP2::VEC3, VERTEX>("position");
        mh_map->registerAttribute(positionCages);
    }

    VertexAttribute<PFP2::VEC3> positionVCages = vcages->getAttribute<PFP2::VEC3, VERTEX>("position") ;
    if(!positionVCages.isValid())
    {
        positionVCages = vcages->addAttribute<PFP2::VEC3, VERTEX>("position");
        mh_vcages->registerAttribute(positionVCages);
    }

    FaceAttribute<int> idCageCages = cages->getAttribute<int, FACE>("IdCage") ;
    if(!idCageCages.isValid())
    {
        idCageCages = cages->addAttribute<int, FACE>("IdCage");
        mh_map->registerAttribute(idCageCages);
    }

    FaceAttribute<int> idCageVCages = vcages->getAttribute<int, FACE>("IdCage") ;
    if(!idCageVCages.isValid())
    {
        idCageVCages = vcages->addAttribute<int, FACE>("IdCage");
        mh_map->registerAttribute(idCageVCages);
    }

//    Geom::BoundingBox<PFP2::VEC3> bb = Algo::Geometry::computeBoundingBox<PFP2>(*object, positionObject);
//    PFP2::VEC3 min = bb.min();
//    PFP2::VEC3 max = bb.max();

//    Algo::Surface::Modelisation::swapVectorMax(min, max);

//    min += (max-min)/2.f + 0.1f;
//    max -= (max-min)/2.f + 0.1f;

//    Algo::Surface::Tilings::Square::Grid<PFP2> grid(*cages, x, y);
//    grid.embedIntoGrid(positionMap, x, y);

//    PFP2::MATRIX44 mat;
//    mat.identity();

//    mat.setSubVectorV<3>(0, 3, PFP2::VEC3(0.,0., 0.));

//    mat(0, 0) = (max[0]-min[0])/x;
//    mat(1, 1) = (max[1]-min[1])/y;

//    grid.transform(positionMap, mat);

//    min -= (max-min)/2.f + 0.1f;
//    max += (max-min)/2.f + 0.1f;

//    Algo::Surface::Tilings::Square::Grid<PFP2> grid2(*vcages, x, y);
//    grid2.embedIntoGrid(positionVCages, x, y);

//    mat.identity();

//    mat.setSubVectorV<3>(0, 3, PFP2::VEC3(0.,0., 0.));

//    mat(0, 0) = (max[0]-min[0])/x;
//    mat(1, 1) = (max[1]-min[1])/y;

//    grid2.transform(positionVCages, mat);

    Geom::BoundingBox<PFP2::VEC3> bb = Algo::Geometry::computeBoundingBox<PFP2>(*object, positionObject);
    PFP2::VEC3 min = bb.min();
    PFP2::VEC3 max = bb.max();

    Algo::Surface::Modelisation::swapVectorMax(min, max);

    PFP2::REAL width = max[0] - min[0];
    PFP2::REAL height = max[1] - min[1];

    PFP2::REAL stepW = width/nbCagesPerRow, stepH = height/nbCagesPerColumn;
    PFP2::REAL w = min[0]+ stepW/2.f, h = min[1]+ stepH/2.f;

    Dart d;

    const PFP2::REAL sqrt2DivBy2 = std::sqrt(2)/2.f;

    for(int i = 0; i < nbCagesPerColumn; ++i)
    {
        w = min[0] + stepW/2.f;
        for(int j = 0; j < nbCagesPerRow; ++j)
        {
            d = cages->newFace(4);
            idCageCages[d] = i*nbCagesPerRow+j;

            positionCages[d] = PFP2::VEC3(w-(sqrt2DivBy2*stepW/2.f), h-(sqrt2DivBy2*stepH/2.f), 0.f);
            d = cages->phi1(d);
            positionCages[d] = PFP2::VEC3(w+(sqrt2DivBy2*stepW/2.f), h-(sqrt2DivBy2*stepH/2.f), 0.f);
            d = cages->phi1(d);
            positionCages[d] = PFP2::VEC3(w+(sqrt2DivBy2*stepW/2.f), h+(sqrt2DivBy2*stepH/2.f), 0.f);
            d = cages->phi1(d);
            positionCages[d] = PFP2::VEC3(w-(sqrt2DivBy2*stepW/2.f), h+(sqrt2DivBy2*stepH/2.f), 0.f);

            d = vcages->newFace(4);
            idCageVCages[d] = i*nbCagesPerRow+j;

            positionVCages[d] = PFP2::VEC3(w-(sqrt2DivBy2*stepW), h-(sqrt2DivBy2*stepH), 0.f);
            d = cages->phi1(d);
            positionVCages[d] = PFP2::VEC3(w+(sqrt2DivBy2*stepW), h-(sqrt2DivBy2*stepH), 0.f);
            d = cages->phi1(d);
            positionVCages[d] = PFP2::VEC3(w+(sqrt2DivBy2*stepW), h+(sqrt2DivBy2*stepH), 0.f);
            d = cages->phi1(d);
            positionVCages[d] = PFP2::VEC3(w-(sqrt2DivBy2*stepW), h+(sqrt2DivBy2*stepH), 0.f);
            w += stepW;
        }
        h += stepH;
    }

    cages->enableQuickTraversal<FACE>();

    vcages->enableQuickTraversal<FACE>();

    mh_map->updateBB(positionCages);
    mh_map->notifyAttributeModification(positionCages);
    mh_map->notifyAttributeModification(idCageCages);
    mh_map->notifyConnectivityModification();

    mh_vcages->updateBB(positionVCages);
    mh_vcages->notifyAttributeModification(positionVCages);
    mh_map->notifyAttributeModification(idCageVCages);
    mh_vcages->notifyConnectivityModification();
}

#ifndef DEBUG
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_Plugin, Surface_GenerationMDTool_Plugin)
#else
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_PluginD, Surface_GenerationMDTool_Plugin)
#endif

} // namespace SCHNApps

} // namespace CGoGN
