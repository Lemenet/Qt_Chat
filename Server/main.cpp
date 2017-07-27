#ifdef WIN32
#define _WIN32_WINNT 0x0501
#endif // WIN32


#include "Server.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Server w;
	w.show();
	return a.exec();
}
