#ifndef _SURFACE_GENERATIONMDTOOL_PLUGIN_H_
#define _SURFACE_GENERATIONMDTOOL_PLUGIN_H_

#include "plugin_interaction.h"

#include "surface_generationMDTool_dockTab.h"

#include "Algo/Import/import.h"

namespace CGoGN
{

namespace SCHNApps
{

class Surface_GenerationMDTool_Plugin : public PluginInteraction
{
	Q_OBJECT
	Q_INTERFACES(CGoGN::SCHNApps::Plugin)

    friend class Surface_GenerationMDTool_DockTab;

public:
    Surface_GenerationMDTool_Plugin()
	{}

    ~Surface_GenerationMDTool_Plugin()
	{}

	virtual bool enable();
	virtual void disable();

    virtual void draw(View* view) {}
    virtual void drawMap(View* view, MapHandlerGen* map) {}

    virtual void keyPress(View* view, QKeyEvent* event) {}
    virtual void keyRelease(View* view, QKeyEvent* event) {}
    virtual void mousePress(View* view, QMouseEvent* event) {}
    virtual void mouseRelease(View* view, QMouseEvent* event) {}
    virtual void mouseMove(View* view, QMouseEvent* event) {}
    virtual void wheelEvent(View* view, QWheelEvent* event) {}

    virtual void viewLinked(View* view) {}
    virtual void viewUnlinked(View* view) {}

private slots:
    void selectedMapChanged(MapHandlerGen* pev, MapHandlerGen* cur);

public slots:
    //Slots for Python calls

protected:
    Surface_GenerationMDTool_DockTab* m_dockTab;
};

} // namespace SCHNApps

} // namespace CGoGN

#endif

