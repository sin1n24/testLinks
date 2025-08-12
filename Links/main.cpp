
#include "DxLib.h"

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <cmath>
#include <fstream>
#include <string>
#include "resource"
#include "../Common/cal.h"
#include "../Common/gradual.h"
#include "config.h"
#include "../Common/dxf.h"
#include "../Common/utility.h"
#include "../Common/graphic.h"
#include "../Common/icon.h"
#include "../Common/value.h"
#include "hecken.h"


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
				 LPSTR lpCmdLine, int nCmdShow )
{
	//std::locale::global( std::locale( "japanese" ) );
	//SetUseDXArchiveFlag( TRUE );
	SetGraphMode( cfg.screen.x, cfg.screen.y, 16 );
	ChangeWindowMode( TRUE );
	{
		char buf[maxFileName];
		strcpy( cfg.title, "Links ver1.09L" );
		strcpy( buf, cfg.title );
		strcat( buf, "   Loading... " );
		setWindowText( buf );
	}
	SetOutApplicationLogValidFlag( TRUE ) ;
	SetBackgroundColor( cfg.BGcolor, cfg.BGcolor, cfg.BGcolor );
	SetDoubleStartValidFlag( TRUE );
	if( DxLib_Init() )	return -1;
	SetFontThickness( cfg.bold ? 6 : 1 );
	SetFontSize( cfg.fontsize );
	SetDrawScreen( DX_SCREEN_BACK );
	//SetWindowSizeChangeEnableFlag( TRUE );
	SetDragFileValidFlag( TRUE );
	DragFileInfoClear();

	graphic	draw( cfg.screen.x/2, cfg.screen.y/2 );
	draw.centerDrag( true );
	
	hecken	link( lpCmdLine[0] == '\0' ? cfg.name : lpCmdLine );
	
	while( !ProcessMessage() )
	{
		draw.getmouse();
		
		link.proceed( draw );
		
		ScreenFlip();
		ClsDrawScreen();
	}
	DxLib_End();
	return 0;
}