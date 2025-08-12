
enum rlxy{ RX, RY, LX, LY, RLXY };
const int btnlim = 12;

class gamepad
{
	int		rightx, righty, leftx, lefty, id;
	int		*p[RLXY];
	
  public:
	
	bool	set( rlxy, rlxy, rlxy, rlxy );
	void	setid( int ID ){ id = ID; }
	int		getid(){ return id; }
	
	double	rx(){ return (double)rightx/1000; }
	double	ry(){ return (double)righty/1000; }
	double	lx(){ return (double)leftx/1000; }
	double	ly(){ return (double)lefty/1000; }
	
	bool	up, down, right, left;
	bool	btn[btnlim+1];
	
	void	drawstick( graphic );
	void	get();
	
	gamepad();
};

void gamepad::get()
{
	GetJoypadAnalogInput( p[LX], p[LY], id );
	GetJoypadAnalogInputRight( p[RX], p[RY], id );
	if( !GetJoypadNum() ){
		*p[LX] = 1000*( CheckHitKey(KEY_INPUT_RIGHT) - CheckHitKey(KEY_INPUT_LEFT) );
		*p[LY] = 1000*( CheckHitKey(KEY_INPUT_UP) - CheckHitKey(KEY_INPUT_DOWN) );
	}
	btn[1] = 	GetJoypadInputState( id ) & PAD_INPUT_A;
	btn[2] = 	GetJoypadInputState( id ) & PAD_INPUT_B;
	btn[3] = 	GetJoypadInputState( id ) & PAD_INPUT_C;
	btn[4] = 	GetJoypadInputState( id ) & PAD_INPUT_X;
	btn[5] = 	GetJoypadInputState( id ) & PAD_INPUT_Y;
	btn[6] = 	GetJoypadInputState( id ) & PAD_INPUT_Z;
	btn[7] = 	GetJoypadInputState( id ) & PAD_INPUT_L;
	btn[8] = 	GetJoypadInputState( id ) & PAD_INPUT_R ;
	btn[9] = 	GetJoypadInputState( id ) & PAD_INPUT_START;
	btn[10] = 	GetJoypadInputState( id ) & PAD_INPUT_M;
	//btn[11] = GetJoypadInputState( id ) & PAD_INPUT_ ;
	//btn[12] = GetJoypadInputState( id ) & PAD_INPUT_ ;
	up = 		GetJoypadInputState( id ) & PAD_INPUT_UP;
	down = 		GetJoypadInputState( id ) & PAD_INPUT_DOWN;
	left = 		GetJoypadInputState( id ) & PAD_INPUT_LEFT;
	right = 	GetJoypadInputState( id ) & PAD_INPUT_RIGHT;
}

bool gamepad::set( rlxy a, rlxy b, rlxy c, rlxy d )
{
	if( a+b+c+d != 6 ) return -1;
	p[a] = &rightx;
	p[b] = &righty;
	p[c] = &leftx;
	p[d] = &lefty;
	return 0;
}

void gamepad::drawstick( graphic draw )
{
	const int x=5, y=5, width=50, pass=5, r=3, color=GetColor( 64,64,64 );
	
	DrawBox( x, y, x+width, y+width, color, FALSE );
	DrawBox( x+width+pass, y, x+2*width+pass, y+width, color, FALSE );
	
	DrawLine( x+width/2, y+width/2, 
		x+width/2+leftx*width/2/1000, y+width/2+lefty*width/2/1000, color );
	DrawLine( x+width+pass+width/2, y+width/2, 
		x+width+pass+width/2+rightx*width/2/1000, y+width/2+righty*width/2/1000, color );
	
	DrawCircle( x+width/2+leftx*width/2/1000, y+width/2+lefty*width/2/1000, r, color );
	DrawCircle( x+width+pass+width/2+rightx*width/2/1000, y+width/2+righty*width/2/1000, r, color );
}

gamepad::gamepad()
{
	p[RX] = &rightx;
	p[RY] = &righty;
	p[LX] = &leftx;
	p[LY] = &lefty;
	id = DX_INPUT_PAD1;
	
	for( int i=0; i<btnlim; i++ ) btn[i] = false;
	up = false; down = false; left = false; right = false;
}