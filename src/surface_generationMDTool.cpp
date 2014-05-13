#include "surface_generationMDTool.h"

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

    m_addFaces = false;
    m_addVertices = false;

    m_verticesCurrentlyAdded.reserve(100);  //Pour avoir de la marge avant la prochaine réallocation mémoire

    m_drawer = new Utils::Drawer();
    registerShader(m_drawer->getShader());

    return true;
}

void Surface_GenerationMDTool_Plugin::disable()
{
    delete m_colorPerVertexShader;
    delete m_positionVBO;
    delete m_colorVBO;
    delete m_drawer;
}

void Surface_GenerationMDTool_Plugin::draw(View *view)
{
    if(m_addFaces)
    {
        m_drawer->newList(GL_COMPILE_AND_EXECUTE);
        m_drawer->pointSize(10.f);
        m_drawer->color3f(1.0f, 0.f, 0.f);
        m_drawer->begin(GL_POINTS);
        for(int i = 0; i < m_verticesCurrentlyAdded.size(); ++i)
        {
            m_drawer->vertex(m_verticesCurrentlyAdded[i]);
        }
        m_drawer->end();
        m_drawer->endList();
    }
}

void Surface_GenerationMDTool_Plugin::keyPress(View *view, QKeyEvent *event)
{
    if(m_schnapps->getSelectedView() == view)
    {
        switch(QApplication::keyboardModifiers())
        {
        case Qt::ShiftModifier :
            switch(event->key())
            {
            case Qt::Key_A:     //Ajout de faces
                if(!m_addFaces)
                {
                    CGoGNout << "--- DEBUT - AJOUT DE FACES ---" << CGoGNendl;
                    CGoGNout << "--- DEBUT - AJOUT DE SOMMETS ---" << CGoGNendl;
                    m_addFaces = true;
                    m_addVertices = true;
                }
                else
                {
                    if(m_addVertices)
                    {
                        CGoGNout << "--- FIN - AJOUT DE SOMMETS ---" << CGoGNendl;
                        CGoGNout << "Création de la nouvelle face .." << CGoGNendl;
                        addNewFace();
                        CGoGNout << ".. fait" << CGoGNendl;
                        m_verticesCurrentlyAdded.clear();
                        m_verticesCurrentlyAdded.reserve(100);  //Pour avoir de la marge avant la prochaine allocation mémoire
                        m_addVertices = false;
                    }
                    m_addFaces = false;
                }
                break;
            case Qt::Key_S :    //Ajout de sommets
                if(m_addFaces)
                {
                    if(!m_addVertices)
                    {
                        CGoGNout << "--- DEBUT - AJOUT DE SOMMETS ---" << CGoGNendl;
                        m_addVertices = true;
                    }
                    else
                    {
                        //On enregistre les sommets ajoutés
                        CGoGNout << "--- FIN - AJOUT DE SOMMETS ---" << CGoGNendl;
                        CGoGNout << "Création de la nouvelle face .." << CGoGNendl;
                        addNewFace();
                        CGoGNout << ".. fait" << CGoGNendl;
                        m_verticesCurrentlyAdded.clear();
                        m_verticesCurrentlyAdded.reserve(100);  //Pour avoir de la marge avant la prochaine allocation mémoire
                        m_addVertices = false;
                    }
                }
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }

    }
}

void Surface_GenerationMDTool_Plugin::mousePress(View* view, QMouseEvent* event)
{
    if(m_addVertices && event->button() == Qt::LeftButton)
    {
        qglviewer::Vec pixelClicked(event->x(), event->y(), 0.5);
        qglviewer::Vec realVertex = view->camera()->unprojectedCoordinatesOf(pixelClicked);
        PFP2::VEC3 realPosition(realVertex[0], realVertex[1], 0.f);
        m_verticesCurrentlyAdded.push_back(realPosition);
        view->updateGL();
    }
}

void Surface_GenerationMDTool_Plugin::initializeObject(const QString& view, const QString& filename, int x, int y)
{
    MapHandlerGen* mhg_map = m_schnapps->addMap("Model", 2);
    MapHandler<PFP2>* mh_map = static_cast<MapHandler<PFP2>*>(mhg_map);
    PFP2::MAP* map = mh_map->getMap();

    VertexAttribute<PFP2::VEC3, PFP2::MAP::IMPL> position = map->getAttribute<PFP2::VEC3, VERTEX>("position");
    if(!position.isValid())
    {
        position = map->addAttribute<PFP2::VEC3, VERTEX>("position");
        mh_map->registerAttribute(position);
    }

    VertexAttribute <PFP2::VEC3, PFP2::MAP::IMPL> colorMap = map->getAttribute<PFP2::VEC3, VERTEX>("color");
    if(!colorMap.isValid())
    {
        colorMap = map->addAttribute<PFP2::VEC3, VERTEX>("color");
    }

    if(!filename.isEmpty())
    {
        QString extension = filename.mid(filename.lastIndexOf('.'));
        extension.toUpper();

        QImage image;
        if(!image.load(filename, extension.toUtf8().constData()))
        {
            CGoGNout << "Image has not been loaded correctly" << CGoGNendl;
            return;
        }

        int imageX = image.width(), imageY = image.height();

        Algo::Surface::Tilings::Square::Grid<PFP2> grid(*map, imageX-1, imageY-1);
        grid.embedIntoGrid(position, imageX-1, imageY-1);

        std::vector<Dart> vDarts = grid.getVertexDarts();

        QRgb pixel;

        for(int i = 0; i < imageX; ++i)
        {
            for(int j = 0; j < imageY; ++j)
            {
                pixel = image.pixel(i,(imageY-j)-1);
                colorMap[vDarts[j*imageX+i]] = PFP2::VEC3(qRed(pixel)/255.f, qGreen(pixel)/255.f, qBlue(pixel)/255.f);
            }
        }
    }
    else
    {
        Algo::Surface::Tilings::Square::Grid<PFP2> grid(*map, x, y);
        grid.embedIntoGrid(position, x, y);
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

        createCages(mh_selected, nbCagesPerRow, nbCagesPerColumn, scale);

        if(view==m_schnapps->getSelectedView()->getName())
        {
            m_schnapps->getSelectedView()->updateGL();
        }
    }
}

void Surface_GenerationMDTool_Plugin::createCages(MapHandler<PFP2>* mh_object, int nbCagesPerRow, int nbCagesPerColumn, PFP2::REAL scale)
{
    PFP2::MAP* object = mh_object->getMap();

    MapHandlerGen* mhg_cages = m_schnapps->getMap("Cages");
    if(!mhg_cages)
    {
        mhg_cages = m_schnapps->addMap("Cages", 2);
    }

    MapHandler<PFP2>* mh_cages = static_cast<MapHandler<PFP2>*>(mhg_cages);
    PFP2::MAP* cages = mh_cages->getMap();
    MapHandlerGen* mhg_vcages = m_schnapps->getMap("VCages");
    if(!mhg_vcages)
    {
        mhg_vcages = m_schnapps->addMap("VCages", 2);
    }
    MapHandler<PFP2>* mh_vcages = static_cast<MapHandler<PFP2>*>(mhg_vcages);
    PFP2::MAP* vcages = mh_vcages->getMap();


    VertexAttribute<PFP2::VEC3, PFP2::MAP::IMPL> positionObject = object->getAttribute<PFP2::VEC3, VERTEX>("position");
    if(!positionObject.isValid())
    {
        CGoGNout << "Object position attribute not valid" << CGoGNendl;
        exit(-1);
    }

    VertexAttribute<PFP2::VEC3, PFP2::MAP::IMPL> positionCages = cages->getAttribute<PFP2::VEC3, VERTEX>("position") ;
    if(!positionCages.isValid())
    {
        positionCages = cages->addAttribute<PFP2::VEC3, VERTEX>("position");
        mh_cages->registerAttribute(positionCages);
    }

    VertexAttribute<PFP2::VEC3, PFP2::MAP::IMPL> positionVCages = vcages->getAttribute<PFP2::VEC3, VERTEX>("position") ;
    if(!positionVCages.isValid())
    {
        positionVCages = vcages->addAttribute<PFP2::VEC3, VERTEX>("position");
        mh_vcages->registerAttribute(positionVCages);
    }

    VertexAttribute<PFP2::VEC3, PFP2::MAP::IMPL> linkCagesVCages = vcages->getAttribute<PFP2::VEC3, VERTEX>("LinkCages");
    if(!linkCagesVCages.isValid())
    {
        linkCagesVCages = vcages->addAttribute<PFP2::VEC3, VERTEX>("LinkCages");
        mh_vcages->registerAttribute(linkCagesVCages);
    }

    FaceAttribute<int, PFP2::MAP::IMPL> idCageCages = cages->getAttribute<int, FACE>("IdCage") ;
    if(!idCageCages.isValid())
    {
        idCageCages = cages->addAttribute<int, FACE>("IdCage");
        mh_cages->registerAttribute(idCageCages);
    }

    FaceAttribute<int, PFP2::MAP::IMPL> idCageVCages = vcages->getAttribute<int, FACE>("IdCage") ;
    if(!idCageVCages.isValid())
    {
        idCageVCages = vcages->addAttribute<int, FACE>("IdCage");
        mh_vcages->registerAttribute(idCageVCages);
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
//        w = min[0] + stepW;
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
            unsigned int k = 0;

            PFP2::REAL moyNorm(0.f);

            //Calcul des normales aux sommets de la petite cage
            //On prend comme base les normales des arêtes incidentes au sommet considéré, ici le vecteur perpendiculaire à chacune des 2 arêtes
            //On normalise les vecteurs et on leur attribue une norme commune correspond à la moyenne de leur anciennes normes

            std::vector<PFP2::VEC3> edgeNormal;
            edgeNormal.reserve(cages->faceDegree(d)*2);

            do
            {
                previous = cages->phi_1(current);
                next = cages->phi1(current);

                previousEdgeNormal = PFP2::VEC3(-(positionCages[previous][1]-positionCages[current][1]), positionCages[previous][0]-positionCages[current][0], 0.f);
                moyNorm += previousEdgeNormal.normalize();
                nextEdgeNormal = PFP2::VEC3(-(positionCages[current][1]-positionCages[next][1]), positionCages[current][0]-positionCages[next][0], 0.f);
                moyNorm += nextEdgeNormal.normalize();

                edgeNormal.push_back(previousEdgeNormal);
                edgeNormal.push_back(nextEdgeNormal);

                current = cages->phi1(current);
            } while(current != d);

            moyNorm /= edgeNormal.size();

            current = d;
            int l = 0;

            do
            {
                previousEdgeNormal = edgeNormal[l]*moyNorm;
                nextEdgeNormal = edgeNormal[l+1]*moyNorm;

                linkVector[k] = ((previousEdgeNormal+nextEdgeNormal)/2.f)*(scale-1.f);
                newPosition[k] = positionCages[current]+linkVector[k];

                current = cages->phi1(current);
                ++k;
                l += 2;
            } while(current != d);

            d = vcages->newFace(cages->faceDegree(d));
            idCageVCages[d] = i*nbCagesPerRow+j;

            for(k = 0; k < newPosition.size(); ++k)
            {
                positionVCages[d] = newPosition[k];
                linkCagesVCages[d] = linkVector[k];
                d = vcages->phi1(d);
            }
//            w += stepW/2.f;
            w += stepW;
        }
//        h += stepH/2.f;
        h += stepH;
    }

    cages->enableQuickTraversal<FACE>();

    vcages->enableQuickTraversal<FACE>();

    mh_cages->updateBB(positionCages);
    mh_cages->notifyAttributeModification(positionCages);
    mh_cages->notifyConnectivityModification();

    mh_vcages->updateBB(positionVCages);
    mh_vcages->notifyAttributeModification(positionVCages);
    mh_vcages->notifyConnectivityModification();
}

