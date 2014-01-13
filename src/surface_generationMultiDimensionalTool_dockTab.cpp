#include "surface_generationMultiDimensionalTool_dockTab.h"

#include "schnapps.h"
#include "mapHandler.h"

namespace CGoGN
{

namespace SCHNApps
{

Surface_GenerationMultiDimensionalTool_DockTab::Surface_GenerationMultiDimensionalTool_DockTab(SCHNApps* s) :
    m_schnapps(s),
    m_selectedMap(NULL)
{
    setupUi(this);

    connect(m_schnapps, SIGNAL(mapAdded(MapHandlerGen*)), this, SLOT(addMapToList(MapHandlerGen*)));
    connect(m_schnapps, SIGNAL(mapRemoved(MapHandlerGen*)), this, SLOT(removeMapFromList(MapHandlerGen*)));

    connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selectedMapChanged()));
}

void Surface_GenerationMultiDimensionalTool_DockTab::addMapToList(MapHandlerGen* m)
{
}

void Surface_GenerationMultiDimensionalTool_DockTab::removeMapFromList(MapHandlerGen* m)
{
}

void Surface_GenerationMultiDimensionalTool_DockTab::selectedMapChanged()
{
}

} // namespace SCHNApps

} // namespace CGoGN
