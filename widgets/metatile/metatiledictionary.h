#ifndef METATILEDICTIONARY_H
#define METATILEDICTIONARY_H

#include <QPixmap>

class MetatileDictionary
{
public:
	static QHash<QString,QPixmap> hPixmapList;
	static QPixmap find(QString key){return hPixmapList[key];}
	static void insert(QString key, QPixmap p){hPixmapList.insert(key,p);}
private:
	MetatileDictionary(){}
};

#endif // METATILEDICTIONARY_H