void Surface_GenerationMDTool_Plugin::addNewFace()
{
    if(m_addVertices && m_verticesCurrentlyAdded.size() > 2)
    {
        //Si on est en mode ajout de sommets et qu'on a plus de 2 sommets actuellement en création
        MapHandlerGen* mhg_cages = m_schnapps->getMap("Cages");
        if(!mhg_cages)
        {
            mhg_cages = m_schnapps->addMap("Cages", 2);
        }
        MapHandler<PFP2>* mh_cages = static_cast<MapHandler<PFP2>*>(mhg_cages);
        PFP2::MAP* cages = mh_cages->getMap();

        MapHandlerGen* mhg_vcages = m_schnapps->getMap("VCages");
        if(!mhg_vcages)
        {
            mhg_vcages = m_schnapps->addMap("VCages", 2);
        }
        MapHandler<PFP2>* mh_vcages = static_cast<MapHandler<PFP2>*>(mhg_vcages);
        PFP2::MAP* vcages = mh_vcages->getMap();

        VertexAttribute<PFP2::VEC3, PFP2::MAP::IMPL> positionCages = cages->getAttribute<PFP2::VEC3, VERTEX>("position");
        if(!positionCages.isValid())
        {
            positionCages = cages->addAttribute<PFP2::VEC3, VERTEX>("position");
            mh_cages->registerAttribute(positionCages);
        }

        VertexAttribute<PFP2::VEC3, PFP2::MAP::IMPL> positionVCages = vcages->getAttribute<PFP2::VEC3, VERTEX>("position");
        if(!positionVCages.isValid())
        {
            positionVCages = vcages->addAttribute<PFP2::VEC3, VERTEX>("position");
            mh_vcages->registerAttribute(positionVCages);
        }

        VertexAttribute<PFP2::VEC3, PFP2::MAP::IMPL> linkCagesVCages = vcages->getAttribute<PFP2::VEC3, VERTEX>("LinkCages");
        if(!linkCagesVCages.isValid())
        {
            linkCagesVCages = vcages->addAttribute<PFP2::VEC3, VERTEX>("LinkCages");
            mh_vcages->registerAttribute(linkCagesVCages);
        }

        FaceAttribute<int, PFP2::MAP::IMPL> idCageCages = cages->getAttribute<int, FACE>("IdCage") ;
        if(!idCageCages.isValid())
        {
            idCageCages = cages->addAttribute<int, FACE>("IdCage");
            mh_cages->registerAttribute(idCageCages);
        }

        FaceAttribute<int, PFP2::MAP::IMPL> idCageVCages = vcages->getAttribute<int, FACE>("IdCage") ;
        if(!idCageVCages.isValid())
        {
            idCageVCages = vcages->addAttribute<int, FACE>("IdCage");
            mh_vcages->registerAttribute(idCageVCages);
        }

        Dart d = cages->newFace(m_verticesCurrentlyAdded.size());

        for(unsigned int i = 0; i < m_verticesCurrentlyAdded.size(); ++i)
        {
            //Les sommets sont ajoutés dans l'ordre
            positionCages[d] = m_verticesCurrentlyAdded[i];
            d = cages->phi1(d);
        }

        int idCage = d.label();
        idCageCages[d] = idCage;

        Dart current = d, previous, next;
        PFP2::VEC3 previousEdgeNormal, nextEdgeNormal;
        PFP2::REAL moyNorm(0.f), scale(1.5f);
        std::vector<PFP2::VEC3> linkVector, newPosition;
        linkVector.resize(cages->faceDegree(d));
        newPosition.resize(cages->faceDegree(d));

        //Calcul des normales aux sommets de la petite cage
        //On prend comme base les normales des arêtes incidentes au sommet considéré, ici le vecteur perpendiculaire à chacune des 2 arêtes
        //On normalise les vecteurs et on leur attribue une norme commune correspond à la moyenne de leur anciennes normes

        std::vector<PFP2::VEC3> edgeNormal;
        edgeNormal.reserve(cages->faceDegree(d)*2);

        do
        {
            previous = cages->phi_1(current);
            next = cages->phi1(current);

            previousEdgeNormal = PFP2::VEC3(-(positionCages[previous][1]-positionCages[current][1]), positionCages[previous][0]-positionCages[current][0], 0.f);
            moyNorm += previousEdgeNormal.normalize();
            nextEdgeNormal = PFP2::VEC3(-(positionCages[current][1]-positionCages[next][1]), positionCages[current][0]-positionCages[next][0], 0.f);
            moyNorm += nextEdgeNormal.normalize();

            edgeNormal.push_back(previousEdgeNormal);
            edgeNormal.push_back(nextEdgeNormal);

            current = cages->phi1(current);
        } while(current != d);

        moyNorm /= edgeNormal.size();

        current = d;

        int i = 0, j = 0;
        do
        {
            previousEdgeNormal = edgeNormal[j]*moyNorm;
            nextEdgeNormal = edgeNormal[j+1]*moyNorm;

            linkVector[i] = ((previousEdgeNormal+nextEdgeNormal)/2.f)*(scale-1.f);
            newPosition[i] = positionCages[current]+linkVector[i];

            current = cages->phi1(current);
            ++i;
            j += 2;
        } while(current != d);

        d = vcages->newFace(cages->faceDegree(d));
        idCageVCages[d] = idCage;

        for(i = 0; i < newPosition.size(); ++i)
        {
            positionVCages[cages->phi1(d)] = newPosition[i];
            linkCagesVCages[cages->phi1(d)] = linkVector[i];
            d = vcages->phi1(d);
        }

        mh_cages->updateBB(positionCages);
        mh_cages->notifyAttributeModification(positionCages);
        mh_cages->notifyConnectivityModification();

        mh_vcages->updateBB(positionVCages);
        mh_vcages->notifyAttributeModification(positionVCages);
        mh_vcages->notifyConnectivityModification();
    }
}

#ifndef DEBUG
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_Plugin, Surface_GenerationMDTool_Plugin)
#else
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_PluginD, Surface_GenerationMDTool_Plugin)
#endif

} // namespace SCHNApps

} // namespace CGoGN
