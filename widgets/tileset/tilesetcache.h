#ifndef TILESETCACHE_H
#define TILESETCACHE_H

#include <QPixmap>

#define TILESETCACHE_PIXMAP_KEY_FORMAT "t%1p%2a%3"
#define METATILECACHE_PIXMAP_KEY_FORMAT "t%1m%2p%3a%4"

class TilesetCache
{
public:
	static QPixmap find(int t,int p,int a){return hPixmapList[QString(TILESETCACHE_PIXMAP_KEY_FORMAT).arg(t).arg(p).arg(a)];}
	static void insert(int t,int p,int a,QPixmap pix){hPixmapList.insert(QString(TILESETCACHE_PIXMAP_KEY_FORMAT).arg(t).arg(p).arg(a),pix);}
private:
	TilesetCache(){}
	static QHash<QString,QPixmap> hPixmapList;
};

class MetatileCache
{
public:
	static QPixmap find(int t,int m,int p,int a){return hPixmapList[QString(METATILECACHE_PIXMAP_KEY_FORMAT).arg(t).arg(m).arg(p).arg(a)];}
	static void insert(int t,int m,int p,int a,QPixmap pix){hPixmapList.insert(QString(METATILECACHE_PIXMAP_KEY_FORMAT).arg(t).arg(m).arg(p).arg(a),pix);}
private:
	MetatileCache(){}
	static QHash<QString,QPixmap> hPixmapList;
};

#endif // TILESETCACHE_H
