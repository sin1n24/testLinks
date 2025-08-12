
inline double rad( double deg ){
	return deg * 2 * M_PI / 360;
}
inline double deg( double rad ){
	return rad * 360 / ( 2*M_PI );
}

double regularize( double angle )
{
	if( angle > 2*M_PI ) while( angle > 2*M_PI ) angle -= 2*M_PI;
	else if( angle < 0 ) while( angle < 0 ) angle += 2*M_PI;
	return angle;
}

double getDir( point datam, point sub )
{
	const double dx = datam.x - sub.x, dy = datam.y - sub.y;
	double dir;
	dir = atan( dy / dx );
	if( dx < 0 && dy < 0 )  dir += M_PI;
	else if( dx > 0 && dy < 0 )  dir -= 2*M_PI;
	else if( dx < 0 && dy > 0 )  dir -= M_PI;
	
	return dir;
}

double get_dir( point datam, point sub )
{
	//const double dx = datam.x - sub.x, dy = datam.y - sub.y;
	//double dir = atan( abs(dy) / abs(dx) );
	//
	//if( dx > 0 && dy > 0 )  dir = dir;				// Quadrant 1
	//else if( dx < 0 && dy > 0 ) dir = M_PI - dir;	// Quadrant 2
	//else if( dx < 0 && dy < 0 ) dir = dir;			// Quadrant 3 
	//else if( dx > 0 && dy < 0 ) dir = M_PI - dir;	// Quadrant 4
	//else dir = 0;
	//
	//return dir;
	return datam._dir( sub );
}

bool wait( int wait = 20 )  // 本来待つべき時間wait[ms]
{
	static int before_time = GetNowCount();
	const int draw_time = GetNowCount() - before_time;
	bool time_over = false;
	
	if( wait > draw_time ){
		WaitTimer( wait - draw_time );
	}else{
		time_over = true;
	}
	before_time = GetNowCount(); 
	
	return time_over;
}

void messageOK( const char msg[], const char title[] = "information" )
{
	MessageBox( GetMainWindowHandle(), msg, title, MB_OK | MB_ICONINFORMATION );
}
bool messageYN( const char msg[], const char title[] = "information" )
{
	return MessageBox( GetMainWindowHandle(), msg, title, MB_YESNO ) == IDYES;
}
int messageYNC( const char msg[], const char title[] = "information" )
{
	return MessageBox( GetMainWindowHandle(), msg, title, MB_YESNOCANCEL );
}

bool openFileDialog( char fname[], char filter[] = "全てのファイル形式(*.*)\0*.*\0\0"  )
{
	OPENFILENAME ofn = { 0 };
	
	ofn.lStructSize = sizeof( ofn );
	ofn.lpstrFilter = filter;
	ofn.hwndOwner = GetMainWindowHandle();
	ofn.lpstrFile = fname;
	ofn.nMaxFile = maxFileName;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrTitle = "ファイルを開く";
	ofn.lpstrDefExt = ".links";
	
	const bool ret = GetOpenFileName( &ofn );
	
	if( ret ) strcpy( fname, ofn.lpstrFile );
	
	return ret;
}

bool saveFileDialog( char fname[], char filter[] = "全てのファイル形式(*.*)\0*.*\0\0"  )
{
	OPENFILENAME ofn = { 0 };
	
	ofn.lStructSize = sizeof( ofn );
	ofn.lpstrFilter = filter;
	ofn.hwndOwner = GetMainWindowHandle();
	ofn.lpstrFile = fname;
	ofn.nMaxFile = maxFileName;
	ofn.lpstrTitle = "ファイルを保存";
	ofn.lpstrDefExt = ".links";
	
	const bool ret = GetSaveFileName( &ofn );
	
	if( ret ) strcpy( fname, ofn.lpstrFile );
	
	return ret;
}

bool saveDrawScreen( point p1, point p2, char fname[] )
{
	return SaveDrawScreen( p1.x, p1.y, p2.x, p2.y, fname );
}

point offset( point datam, point sub, point shift, double conrod )
{
	if( !conrod ) conrod = hypot( datam.x-sub.x, datam.y-sub.y );
	return point( datam.x + ( shift.y * (datam.x-sub.x) 
		+ shift.x * (datam.y-sub.y) ) / (conrod ? conrod : 1) , 
		datam.y + ( shift.y * (datam.y-sub.y) 
		- shift.x * (datam.x-sub.x) ) / (conrod ? conrod : 1) );
}

bool clipboard( const char Str[] )
{
	int    BufSize;
	char  *Buf;
	HANDLE hMem;

	BufSize = strlen( Str ) + 1;                               // 確保するメモリのサイズを計算する
	hMem = GlobalAlloc( GMEM_SHARE | GMEM_MOVEABLE, BufSize ); // 移動可能な共有メモリを確保する
	if ( !hMem )
	{
		return FALSE;
	}

	Buf = (char *)GlobalLock( hMem ); // 確保したメモリをロックし、アクセス可能にする
	if ( Buf )
	{
		strcpy( Buf, Str );   // 文字列を複写する。
		GlobalUnlock( hMem ); // メモリのロックを解除する。
		if ( OpenClipboard(NULL) )
		{
			EmptyClipboard();                  // クリップボード内の古いデータを解放する
			SetClipboardData( CF_TEXT, hMem ); // クリップボードに新しいデータを入力する
			CloseClipboard();

			// クリップボードにテキストデータがコピーされた。
			// コピーされたメモリはクリップボードが管理するので解放してはならない。
			//
			// 関数 SetClipboardData() に関する MSDN の記述が、
			//
			//    The application can read the data,
			//   but must not free the handle or leave it locked.
			//   ↓
			//    The application can read the data,
			//   but must not free the handle or leave it locked
			//   until the CloseClipboard function is called.
			//
			// と改訂されているようだ。
			// 関数 SetClipboardData() に渡すために確保されたメモリは、
			// 関数 CloseClipboard() の呼び出し後には解放して良いのかもしれない。
			GlobalFree( hMem ); // 判断微妙
			return TRUE;
		}
	}
	GlobalFree( hMem );
	return FALSE;
}//SetClipboardText

bool setWindowText( const char str[] )
{
	SetMainWindowText( str );
	//SetWindowText( str ); error
	return true;
}
