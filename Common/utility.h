
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

bool wait( int wait = 20 )  // �{���҂ׂ�����wait[ms]
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

bool openFileDialog( char fname[], char filter[] = "�S�Ẵt�@�C���`��(*.*)\0*.*\0\0"  )
{
	OPENFILENAME ofn = { 0 };
	
	ofn.lStructSize = sizeof( ofn );
	ofn.lpstrFilter = filter;
	ofn.hwndOwner = GetMainWindowHandle();
	ofn.lpstrFile = fname;
	ofn.nMaxFile = maxFileName;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrTitle = "�t�@�C�����J��";
	ofn.lpstrDefExt = ".links";
	
	const bool ret = GetOpenFileName( &ofn );
	
	if( ret ) strcpy( fname, ofn.lpstrFile );
	
	return ret;
}

bool saveFileDialog( char fname[], char filter[] = "�S�Ẵt�@�C���`��(*.*)\0*.*\0\0"  )
{
	OPENFILENAME ofn = { 0 };
	
	ofn.lStructSize = sizeof( ofn );
	ofn.lpstrFilter = filter;
	ofn.hwndOwner = GetMainWindowHandle();
	ofn.lpstrFile = fname;
	ofn.nMaxFile = maxFileName;
	ofn.lpstrTitle = "�t�@�C����ۑ�";
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

	BufSize = strlen( Str ) + 1;                               // �m�ۂ��郁�����̃T�C�Y���v�Z����
	hMem = GlobalAlloc( GMEM_SHARE | GMEM_MOVEABLE, BufSize ); // �ړ��\�ȋ��L���������m�ۂ���
	if ( !hMem )
	{
		return FALSE;
	}

	Buf = (char *)GlobalLock( hMem ); // �m�ۂ��������������b�N���A�A�N�Z�X�\�ɂ���
	if ( Buf )
	{
		strcpy( Buf, Str );   // ������𕡎ʂ���B
		GlobalUnlock( hMem ); // �������̃��b�N����������B
		if ( OpenClipboard(NULL) )
		{
			EmptyClipboard();                  // �N���b�v�{�[�h���̌Â��f�[�^���������
			SetClipboardData( CF_TEXT, hMem ); // �N���b�v�{�[�h�ɐV�����f�[�^����͂���
			CloseClipboard();

			// �N���b�v�{�[�h�Ƀe�L�X�g�f�[�^���R�s�[���ꂽ�B
			// �R�s�[���ꂽ�������̓N���b�v�{�[�h���Ǘ�����̂ŉ�����Ă͂Ȃ�Ȃ��B
			//
			// �֐� SetClipboardData() �Ɋւ��� MSDN �̋L�q���A
			//
			//    The application can read the data,
			//   but must not free the handle or leave it locked.
			//   ��
			//    The application can read the data,
			//   but must not free the handle or leave it locked
			//   until the CloseClipboard function is called.
			//
			// �Ɖ�������Ă���悤���B
			// �֐� SetClipboardData() �ɓn�����߂Ɋm�ۂ��ꂽ�������́A
			// �֐� CloseClipboard() �̌Ăяo����ɂ͉�����ėǂ��̂�������Ȃ��B
			GlobalFree( hMem ); // ���f����
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
