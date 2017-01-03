#include <QApplication>
#include "nesstagetool.h"
#include "version.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setOrganizationName("Batty Bovine Productions, LLC");
	a.setOrganizationDomain("battybovine.com");
    a.setApplicationName(QApplication::tr("NES Stage Tool"));
    a.setApplicationVersion(QString::number(Version::MAJOR)+QString(".")+
                            QString::number(Version::MINOR)+QString(".")+
                            QString::number(Version::REVISION)+QString(".")+
							QString::number(Version::BUILD));

	NESStageTool w;
	w.show();

	return a.exec();
}
