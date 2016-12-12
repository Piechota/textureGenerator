#pragma once

#ifdef _DEBUG
#pragma comment( lib, "../qtLib/Qt5Cored" )
#pragma comment( lib, "../qtLib/qtmaind" )
#pragma comment( lib, "../qtLib/Qt5Widgetsd" )
#pragma comment( lib, "../qtLib/Qt5Guid" )
#else
#pragma comment( lib, "../qtLib/Qt5Core" )
#pragma comment( lib, "../qtLib/qtmain" )
#pragma comment( lib, "../qtLib/Qt5Widgets" )
#pragma comment( lib, "../qtLib/Qt5Gui" )
#endif

#include <assert.h>
#include "render.h"

#include <QtCore/QString>
