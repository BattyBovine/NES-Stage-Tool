#ifndef TILESETCACHE_H
#define TILESETCACHE_H

#include <QPixmap>

#define TSC_PIXMAP_KEY_FORMAT "t%1p%2a%3"

class TilesetCache
{
public:
	static QPixmap find(int t,int p,int a){return hPixmapList[QString(TSC_PIXMAP_KEY_FORMAT).arg(t).arg(p).arg(a)];}
	static void insert(int t,int p,int a,QPixmap pix){hPixmapList.insert(QString(TSC_PIXMAP_KEY_FORMAT).arg(t).arg(p).arg(a),pix);}
private:
	TilesetCache(){}
	static QHash<QString,QPixmap> hPixmapList;
};

#endif // TILESETCACHE_H
