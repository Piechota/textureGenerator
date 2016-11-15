#pragma once

#ifdef _DEBUG
#pragma comment( lib, "../qtLib/Qt5Cored" )
#pragma comment( lib, "../qtLib/qtmaind" )
#pragma comment( lib, "../qtLib/Qt5Widgetsd" )
#else
#pragma comment( lib, "../qtLib/Qt5Core" )
#pragma comment( lib, "../qtLib/qtmain" )
#pragma comment( lib, "../qtLib/Qt5Widgets" )
#endif

#include <QtCore/QString>
