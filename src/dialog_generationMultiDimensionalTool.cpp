#include "dialog_generationMultiDimensionalTool.h"

#include "schnapps.h"
#include "mapHandler.h"

namespace CGoGN
{

namespace SCHNApps
{

Dialog_GenerationMultiDimensionalTool::Dialog_GenerationMultiDimensionalTool(SCHNApps* s) :
    m_schnapps(s),
    m_selectedMap(NULL)
{
    setupUi(this);

    connect(m_schnapps, SIGNAL(mapAdded(MapHandlerGen*)), this, SLOT(addMapToList(MapHandlerGen*)));
    connect(m_schnapps, SIGNAL(mapRemoved(MapHandlerGen*)), this, SLOT(removeMapFromList(MapHandlerGen*)));

    connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selectedMapChanged()));

    foreach(MapHandlerGen* map,  m_schnapps->getMapSet().values())
    {
        QListWidgetItem* item = new QListWidgetItem(map->getName(), list_maps);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
    spin_resolution_x->setKeyboardTracking(false);
    spin_resolution_y->setKeyboardTracking(false);
    spin_resolution_z->setKeyboardTracking(false);
}

void Dialog_GenerationMultiDimensionalTool::addMapToList(MapHandlerGen* m)
{
    QListWidgetItem* item = new QListWidgetItem(m->getName(), list_maps);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void Dialog_GenerationMultiDimensionalTool::removeMapFromList(MapHandlerGen* m)
{
    QList<QListWidgetItem*> items = list_maps->findItems(m->getName(), Qt::MatchExactly);
    if(!items.empty())
        delete items[0];

    if(m_selectedMap == m)
    {
        disconnect(m_selectedMap, SIGNAL(attributeAdded(unsigned int, const QString&)), this, SLOT(addAttributeToList(unsigned int, const QString&)));
        m_selectedMap = NULL;
    }
}

void Dialog_GenerationMultiDimensionalTool::addAttributeToList(unsigned int orbit, const QString& nameAttr)
{
    QString vec3TypeName = QString::fromStdString(nameOfType(PFP2::VEC3()));

    const QString& typeAttr = m_selectedMap->getAttributeTypeName(orbit, nameAttr);

    if(typeAttr == vec3TypeName)
    {   //On n'ajoute l'élment que s'il est de type Vec3
        //C'est-à-dire s'il peut être interprêté comme un point
        combo_positionAttribute->addItem(nameAttr);
    }
}

void Dialog_GenerationMultiDimensionalTool::selectedMapChanged()
{
    if(m_selectedMap)
        disconnect(m_selectedMap, SIGNAL(attributeAdded(unsigned int, const QString&)), this, SLOT(addAttributeToList(unsigned int, const QString&)));

    QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
    if(!currentItems.empty())
    {
        combo_positionAttribute->clear();

        const QString& mapname = currentItems[0]->text();
        MapHandlerGen* mh = m_schnapps->getMap(mapname);

        QString vec3TypeName = QString::fromStdString(nameOfType(PFP2::VEC3()));

        unsigned int j = 0;
        const AttributeSet& attribs = mh->getAttributeSet(VERTEX);
        for(AttributeSet::const_iterator i = attribs.constBegin(); i != attribs.constEnd(); ++i)
        {
            if(i.value() == vec3TypeName)
            {
                combo_positionAttribute->addItem(i.key());
                ++j;
            }
        }

        m_selectedMap = mh;
        connect(m_selectedMap, SIGNAL(attributeAdded(unsigned int, const QString&)), this, SLOT(addAttributeToList(unsigned int, const QString&)));
    }
    else
        m_selectedMap = NULL;
}

} // namespace SCHNApps

} // namespace CGoGN
