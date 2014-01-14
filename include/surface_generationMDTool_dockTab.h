#ifndef _SURFACE_GENERATI0NMDTOOL_DOCKTAB_H_
#define _SURFACE_GENERATI0NMDTOOL_DOCKTAB_H_

#include "ui_surface_generationMDTool.h"

#include "Geometry/vector_gen.h"

namespace CGoGN
{

namespace SCHNApps
{

class SCHNApps;
class MapHandlerGen;
class Surface_GenerationMDTool_Plugin;
class MapParameters;

class Surface_GenerationMDTool_DockTab : public QWidget, public Ui::Surface_GenerationMDTool_TabWidget
{
    Q_OBJECT

    friend class Surface_GenerationMDTool_Plugin;

public:
    Surface_GenerationMDTool_DockTab(SCHNApps* s, Surface_GenerationMDTool_Plugin* p);

private:
    SCHNApps* m_schnapps;
    Surface_GenerationMDTool_Plugin* m_plugin;
    bool b_updatingUI;

private slots:
};

} // namespace SCHNApps

} // namespace CGoGN

#endif
