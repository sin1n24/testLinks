
class value
{
	static const int lim = 64;
	int		strcolor, rimcolor, bgcolor, HighModecolor, MidModecolor, LowModecolor;
	bool	focus, active/*, boolean*/, *bdata, lock;
	double	*data, magni, modemagni;
	point	pos, str;
	char	brief[lim], brieffalse[lim];
	enum	modes{ low, mid, high, modelim }mode;
	
  public:
	
	void setColor( int col ){ strcolor = rimcolor = HighModecolor =  MidModecolor =  LowModecolor = col; }
	void set3Color( int Hicol, int Midcol, int Lowcol ){ HighModecolor =  Hicol, MidModecolor =  Midcol, LowModecolor = Lowcol; }
	void setBGcolor( int col ){ bgcolor = col; }
	void setLock(){ lock = true; }
	void mun(){ active = false; }
	bool set( point, bool, bool, int );
	void init( int, int, double*, const char * );
	//void init( int, int, bool*, char[], char[] );
	void setMagni( double Magni ){ magni = Magni; lock = false; }
	bool getFocus(){ return focus; }
	bool getActive(){ return active; }
	value()
	{
		strcolor = -1, rimcolor = -1, bgcolor = 1, HighModecolor = -1, MidModecolor = -1, LowModecolor = -1;
		magni = 1; focus = lock/* = boolean*/ = active = false; mode = mid;
	}
};

bool value::set( point cursor, bool release, bool subrelease, int wheel )
{
	const int slit = 4,  w = 4/*GetDrawFormatStringWidth( " " )*/, h = cfg.fontsize, shift = slit-1;

	if( !active ) return false;
	
	focus = cursor.on( pos, str );
	
	if( focus )
	{
		DrawBox( pos.x, pos.y, pos.x+str.x+3*w, pos.y+cfg.fontsize, bgcolor, TRUE );
		DrawBox( pos.x-slit, pos.y-slit, pos.x+str.x+slit+3*w, pos.y+cfg.fontsize+slit, rimcolor, FALSE );
		
		

		if( !lock )
		{

			if( release ){
				char buf[maxFileName];
				if( KeyInputSingleCharString( pos.x, pos.y, maxFileName, buf, TRUE ) == 1 ) *data = atof( buf );
				//ê¨å˜
			}
			if( subrelease ) mode = static_cast<modes>((mode+1) % modelim);
			
			
			if( mode == high ){
				modemagni = 5;
				DrawBox( pos.x-shift+0*w+1, pos.y, 
					pos.x-shift+1*w+1, pos.y+h, HighModecolor, TRUE );
				DrawBox( pos.x-shift+1*w+2, pos.y, 
					pos.x-shift+2*w+2, pos.y+h, HighModecolor, TRUE );
				DrawBox( pos.x-shift+2*w+3, pos.y, 
					pos.x-shift+3*w+3, pos.y+h, HighModecolor, TRUE );
			}
			else if( mode == mid ){
				modemagni = 1;
				DrawBox( pos.x-shift+0*w+1, pos.y, 
					pos.x-shift+1*w+1, pos.y+h, MidModecolor, TRUE );
				DrawBox( pos.x-shift+1*w+2, pos.y, 
					pos.x-shift+2*w+2, pos.y+h, MidModecolor, TRUE );
			}
			else if( mode == low ){
				modemagni = 0.2;
				
				DrawBox( pos.x-shift+0*w+1, pos.y,  
					pos.x-shift+1*w+1, pos.y+h, LowModecolor, TRUE );
			}

			*data += wheel * magni * modemagni;
		}
		else
		{
			DrawLine( pos.x-shift+0*w+1, pos.y, 
					pos.x-shift+3*w+1, pos.y+cfg.fontsize, HighModecolor );
			DrawLine( pos.x-shift+3*w+1, pos.y, 
					pos.x-shift+0*w+1, pos.y+cfg.fontsize, HighModecolor );
		}
	}
	
	DrawFormatString( pos.x+3*w, pos.y+((focus&&wheel)?(wheel>0?2:-2):0), strcolor, brief, *data );
	
	return focus && wheel;
}

void value::init( int argx, int argy, double *Data, const char Brief[] )
{
	data = Data; pos.x = argx; pos.y = argy;
	strcpy( brief, " %7.2f " ); strcat( brief, Brief );
	str.set( GetDrawFormatStringWidth( brief, *data ), cfg.fontsize );
	active = true;
}/*
void value::init( int argx, int argy, bool *Data, char Brieft[], char Brieff[] )
{
	bdata = Data; pos.x = argx; pos.y = argy;
	strcpy( brief, Brieft ); strcpy( brieffalse, Brieff );
	const int strwtrue = GetDrawFormatStringWidth( brief ),
		strwfalse = GetDrawFormatStringWidth( brieffalse );
	str.set( (strwtrue>strwfalse ? strwtrue : strwfalse), cfg.fontsize );
	boolean = active = true;
}*/