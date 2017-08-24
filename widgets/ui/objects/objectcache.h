#ifndef OBJECTCACHE_H
#define OBJECTCACHE_H

#include <QString>
#include <QPixmap>
#include <QSettings>

struct Object {
	QString name;
	QPixmap img;
};

class ObjectCache
{
public:
	static Object find(int id){return hObjectList[id];}
	static void replace(int id, Object pix){hObjectList.insert(id,pix);}
private:
	ObjectCache(){}
	static QHash<int,Object> hObjectList;
};

#endif // OBJECTCACHE_H
