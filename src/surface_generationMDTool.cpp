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

void Surface_GenerationMDTool_Plugin::initializeObject(const QString& view, const QString& filename)
{
    MapHandlerGen* mhg_map = m_schnapps->addMap("Model", 2);
    MapHandler<PFP2>* mh_map = static_cast<MapHandler<PFP2>*>(mhg_map);
    PFP2::MAP* map = mh_map->getMap();

    VertexAttribute<PFP2::VEC3> position = map->getAttribute<PFP2::VEC3, VERTEX>("position");
    if(!position.isValid())
    {
        position = map->addAttribute<PFP2::VEC3, VERTEX>("position");
    }

    VertexAttribute <PFP2::VEC3> colorMap = map->getAttribute<PFP2::VEC3, VERTEX>("color");
    if(!colorMap.isValid())
    {
        colorMap = map->addAttribute<PFP2::VEC3, VERTEX>("color");
    }

    QString extension = filename.mid(filename.lastIndexOf('.'));
    extension.toUpper();

    QImage image;
    if(!image.load(filename, extension.toUtf8().constData()))
    {
        CGoGNout << "Image has not been loaded correctly" << CGoGNendl;
        return;
    }

    int x = image.width(), y = image.height();

    Algo::Surface::Tilings::Square::Grid<PFP2> grid(*map, x-1, y-1);
    grid.embedIntoGrid(position, x-1, y-1);

    std::vector<Dart> vDarts = grid.getVertexDarts();

    QRgb pixel;

    for(int i = 0; i < x; ++i)
    {
        for(int j = 0; j < y; ++j)
        {
            pixel = image.pixel(i,(y-j)-1);
            colorMap[vDarts[j*x+i]] = PFP2::VEC3(qRed(pixel)/255.f, qGreen(pixel)/255.f, qBlue(pixel)/255.f);
        }
    }

    mh_map->updateBB(position);
    mh_map->notifyAttributeModification(position);
    mh_map->notifyAttributeModification(colorMap);
    mh_map->notifyConnectivityModification();

    map->enableQuickTraversal<VERTEX>();

    if(view==m_schnapps->getSelectedView()->getName())
    {
        m_schnapps->getSelectedView()->updateGL();
    }

}

void Surface_GenerationMDTool_Plugin::initializeCages(const QString& view, const int nbCagesPerRow, const int nbCagesPerColumn, const float scale)
{
    MapHandlerGen* mhg_selected = m_schnapps->getMap("Model");

    if(mhg_selected)
    {
        MapHandler<PFP2>* mh_selected = static_cast<MapHandler<PFP2>*>(mhg_selected);
        PFP2::MAP* selectedMap = mh_selected->getMap();

        createCages(selectedMap, nbCagesPerRow, nbCagesPerColumn, scale);

        if(view==m_schnapps->getSelectedView()->getName())
        {
            m_schnapps->getSelectedView()->updateGL();
        }
    }
}

void Surface_GenerationMDTool_Plugin::createCages(PFP2::MAP* object, int nbCagesPerRow, int nbCagesPerColumn, PFP2::REAL scale)
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

    VertexAttribute<PFP2::VEC3> linkCagesVCages = vcages->getAttribute<PFP2::VEC3, VERTEX>("LinkCages");
    if(!linkCagesVCages.isValid())
    {
        linkCagesVCages = vcages->addAttribute<PFP2::VEC3, VERTEX>("LinkCages");
    }

    FaceAttribute<int> idCageCages = cages->getAttribute<int, FACE>("IdCage") ;
    if(!idCageCages.isValid())
    {
        idCageCages = cages->addAttribute<int, FACE>("IdCage");
    }

    FaceAttribute<int> idCageVCages = vcages->getAttribute<int, FACE>("IdCage") ;
    if(!idCageVCages.isValid())
    {
        idCageVCages = vcages->addAttribute<int, FACE>("IdCage");
    }

    Geom::BoundingBox<PFP2::VEC3> bb = Algo::Geometry::computeBoundingBox<PFP2>(*object, positionObject);
    PFP2::VEC3 min = bb.min();
    PFP2::VEC3 max = bb.max();

    Algo::Surface::Modelisation::swapVectorMax(min, max);

    PFP2::REAL width = max[0] - min[0];
    PFP2::REAL height = max[1] - min[1];

    PFP2::REAL stepW = width/nbCagesPerRow, stepH = height/nbCagesPerColumn;
    PFP2::REAL w = min[0]+ stepW/2.f, h = min[1]+ stepH/2.f;

    Dart d, current, previous, next;
    std::vector<PFP2::VEC3> newPosition, linkVector;
    newPosition.resize(4);
    linkVector.resize(4);
    PFP2::VEC3 previousEdgeNormal, nextEdgeNormal;

    const PFP2::REAL sqrt2DivBy2 = std::sqrt(2)/2.f;

    if(scale-1.f <= FLT_EPSILON)
    {
        scale += 0.05f;
    }

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

            current = d;
            int k = 0;

            PFP2::REAL moyNorm;

            //Calcul des normales aux sommets de la petite cage
            //On prend comme base les normales des arêtes incidentes au sommet considéré, ici le vecteur perpendiculaire à chacune des 2 arêtes
            //On normalise les vecteurs et on leur attribue une norme commune correspond à la moyenne de leur anciennes normes
            do
            {
                previous = cages->phi_1(current);
                next = cages->phi1(current);

                previousEdgeNormal = PFP2::VEC3(-(positionCages[previous][1]-positionCages[current][1]), positionCages[previous][0]-positionCages[current][0], 0.f);
                moyNorm = previousEdgeNormal.normalize();
                nextEdgeNormal = PFP2::VEC3(-(positionCages[current][1]-positionCages[next][1]), positionCages[current][0]-positionCages[next][0], 0.f);
                moyNorm += nextEdgeNormal.normalize();
                moyNorm /= 2.f;

                previousEdgeNormal *= moyNorm;
                nextEdgeNormal *= moyNorm;

                linkVector[k] = ((previousEdgeNormal+nextEdgeNormal)/2.f)*(scale-1.f);
                newPosition[k] = positionCages[current]+linkVector[k];

                current = cages->phi1(current);
                ++k;
            } while(current != d);

            d = vcages->newFace(cages->faceDegree(d));
            idCageVCages[d] = i*nbCagesPerRow+j;

            for(k = 0; k < newPosition.size(); ++k)
            {
                positionVCages[d] = newPosition[k];
                linkCagesVCages[d] = linkVector[k];
                d = vcages->phi1(d);
            }

//            d = vcages->newFace(4);
//            idCageVCages[d] = i*nbCagesPerRow+j;

//            positionVCages[d] = PFP2::VEC3(w-(sqrt2DivBy2*stepW)*scale/2.f, h-(sqrt2DivBy2*stepH)*scale/2.f, 0.f);
//            d = cages->phi1(d);
//            positionVCages[d] = PFP2::VEC3(w+(sqrt2DivBy2*stepW)*scale/2.f, h-(sqrt2DivBy2*stepH)*scale/2.f, 0.f);
//            d = cages->phi1(d);
//            positionVCages[d] = PFP2::VEC3(w+(sqrt2DivBy2*stepW)*scale/2.f, h+(sqrt2DivBy2*stepH)*scale/2.f, 0.f);
//            d = cages->phi1(d);
//            positionVCages[d] = PFP2::VEC3(w-(sqrt2DivBy2*stepW)*scale/2.f, h+(sqrt2DivBy2*stepH)*scale/2.f, 0.f);
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
