#ifndef _SURFACE_GENERATIONMDTOOL_PLUGIN_H_
#define _SURFACE_GENERATIONMDTOOL_PLUGIN_H_

#include "plugin_interaction.h"

#include <QKeyEvent>

#include "surface_generationMDTool_dockTab.h"

#include "Geometry/bounding_box.h"

#include "Utils/Shaders/shaderColorPerVertex.h"
#include "Utils/drawer.h"

#include "Algo/Modelisation/subdivision.h"
#include "Algo/Modelisation/voxellisation.h"

#include "Algo/Tiling/Surface/square.h"

#include "mapHandler.h"

#include "qimage.h"

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

    virtual void draw(View* view);
    virtual void drawMap(View* view, MapHandlerGen* map) {}

    virtual void keyPress(View* view, QKeyEvent* event);
    virtual void keyRelease(View* view, QKeyEvent* event) {}
    virtual void mousePress(View* view, QMouseEvent* event);
    virtual void mouseRelease(View* view, QMouseEvent* event) {}
    virtual void mouseMove(View* view, QMouseEvent* event) {}
    virtual void wheelEvent(View* view, QWheelEvent* event) {}

    virtual void viewLinked(View* view) {}
    virtual void viewUnlinked(View* view) {}

private :
    void createCages(MapHandler<PFP2>* mh_object, int nbCagesPerRow, int nbCagesPerColumn, PFP2::REAL scale);

    void addNewFace();

private slots:

    void selectedMapChanged(MapHandlerGen *prev, MapHandlerGen *cur);

public slots:
    //Slots for Python calls
    void initializeObject(const QString& view, const QString& filename="", int x = 0, int y = 0);
    void initializeCages(const QString& view, const int nbCagesPerRow, const int nbCagesPerColumn, const float scale);

protected:
    Surface_GenerationMDTool_DockTab* m_dockTab;
    std::deque<std::deque<Dart> > m_cages;

    CGoGN::Utils::ShaderColorPerVertex* m_colorPerVertexShader;
    Utils::VBO* m_positionVBO;
    Utils::VBO* m_colorVBO;

    Utils::Drawer* m_drawer;

    bool m_toDraw;

    bool m_addFaces;
    bool m_addVertices;

    std::vector<PFP2::VEC3> m_verticesCurrentlyAdded;
};

} // namespace SCHNApps

} // namespace CGoGN

#endif

