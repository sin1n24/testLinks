
class icon
{
	static const int lim = 64;
	int strcolor, rimcolor, bgcolor;
	int		id_non, id_lock, delay;
	char	caret_non[lim], caret_lock[lim];
	point	pos;
	bool	lock, focus, pushback;
	
  public:
	
	static const int size = 22;
	
	void setColor( int col ){ strcolor = rimcolor = col; }
	void setBGcolor( int col ){ bgcolor = col; }
	void	active(){ lock = !lock; } 
	void	set( point, bool );
	bool	get(){ return lock; }
	bool	getFocus(){ return focus; }
	void	init( int, int, char[], char[], char[], char[]);
	void	init( int, int, int, char[], int, char[] );
	
	icon( int, int, char[], char[], char[], char[] );
	icon(){ caret_non[0] = '\0'; caret_lock[0] = '\0'; lock = false; }
};

void icon::set( point cursor, bool release )
{
	focus = cursor.on( pos, size ) && caret_non[0] != '\0';
	if( pushback ) lock = ( release && focus );
	else if( release && focus ) lock = !lock;
	
	const int delaylim = (id_non==-1)?0:12, downsize = 4;
	
	DrawGraph( pos.x, pos.y+(focus ? (release?0:downsize):0), lock ? id_lock : id_non, FALSE );
	
	if( focus && (++delay > delaylim) ){
		const int strw = GetDrawFormatStringWidth( lock ? caret_lock : caret_non );
		const int overx = (cursor.x + strw > cfg.screen.x)? -1 : 1, 
			overy = (cursor.y + cfg.fontsize + graphic::cursor > cfg.screen.y)? -1 : 1;
		const int slit = 3;
		
		DrawBox( cursor.x - slit*overx, cursor.y + graphic::cursor*overy - slit, 
			cursor.x + strw*overx + slit*overx, cursor.y + cfg.fontsize + graphic::cursor*overy + slit, bgcolor, TRUE );
		DrawBox( cursor.x - slit*overx, cursor.y + graphic::cursor*overy - slit, 
			cursor.x + strw*overx + slit*overx, cursor.y + cfg.fontsize + graphic::cursor*overy + slit, rimcolor, FALSE );
			
		DrawString( cursor.x - (overx==-1?strw:0), cursor.y + graphic::cursor*overy, 
			lock ? caret_lock : caret_non, strcolor );
	}
	if( !focus ) delay = 0;
}

void icon::init( int x, int y, char in[], char cn[], char il[]="", char cl[]="" )
{
	pos.set(x,y);
	strcpy( caret_non, cn );
	DeleteGraph( id_non );
	id_non = LoadGraph( in );
	
	strcpy( caret_lock, cl );
	DeleteGraph( id_lock );
	id_lock = LoadGraph( il );
	
	pushback = ( il[0] == '\0' && cl[0] == '\0' );
}

void icon::init( int x, int y, int in, char cn[], int il=0, char cl[]="" )
{
	pos.set(x,y);
	strcpy( caret_non, cn );
	DeleteGraph( id_non );
	id_non = LoadGraphToResource( in );
	
	strcpy( caret_lock, cl );
	DeleteGraph( id_lock );
	id_lock = LoadGraphToResource( il );
	
	pushback = ( il == 0 && cl[0] == '\0' );
}

icon::icon( int x, int y, char in[], char cn[], char il[]="", char cl[]="" )
{
	strcolor = -1, rimcolor = -1, bgcolor = 1;
	caret_non[0] = '\0'; caret_lock[0] = '\0';
	init( x, y, in, cn, il, cl );
	lock = false;
}
