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

    connect(m_dockTab->button_initializeCages, SIGNAL(clicked()), this, SLOT(initializeCages()));

    return true;
}

void Surface_GenerationMDTool_Plugin::disable()
{
}

void Surface_GenerationMDTool_Plugin::initializeCages()
{
    MapHandlerGen* mhg_selected = m_schnapps->getSelectedMap();
    if(mhg_selected)
    {
        MapHandler<PFP2>* mh_selected = static_cast<MapHandler<PFP2>*>(mhg_selected);
        PFP2::MAP* selectedMap = mh_selected->getMap();

        VertexAttribute<PFP2::VEC3> position = selectedMap->getAttribute<PFP2::VEC3, VERTEX>("position");

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
            if(isInCage(position[d], m_cages[0])) {
                CGoGNout << "Appartient à la première cage" << CGoGNendl;

            }
            else if(isInCage(position[d], m_cages[1])) {
                CGoGNout << "Appartient à la deuxième cage" << CGoGNendl;
            }
            else if(isInCage(position[d], m_cages[2])) {
                CGoGNout << "Appartient à la troisième cage" << CGoGNendl;
            }
            else if(isInCage(position[d], m_cages[3])) {
                CGoGNout << "Appartient à la quatrième cage" << CGoGNendl;
            }
        }
    }
}

//Check whether a point is in the given cage
bool Surface_GenerationMDTool_Plugin::isInCage(PFP2::VEC3 point, std::vector<PFP2::VEC3> cage)
{
    return cage[0][0]<=point[0] && cage[0][1]<=point[1] && cage[0][2]<=point[2]
       &&  cage[1][0]>=point[0] && cage[1][1]>=point[1] && cage[1][2]>=point[2];
}

#ifndef DEBUG
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_Plugin, Surface_GenerationMDTool_Plugin)
#else
Q_EXPORT_PLUGIN2(Surface_GenerationMDTool_PluginD, Surface_GenerationMDTool_Plugin)
#endif

} // namespace SCHNApps

} // namespace CGoGN
