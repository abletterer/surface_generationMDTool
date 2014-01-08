#include "surface_generationMultiDimensionalTool.h"

#include "mapHandler.h"

namespace CGoGN
{

namespace SCHNApps
{

MapParameters::MapParameters() :
    m_initialized(false),
    m_voxellisation(),
    m_resolutions(),
    m_bb(),
    m_dilatation(0),
    m_toVoxellise(true),
    m_independant(true),
    m_extractionFaces(true),
    m_toCalculateResolutions(true)
{}

MapParameters::~MapParameters() {}

void MapParameters::start()
{
    if(!m_initialized) {
        m_initialized = true;
    }
}

void MapParameters::stop()
{
    if(m_initialized) {
        m_initialized = false;
    }
}

bool Surface_GenerationMultiDimensionalTool_Plugin::enable()
{
    m_generationMultiDimensionalToolDialog = new Dialog_GenerationMultiDimensionalTool(m_schnapps);

    m_generationMultiDimensionalToolAction = new QAction("Generate multidimensional tool", this);

    m_schnapps->addMenuAction(this, "Surface;Generate multidimensional tool", m_generationMultiDimensionalToolAction);

    connect(m_generationMultiDimensionalToolAction, SIGNAL(triggered()), this, SLOT(openGenerationMultiDimensionalToolDialog()));

    connect(m_generationMultiDimensionalToolDialog->button_generate, SIGNAL(clicked()), this, SLOT(generationMultiDimensionalToolFromDialog()));
    connect(m_generationMultiDimensionalToolDialog->button_dilaterVoxellisation, SIGNAL(clicked()), this, SLOT(dilaterVoxellisationFromDialog()));
    connect(m_generationMultiDimensionalToolDialog->button_reinitialiserVoxellisation, SIGNAL(clicked()), this, SLOT(reinitialiserVoxellisationFromDialog()));

    connect(m_generationMultiDimensionalToolDialog->list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(currentMapSelectedChangedFromDialog()));
    connect(m_generationMultiDimensionalToolDialog->combo_positionAttribute, SIGNAL(currentIndexChanged(QString)), this, SLOT(currentAttributeIndexChangedFromDialog(QString)));
    connect(m_generationMultiDimensionalToolDialog->check_resolution, SIGNAL(toggled(bool)), this, SLOT(resolutionToggledFromDialog(bool)));
    connect(m_generationMultiDimensionalToolDialog->spin_resolution_x, SIGNAL(valueChanged(int)), this, SLOT(resolutionXModifiedFromDialog(int)));
    connect(m_generationMultiDimensionalToolDialog->spin_resolution_y, SIGNAL(valueChanged(int)), this, SLOT(resolutionYModifiedFromDialog(int)));
    connect(m_generationMultiDimensionalToolDialog->spin_resolution_z, SIGNAL(valueChanged(int)), this, SLOT(resolutionZModifiedFromDialog(int)));
    connect(m_generationMultiDimensionalToolDialog->radio_extractionFaces, SIGNAL(toggled(bool)), this, SLOT(surfaceExtractionToggledFromDialog(bool)));

    connect(m_schnapps, SIGNAL(mapAdded(MapHandlerGen*)), this, SLOT(mapAdded(MapHandlerGen*)));
    connect(m_schnapps, SIGNAL(mapRemoved(MapHandlerGen*)), this, SLOT(mapRemoved(MapHandlerGen*)));

    foreach(MapHandlerGen* map, m_schnapps->getMapSet().values())
        mapAdded(map);

    return true;
}

void Surface_GenerationMultiDimensionalTool_Plugin::disable()
{
    disconnect(m_generationMultiDimensionalToolAction, SIGNAL(triggered()), this, SLOT(openGenerationMultiDimensionalToolDialog()));

    disconnect(m_generationMultiDimensionalToolDialog->button_generate, SIGNAL(clicked()), this, SLOT(generationMultiDimensionalToolFromDialog()));
    disconnect(m_generationMultiDimensionalToolDialog->button_dilaterVoxellisation, SIGNAL(clicked()), this, SLOT(dilaterVoxellisationFromDialog()));

    disconnect(m_generationMultiDimensionalToolDialog->list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(currentMapSelectedChangedFromDialog()));
    disconnect(m_generationMultiDimensionalToolDialog->combo_positionAttribute, SIGNAL(currentIndexChanged(QString)), this, SLOT(currentAttributeIndexChangedFromDialog(QString)));
    disconnect(m_generationMultiDimensionalToolDialog->check_resolution, SIGNAL(toggled(bool)), this, SLOT(resolutionToggledFromDialog(bool)));
    disconnect(m_generationMultiDimensionalToolDialog->spin_resolution_x, SIGNAL(editingFinished()), this, SLOT(resolutionModifiedFromDialog()));
    disconnect(m_generationMultiDimensionalToolDialog->spin_resolution_y, SIGNAL(editingFinished()), this, SLOT(resolutionModifiedFromDialog()));
    disconnect(m_generationMultiDimensionalToolDialog->spin_resolution_z, SIGNAL(editingFinished()), this, SLOT(resolutionModifiedFromDialog()));
    disconnect(m_generationMultiDimensionalToolDialog->radio_extractionFaces, SIGNAL(toggled(bool)), this, SLOT(surfaceExtractionToggledFromDialog(bool)));

    disconnect(m_schnapps, SIGNAL(mapAdded(MapHandlerGen*)), this, SLOT(mapAdded(MapHandlerGen*)));
    disconnect(m_schnapps, SIGNAL(mapRemoved(MapHandlerGen*)), this, SLOT(mapRemoved(MapHandlerGen*)));
}

void Surface_GenerationMultiDimensionalTool_Plugin::mapAdded(MapHandlerGen *map)
{
    connect(map, SIGNAL(attributeModified(unsigned int, QString)), this, SLOT(attributeModified(unsigned int, QString)));
}

void Surface_GenerationMultiDimensionalTool_Plugin::mapRemoved(MapHandlerGen *map)
{
    disconnect(map, SIGNAL(attributeModified(unsigned int, QString)), this, SLOT(attributeModified(unsigned int, QString)));
}

void Surface_GenerationMultiDimensionalTool_Plugin::attributeModified(unsigned int orbit, QString nameAttr)
{
    //Rien pour le moment
}

void Surface_GenerationMultiDimensionalTool_Plugin::openGenerationMultiDimensionalToolDialog()
{
    if(m_generationMultiDimensionalToolDialog->list_maps->currentRow()==-1)
        m_generationMultiDimensionalToolDialog->updateAppearanceFromPlugin(true, false);
    m_generationMultiDimensionalToolDialog->show();
}

void Surface_GenerationMultiDimensionalTool_Plugin::generationMultiDimensionalToolFromDialog()
{
    QList<QListWidgetItem*> currentItems = m_generationMultiDimensionalToolDialog->list_maps->selectedItems();
    if(!currentItems.empty())
    {
        const QString& mapName = currentItems[0]->text();

        QString positionName = m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentText();

        generationMultiDimensionalTool(mapName, positionName);
    }
}

void Surface_GenerationMultiDimensionalTool_Plugin::dilaterVoxellisationFromDialog()
{
    QList<QListWidgetItem*> currentItems = m_generationMultiDimensionalToolDialog->list_maps->selectedItems();
    if(!currentItems.empty())
    {
        const QString& mapName = currentItems[0]->text();

        QString positionName = m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentText();

        dilaterVoxellisation(mapName, positionName);
    }
}

void Surface_GenerationMultiDimensionalTool_Plugin::reinitialiserVoxellisationFromDialog() {
    QList<QListWidgetItem*> currentItems = m_generationMultiDimensionalToolDialog->list_maps->selectedItems();
    if(!currentItems.empty())
    {
        const QString& mapName = currentItems[0]->text();

        QString positionName = m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentText();

        reinitialiserVoxellisation(mapName, positionName);
    }
}

void Surface_GenerationMultiDimensionalTool_Plugin::currentMapSelectedChangedFromDialog()
{
    QList<QListWidgetItem*> currentItems = m_generationMultiDimensionalToolDialog->list_maps->selectedItems();
    if(!currentItems.empty() && m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentIndex()!=-1)
    {
        MapParameters& p = h_parameterSet[currentItems[0]->text()+m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentText()];
        if(p.m_initialized) {
            m_generationMultiDimensionalToolDialog->updateAppearanceFromPlugin(p.m_independant, p.m_resolutions[0]!=0);
            m_generationMultiDimensionalToolDialog->updateResolutionsFromPlugin(p.m_resolutions);
            m_generationMultiDimensionalToolDialog->updateNiveauDilatationFromPlugin(p.m_dilatation);
        }
        else
        {
            m_generationMultiDimensionalToolDialog->updateAppearanceFromPlugin(p.m_independant, false);
        }
    }
    else {
        m_generationMultiDimensionalToolDialog->updateAppearanceFromPlugin(true, false);
    }
}

void Surface_GenerationMultiDimensionalTool_Plugin::currentAttributeIndexChangedFromDialog(QString nameAttr)
{
    QList<QListWidgetItem*> currentItems = m_generationMultiDimensionalToolDialog->list_maps->selectedItems();
    if(!currentItems.empty() && m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentIndex()!=-1) {
        MapParameters& p = h_parameterSet[currentItems[0]->text()+nameAttr];
        if(p.m_initialized)
        {
            m_generationMultiDimensionalToolDialog->updateAppearanceFromPlugin(p.m_independant, p.m_resolutions[0]!=0);
            m_generationMultiDimensionalToolDialog->updateResolutionsFromPlugin(p.m_resolutions);
            m_generationMultiDimensionalToolDialog->updateNiveauDilatationFromPlugin(p.m_dilatation);
        }
        else
        {
            m_generationMultiDimensionalToolDialog->updateAppearanceFromPlugin(p.m_independant, false);
        }
    }
}

void Surface_GenerationMultiDimensionalTool_Plugin::resolutionToggledFromDialog(bool b)
{
    QList<QListWidgetItem*> currentItems = m_generationMultiDimensionalToolDialog->list_maps->selectedItems();
    if(!currentItems.empty() && m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentIndex()!=-1) {
        MapParameters& p = h_parameterSet[currentItems[0]->text()+m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentText()];
        if(p.m_initialized)
        {
            p.m_independant = b;
            p.m_toVoxellise = true;
            updateResolutions(currentItems[0]->text(),m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentText());
            m_generationMultiDimensionalToolDialog->updateAppearanceFromPlugin(p.m_independant, p.m_resolutions[0]!=0);
        }
    }
}

void Surface_GenerationMultiDimensionalTool_Plugin::resolutionXModifiedFromDialog(int value)
{
    QList<QListWidgetItem*> currentItems = m_generationMultiDimensionalToolDialog->list_maps->selectedItems();
    if(!currentItems.empty())
    {
        MapParameters& p = h_parameterSet[currentItems[0]->text()+m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentText()];
        if(p.m_initialized)
        {
            p.m_resolutions[0] = value;
            if(!p.m_independant) {
                updateResolutions(currentItems[0]->text(), m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentText());
            }
            p.m_toVoxellise = true;
        }
    }
}

void Surface_GenerationMultiDimensionalTool_Plugin::resolutionYModifiedFromDialog(int value)
{
    QList<QListWidgetItem*> currentItems = m_generationMultiDimensionalToolDialog->list_maps->selectedItems();
    if(!currentItems.empty())
    {
        MapParameters& p = h_parameterSet[currentItems[0]->text()+m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentText()];
        if(p.m_initialized && p.m_independant)
        {
            p.m_resolutions[1] = value;
            p.m_toVoxellise = true;
        }
    }
}

void Surface_GenerationMultiDimensionalTool_Plugin::resolutionZModifiedFromDialog(int value)
{
    QList<QListWidgetItem*> currentItems = m_generationMultiDimensionalToolDialog->list_maps->selectedItems();
    if(!currentItems.empty())
    {
        MapParameters& p = h_parameterSet[currentItems[0]->text()+m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentText()];
        if(p.m_initialized && p.m_independant)
        {
            p.m_resolutions[2] = value;
            p.m_toVoxellise = true;
        }
    }
}

void Surface_GenerationMultiDimensionalTool_Plugin::surfaceExtractionToggledFromDialog(bool b)
{
    QList<QListWidgetItem*> currentItems = m_generationMultiDimensionalToolDialog->list_maps->selectedItems();
    if(!currentItems.empty())
    {
        MapParameters& p = h_parameterSet[currentItems[0]->text()+m_generationMultiDimensionalToolDialog->combo_positionAttribute->currentText()];
        p.m_extractionFaces = b;
    }
}

/*
  * Fonction qui génère une cage par rapport à un certain maillage
  */
MapHandlerGen* Surface_GenerationMultiDimensionalTool_Plugin::generationMultiDimensionalTool(const QString& mapName, const QString& positionAttributeName)
{
    MapParameters& p = h_parameterSet[mapName+positionAttributeName];
    MapHandlerGen* cageHandler;

    if(!p.m_initialized)
    {
        p.start();
        MapHandler<PFP2>* mh = static_cast<MapHandler<PFP2>*>(m_schnapps->getMap(mapName));
        PFP2::MAP* selectedMap = mh->getMap();
        VertexAttribute<PFP2::VEC3> position = selectedMap->getAttribute<PFP2::VEC3, VERTEX>(positionAttributeName.toStdString());
        if(!position.isValid())
        {
            CGoGNerr << "L'attribut de position choisi pour la carte sélectionnée n'est pas valide." << CGoGNendl;
            return NULL;
        }
        p.m_bb = Algo::Geometry::computeBoundingBox<PFP2>(*selectedMap, position);
    }

    if(p.m_toVoxellise)
    {

        if(p.m_toCalculateResolutions)
        {
            calculateResolutions(mapName, positionAttributeName);
        }

        voxellise(mapName, positionAttributeName);
        p.m_voxellisation.marqueVoxelsExterieurs();
        p.m_voxellisation.remplit();
        p.m_voxellisation.dilate();
        p.m_dilatation = 1;
    }

    cageHandler = extractionCarte(mapName, positionAttributeName);
    return cageHandler;
}

void Surface_GenerationMultiDimensionalTool_Plugin::voxellise(const QString& mapName, const QString& positionAttributeName)
{
    MapParameters& p = h_parameterSet[mapName+positionAttributeName];

    if(p.m_initialized)
    {
        Utils::Chrono chrono;
        chrono.start();
        MapHandler<PFP2>* mh = static_cast<MapHandler<PFP2>*>(m_schnapps->getMap(mapName));
        PFP2::MAP* selectedMap = mh->getMap();

        VertexAttribute<PFP2::VEC3> position = selectedMap->getAttribute<PFP2::VEC3, VERTEX>(positionAttributeName.toStdString());
        if(!position.isValid())
        {
            CGoGNerr << "L'attribut de position choisi pour la carte sélectionnée n'est pas valide." << CGoGNendl;
            return;
        }

        VertexAttribute<Voxel> voxel = selectedMap->getAttribute<Voxel, VERTEX>("voxel");
        if(!voxel.isValid())
        {
            voxel = selectedMap->addAttribute<Voxel, VERTEX>("voxel");
        }
        else
        {
            selectedMap->removeAttribute<Voxel, VERTEX>(voxel);
            voxel = selectedMap->addAttribute<Voxel, VERTEX>("voxel");
        }

        p.m_voxellisation = Algo::Surface::Modelisation::Voxellisation(p.m_resolutions, p.m_bb);

        TraversorF<PFP2::MAP> trav_face_map(*selectedMap);
        std::vector<Geom::Vec3i> polygone = std::vector<Geom::Vec3i>();

        for(Dart d = trav_face_map.begin(); d!=trav_face_map.end(); d=trav_face_map.next())
        {
            //On traverse l'ensemble des faces de la carte
            Traversor2FV<PFP2::MAP> trav_face_vert(*selectedMap,d);
            polygone.clear();
            for(Dart e = trav_face_vert.begin(); e!=trav_face_vert.end(); e=trav_face_vert.next())
            {
                //On récupère les sommets composants la face courante
                if(!voxel[e].isInitialise())
                    voxel[e].setIndexes(getVoxelIndex(mapName, positionAttributeName, position[e]));
                polygone.push_back(voxel[e].getIndexes());
            }
            p.m_voxellisation.voxellisePolygone(polygone);
        }

        p.m_toVoxellise = false;
        m_generationMultiDimensionalToolDialog->updateAppearanceFromPlugin(p.m_independant, p.m_resolutions[0]!=0);

        CGoGNout << "Temps de voxellisation : " << chrono.elapsed() << " ms" << CGoGNendl;
    }
}

void Surface_GenerationMultiDimensionalTool_Plugin::dilaterVoxellisation(const QString& mapName, const QString& positionAttributeName)
{
    MapParameters& p = h_parameterSet[mapName+positionAttributeName];

    if(p.m_initialized)
    {
        if(p.m_toVoxellise)
        {
            MapHandler<PFP2>* mh = static_cast<MapHandler<PFP2>*>(m_schnapps->getMap(mapName));
            PFP2::MAP* selectedMap = mh->getMap();
            VertexAttribute<PFP2::VEC3> position = selectedMap->getAttribute<PFP2::VEC3, VERTEX>(positionAttributeName.toStdString());
            if(!position.isValid())
            {
                CGoGNerr << "L'attribut de position choisi pour la carte sélectionnée n'est pas valide." << CGoGNendl;
                return;
            }

            voxellise(mapName, positionAttributeName);
        }
        Utils::Chrono chrono;
        chrono.start();
        p.m_voxellisation.dilate();
        CGoGNout << "Temps de dilatation : " << chrono.elapsed() << " ms." << CGoGNendl;
        m_generationMultiDimensionalToolDialog->updateNiveauDilatationFromPlugin(++p.m_dilatation);
        extractionCarte(mapName, positionAttributeName);
    }
}

void Surface_GenerationMultiDimensionalTool_Plugin::reinitialiserVoxellisation(const QString& mapName, const QString& positionAttributeName)
{
    MapParameters& p = h_parameterSet[mapName+positionAttributeName];

    if(p.m_initialized)
    {
        p.m_toVoxellise = true;
        generationMultiDimensionalTool(mapName, positionAttributeName);
        m_generationMultiDimensionalToolDialog->updateNiveauDilatationFromPlugin(p.m_dilatation);
    }
}

MapHandlerGen* Surface_GenerationMultiDimensionalTool_Plugin::extractionCarte(const QString& mapName, const QString& positionAttributeName)
{
    MapParameters& p = h_parameterSet[mapName+positionAttributeName];
    MapHandlerGen* cageHandler = NULL;

    if(p.m_initialized)
    {
        cageHandler = m_schnapps->getMap(mapName+QString("Cage"));

        if(cageHandler==NULL)
        {
            //Si la carte représentant la cage de ce maillage n'existait pas encore
            cageHandler = m_schnapps->addMap(mapName+QString("Cage"), 2);
        }

        MapHandler<PFP2>* mh = static_cast<MapHandler<PFP2>*>(cageHandler);

        PFP2::MAP* mapCage = mh->getMap();
        mapCage->clear(true);

        VertexAttribute<PFP2::VEC3> positionCage = mapCage->getAttribute<PFP2::VEC3, VERTEX>("position");
        if(!positionCage.isValid())
        {
            positionCage = mapCage->addAttribute<PFP2::VEC3, VERTEX>("position");
            mh->registerAttribute(positionCage);
        }

        Utils::Chrono chrono;
        chrono.start();
        if(p.m_extractionFaces)
        {
            //Si l'algorithme choisi est celui de l'extraction de faces
            p.m_voxellisation.extractionBord();

            std::vector<std::string> attrNamesCage;
            if(!Algo::Surface::Import::importVoxellisation<PFP2>(*mapCage, p.m_voxellisation, attrNamesCage, true))
            {
                CGoGNerr << "Impossible d'importer la voxellisation" << CGoGNendl ;
                return NULL;
            }

            CGoGNout << "Temps d'extraction des faces : " << chrono.elapsed() << " ms." << CGoGNendl;

            positionCage = mapCage->getAttribute<PFP2::VEC3, VERTEX>(attrNamesCage[0]);
        }
        else
        {
            //Si l'algorithme choisi est celui du marching cube
            Algo::Surface::MC::Image<int>* image = p.m_voxellisation.getImage();   //On récupère l'image correspondant à la voxellisation
            Algo::Surface::MC::WindowingEqual<int> windowing;
            windowing.setIsoValue(1); //L'intérieur est représenté avec une valeur de '1'
            Algo::Surface::MC::MarchingCube<int, Algo::Surface::MC::WindowingEqual,PFP2> marching_cube(image, mapCage, positionCage, windowing, false);
            marching_cube.simpleMeshing();
            marching_cube.recalPoints(p.m_bb.min()-Geom::Vec3f(image->getVoxSizeX()*(p.m_dilatation+1), image->getVoxSizeY()*(p.m_dilatation+1), image->getVoxSizeZ()*(p.m_dilatation+1)));

            CGoGNout << "Temps de réalisation du Marching Cube : " << chrono.elapsed() << " ms." << CGoGNendl;

            delete image;
            positionCage = mapCage->getAttribute<PFP2::VEC3, VERTEX>("position");
        }

        mh->updateBB(positionCage); //Met a jour la boite englobante de la carte

        mh->notifyAttributeModification(positionCage);  //Met a jour le VBO

        mh->notifyConnectivityModification();
    }
    return cageHandler;
}

void Surface_GenerationMultiDimensionalTool_Plugin::calculateResolutions(const QString& mapName, const QString& positionAttributeName)
{
    MapParameters& p = h_parameterSet[mapName+positionAttributeName];

    if(p.m_initialized) {
        MapHandler<PFP2>* mh = static_cast<MapHandler<PFP2>*>(m_schnapps->getMap(mapName));
        PFP2::MAP* selectedMap = mh->getMap();

        Geom::Vec3f bb_min = p.m_bb.min();
        Geom::Vec3f bb_max = p.m_bb.max();

        const float SPARSE_FACTOR=0.001f;

        //Résolution calculée avec la méthode de "Automatic Generation of Coarse Bounding Cages from Dense Meshes"
        int n = std::sqrt((selectedMap->getNbOrbits<VERTEX>())*SPARSE_FACTOR/6);

        Algo::Surface::Modelisation::swapVectorMax(bb_min, bb_max);

        float delta_x = bb_max[0]-bb_min[0];
        float delta_y = bb_max[1]-bb_min[1];
        float delta_z = bb_max[2]-bb_min[2];

        float max = std::max(std::max(delta_x, delta_y), delta_z);  //On récupère la composante qui a l'écart maximum

        //On adapte la résolution calculée pour qu'elle soit différente dans chacune des composantes x, y et z
        do
        {
            //On recalcule les résolutions jusqu'à ce que chacune d'entre elle ne soit plus nulle
            p.m_resolutions[0] = n*delta_x/max;
            p.m_resolutions[1] = n*delta_y/max;
            p.m_resolutions[2] = n*delta_z/max;
            n++;
        } while(p.m_resolutions[0]==0 || p.m_resolutions[1]==0 || p.m_resolutions[2]==0);

        m_generationMultiDimensionalToolDialog->updateResolutionsFromPlugin(p.m_resolutions);

        p.m_toVoxellise = true;
        p.m_toCalculateResolutions = false;
    }
}

/*
  * Fonction qui met à jour les résolutions dans chacun des composantes en fonction de la valeur choisie en x
  */
void Surface_GenerationMultiDimensionalTool_Plugin::updateResolutions(const QString& mapName, const QString& positionAttributeName)
{
    MapParameters& p = h_parameterSet[mapName+positionAttributeName];

    if(p.m_initialized)
    {
        if(!p.m_independant)
        {
            //Si les coordonnées sont calculées de façon indépendante
            Geom::Vec3f bb_min = p.m_bb.min();
            Geom::Vec3f bb_max = p.m_bb.max();

            int res_x = p.m_resolutions[0];

            Algo::Surface::Modelisation::swapVectorMax(bb_min, bb_max);

            float delta_x = bb_max[0]-bb_min[0];
            float delta_y = bb_max[1]-bb_min[1];
            float delta_z = bb_max[2]-bb_min[2];

            float max = std::max(std::max(delta_x, delta_y), delta_z);  //On récupère la composante qui a l'écart maximum

            if(res_x<=0)
                res_x = 1;

            int n = res_x*max/delta_x;

            //On adapte la résolution calculée pour qu'elle soit différente dans chacune des composantes x, y et z
            do
            {
                //On recalcule les résolutions jusqu'à ce que chacune d'entre elle ne soit plus nulle
                p.m_resolutions[0] = n*delta_x/max;
                p.m_resolutions[1] = n*delta_y/max;
                p.m_resolutions[2] = n*delta_z/max;
                n++;
            } while(p.m_resolutions[0]==0 || p.m_resolutions[1]==0 || p.m_resolutions[2]==0);
        }
        else
        {
            int res_x = m_generationMultiDimensionalToolDialog->spin_resolution_x->text().toInt();
            int res_y = m_generationMultiDimensionalToolDialog->spin_resolution_y->text().toInt();
            int res_z = m_generationMultiDimensionalToolDialog->spin_resolution_z->text().toInt();
            p.m_resolutions[0] = res_x>0?res_x:1;
            p.m_resolutions[1] = res_y>0?res_y:1;
            p.m_resolutions[2] = res_z>0?res_z:1;
        }
        m_generationMultiDimensionalToolDialog->updateResolutionsFromPlugin(p.m_resolutions);

        p.m_toVoxellise = true;
        p.m_toCalculateResolutions = false;
    }
}

/*
  * Fonction permettant de récupérer les indices du voxel associé au sommet 'a'
  */
Geom::Vec3i& Surface_GenerationMultiDimensionalTool_Plugin::getVoxelIndex(const QString& mapName, const QString& positionAttributeName, Geom::Vec3f a)
{
    MapParameters& p = h_parameterSet[mapName+positionAttributeName];

    Geom::Vec3i* voxel = new Geom::Vec3i();

    if(p.m_initialized)
    {
        Geom::Vec3f bb_min = p.m_bb.min();
        Geom::Vec3f bb_max = p.m_bb.max();

        Algo::Surface::Modelisation::swapVectorMax(bb_min, bb_max);

        for(int i=0; i<3; ++i)
        {
            if(bb_max[i]-bb_min[i]>0)
                voxel->data()[i] = p.m_voxellisation.getResolution(i)/(bb_max[i]-bb_min[i])*(a[i]-bb_min[i]);
            else
                voxel->data()[i] = 0;


            if(voxel->data()[i] == p.m_voxellisation.getResolution(i))
                --voxel->data()[i];
        }
    }

    return *voxel;
}

#ifndef DEBUG
Q_EXPORT_PLUGIN2(Surface_GenerationMultiDimensionalTool_Plugin, Surface_GenerationMultiDimensionalTool_Plugin)
#else
Q_EXPORT_PLUGIN2(Surface_GenerationCage_MultiDimensionalToolD, Surface_GenerationMultiDimensionalTool_Plugin)
#endif

} // namespace SCHNApps

} // namespace CGoGN
