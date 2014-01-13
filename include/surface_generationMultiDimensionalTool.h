#ifndef _SURFACE_GENERATIONCAGE_PLUGIN_H_
#define _SURFACE_GENERATIONCAGE_PLUGIN_H_

#include "plugin_processing.h"

#include "surface_generationMultiDimensionalTool_dockTab.h"

#include "Algo/Import/import.h"

namespace CGoGN
{

namespace SCHNApps
{

class Surface_GenerationMultiDimensionalTool_Plugin : public PluginProcessing
{
	Q_OBJECT
	Q_INTERFACES(CGoGN::SCHNApps::Plugin)

    friend class Surface_GenerationMultiDimensionalTool_DockTab;

public:
    Surface_GenerationMultiDimensionalTool_Plugin()
	{}

    ~Surface_GenerationMultiDimensionalTool_Plugin()
	{}

	virtual bool enable();
	virtual void disable();

private slots:
	void mapAdded(MapHandlerGen* map);
	void mapRemoved(MapHandlerGen* map);
    void attributeModified(unsigned int orbit, QString nameAttr);

    void openGenerationMultiDimensionalToolDialog();

public slots:
private:
    Surface_GenerationMultiDimensionalTool_DockTab* m_surface_generationMultiDimensionalTool_dockTab;
    QAction* m_generationMultiDimensionalToolAction;
};

} // namespace SCHNApps

} // namespace CGoGN

#endif

