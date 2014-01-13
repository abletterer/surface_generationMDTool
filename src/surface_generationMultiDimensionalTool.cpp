#include "surface_generationMultiDimensionalTool.h"

#include "mapHandler.h"

namespace CGoGN
{

namespace SCHNApps
{

bool Surface_GenerationMultiDimensionalTool_Plugin::enable()
{
    m_surface_generationMultiDimensionalTool_dockTab = new Surface_GenerationMultiDimensionalTool_DockTab(m_schnapps);

    m_generationMultiDimensionalToolAction = new QAction("Generate multidimensional tool", this);

    m_schnapps->addMenuAction(this, "Surface;Generate multidimensional tool", m_generationMultiDimensionalToolAction);

    connect(m_generationMultiDimensionalToolAction, SIGNAL(triggered()), this, SLOT(openGenerationMultiDimensionalToolDialog()));

    connect(m_schnapps, SIGNAL(mapAdded(MapHandlerGen*)), this, SLOT(mapAdded(MapHandlerGen*)));
    connect(m_schnapps, SIGNAL(mapRemoved(MapHandlerGen*)), this, SLOT(mapRemoved(MapHandlerGen*)));

    foreach(MapHandlerGen* map, m_schnapps->getMapSet().values())
        mapAdded(map);

    return true;
}

void Surface_GenerationMultiDimensionalTool_Plugin::disable()
{
    disconnect(m_generationMultiDimensionalToolAction, SIGNAL(triggered()), this, SLOT(openGenerationMultiDimensionalToolDialog()));

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
    m_surface_generationMultiDimensionalTool_dockTab->show();
}

#ifndef DEBUG
Q_EXPORT_PLUGIN2(Surface_GenerationMultiDimensionalTool_Plugin, Surface_GenerationMultiDimensionalTool_Plugin)
#else
Q_EXPORT_PLUGIN2(Surface_GenerationCage_MultiDimensionalToolD, Surface_GenerationMultiDimensionalTool_Plugin)
#endif

} // namespace SCHNApps

} // namespace CGoGN
