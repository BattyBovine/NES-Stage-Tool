#include <QApplication>
#include "nesstagetool.h"
#include "version.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setOrganizationName("Batty Bovine Productions, LLC");
	a.setOrganizationDomain("battybovine.com");
	a.setApplicationName("NES Stage Tool");
	a.setApplicationVersion(QString::number(Version::MAJOR)+
							QString::number(Version::MINOR)+
							QString::number(Version::REVISION)+
							QString::number(Version::BUILD));

	NESStageTool w;
	w.show();

	return a.exec();
}
