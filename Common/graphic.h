
point getMousePoint(){
	int i, j;
	GetMousePoint( &i, &j );
	return point( i, j );
}

class graphic
{
	double	scale;
	point	center;
	bool	wheelbtn, vertex;
	
	void	getwheelbtn();
	
  public:
	
	static const int cursor = 35;
	
	bool	r, c, l, R, C, L, RR, CC, LL, wp, wm;
	int		w;
	point	vir, real;
	
	void	getmouse();
	void	centerDrag( bool d ){ wheelbtn = d; }
	
	void	setScale( double s ){ scale = s; }
	void	setScaleDiff( double s ){ scale += s; }
	void	setCenter( point c ){ center = c; }
	point	getCenter(){ return center; }
	double	getScale(){ return scale; }
	void	setCenterDiff( point c ){ center += c; }
	
	void	line( point, point, int );
	void	line( double, double, double, double, int );
	void	dot( point, int );
	void	circle( point, double, int, bool );
	void	rect( point, point, int, bool );
	void	rectc( point, point, int, bool );
	void	rectc( point, double, int, bool );
	void	pline( point[], int );
	void	pcircle( cyclo[], int );
	
	
	graphic();
	graphic( double, double );
	graphic( point );
};

void graphic::getmouse()
{
	static bool	rr, cc, ll;
	
	real = getMousePoint();
	vir.x = (real.x - center.x) / scale;
	vir.y = (real.y - center.y) / scale;
	
	// 大文字がリリース検知、小文字が押された状態か否か
	L = !( l = GetMouseInput() & MOUSE_INPUT_LEFT ) && ll; ll = l;  // ショートカット対策で２文に分けた
	C = !( c = GetMouseInput() & MOUSE_INPUT_MIDDLE ) && cc; cc = c;
	R = !( r = GetMouseInput() & MOUSE_INPUT_RIGHT ) && rr; rr = r;
	
	LL = CC = RR = false;
	if( L || C || R ){
		static int beforetime = GetNowCount();
		const int spantime = GetNowCount() - beforetime;
		
		if( spantime <= GetDoubleClickTime() )
			if( L ) LL = true;
			else if( C ) CC = true;
			else if( R ) RR = true;
		
		beforetime = GetNowCount(); 
	}
	
	w = GetMouseWheelRotVol();
	if( w < 0 )  w = -1, wm = true, wp = false;
	else if( w > 0 )  w = 1, wp = true, wm = false;
	else wm = wp = false;
	
	if( wheelbtn ) getwheelbtn();
}

void graphic::getwheelbtn()
{
	static bool click_c = false;
	if( c ){
		static point before;
		if( !click_c ){
			before = real;
			click_c = true;
		}
		setCenterDiff( real - before );
		before = real;
	}else click_c = false;
}

void graphic::line( point a, point b, int color )
{
	DrawLine( center.x+scale*a.x, center.y+scale*a.y, 
		 center.x+scale*b.x, center.y+scale*b.y, color );
}

void graphic::line( double a, double b, double c, double d, int color )
{
	DrawLine( center.x+scale*a, center.y+scale*b, 
		 center.x+scale*c, center.y+scale*d, color );
}
void graphic::dot( point a, int color )
{
	DrawPixel( center.x+scale*a.x, center.y+scale*a.y, color );
}
void graphic::circle( point a, double r, int color, bool filled = false )
{
	DrawCircle( center.x+scale*a.x, 
		center.y+scale*a.y, scale*r, color, filled );
}
void graphic::rect( point a, point b, int color, bool filled = false )
{
	DrawBox( center.x+scale*a.x, center.y+scale*a.y, 
		center.x+scale*(a.x+b.x), center.y+scale*(a.y+b.y), color, filled );
}
void graphic::rectc( point a, point b, int color, bool filled = false )
{
	DrawBox( center.x+scale*(a.x-b.x), center.y+scale*(a.y-b.y), 
		center.x+scale*(a.x+b.x), center.y+scale*(a.y+b.y), color, filled );
}
void graphic::rectc( point a, double b, int color, bool filled = false )
{
	DrawBox( center.x+scale*(a.x-b), center.y+scale*(a.y-b), 
		center.x+scale*(a.x+b), center.y+scale*(a.y+b), color, filled );
}
void graphic::pline( point pline[], int color )
{
	for( int i=0; i<point::lim-1; i++ )
	{
		const int vertexsize = 3;
		if( vertex && pline[i].heel )
			DrawBox( center.x+scale*pline[i].x-vertexsize, center.y+scale*pline[i].y-vertexsize, 
				center.x+scale*pline[i].x+vertexsize, center.y+scale*pline[i].y+vertexsize, color, FALSE );
		
		if( pline[i].x == pline[i+1].x && 
			pline[i].y == pline[i+1].y )  return;
			
		if( pline[i].heel ){
			line( pline[i], pline[i+1], color );
		}
		else  dot( pline[i], color );
	}
}
void graphic::pcircle( cyclo pcircle[], int color )
{
	for( int i=0; i<cyclo::lim-1; i++ )
	{	
		if( !pcircle[i].r ) return;
		circle( pcircle[i].pos, pcircle[i].r, color );
	}
}

graphic::graphic( double x, double y )
{
	center.x = x; center.y = y;
	scale = 1;
	wheelbtn = false;
	vertex = false;
}
graphic::graphic( point a )
{
	center = a;
	scale = 1;
	wheelbtn = false;
	vertex = false;
}
graphic::graphic()
{
	center.x = 0; center.y = 0;
	scale = 1;
	wheelbtn = false;
	vertex = false;
}


void drawline( point a, point b )
{
	const double scale = 5;
	
	DrawLine( cfg.screen.x/2+scale*a.x, cfg.screen.y/2+scale*a.y, 
		 cfg.screen.x/2+scale*b.x, cfg.screen.y/2+scale*b.y, GetColor( 200,200,200 ) );
}
