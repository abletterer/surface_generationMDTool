#include "surface_generationMDTool_dockTab.h"

#include "schnapps.h"
#include "mapHandler.h"

namespace CGoGN
{

namespace SCHNApps
{

Surface_GenerationMDTool_DockTab::Surface_GenerationMDTool_DockTab(
        SCHNApps* s, Surface_GenerationMDTool_Plugin* p) :
    m_schnapps(s),
    m_plugin(p),
    b_updatingUI(false)
{
    setupUi(this);
}

} // namespace SCHNApps

} // namespace CGoGN
