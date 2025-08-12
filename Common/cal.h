
class point
{
  public:
	
	static const int lim = 1000;
	
	double	x, y;
	bool	heel;
	
	void	set( double arg ){ x = y = arg; }
	void	set( double argx, double argy ){ x = argx; y = argy; }
	void	set( double argx, double argy, bool argh ){ x = argx; y = argy; heel = argh; }
	void	setDiff( double argx, double argy ){ x += argx; y += argy; }
	
	void 	operator+=( point plus ){ x += plus.x; y += plus.y; }
	void 	operator+=( double plus ){ x += plus; y += plus; }
	bool 	operator==( point eq ){ return x == eq.x && y == eq.y; }
	bool 	operator!=( point eq ){ return x != eq.x || y != eq.y; }
	point	operator+( point plus ){ return point( x + plus.x, y + plus.y ); }
	point	operator-( point plus ){ return point( x - plus.x, y - plus.y ); }
	point	operator+( int plus ){ return point( x + plus, y + plus ); }
	point	operator-( int plus ){ return point( x - plus, y - plus ); }
	bool	operator!(){ return !x && !y; }
	point	operator*( double plus ){ return point( x * plus, y * plus ); }
	point	operator/( double plus ){ return point( x / plus, y / plus ); }
	
	point	offset( point, point, point );
	point	offset( point, point, double );
	point	offset( point, point );
	
	point	rota( point, double );
	double	dir( point );
	double	_dir( point );
	double	__dir( point );
	
	double	dist( point a ){ return hypot( x-a.x, y-a.y ); }
	bool	dist( point a, double hit ){ return pow(x-a.x,2) + pow(y-a.y,2) <= pow(hit,2); }
	
	bool	nigh( point, int );
	bool	on( point, int );
	bool	on( point, point );
	
	point( double argx, double argy ){ x = argx; y = argy; heel = true; }
	point(){ x = 0; y = 0; heel = true; }
};

point point::offset( point sub, point shift, double conrod )
{
	return point( x + ( shift.y * (x-sub.x) 
		+ shift.x * (y-sub.y) ) / (conrod ? conrod : 1) , 
		y + ( shift.y * (y-sub.y) 
		- shift.x * (x-sub.x) ) / (conrod ? conrod : 1) );
}

point point::offset( point sub, point shift )
{
	const double conrod = hypot( x-sub.x, y-sub.y );
	
	return point( x + ( shift.y * (x-sub.x) 
		+ shift.x * (y-sub.y) ) / (conrod ? conrod : 1) , 
		y + ( shift.y * (y-sub.y) 
		- shift.x * (x-sub.x) ) / (conrod ? conrod : 1) );
}

point point::rota( point center, double ang )  // 廻される側が関数を呼び出す
{
 	const point d( x-center.x, y-center.y );
	return point( center.x+d.x*cos(ang)-d.y*sin(ang), center.y+d.x*sin(ang)+d.y*cos(ang) );
}
bool point::nigh( point a, int hit )  // 高速だが判定域が正方形なので精度は低い。a点は中心点。
{
  if( x>a.x+hit || x<a.x-hit || y>a.y+hit || y<a.y-hit )  return false;
  else return true;
}
bool point::on( point a, int hit )  // 高速だが判定域が正方形なので精度は低い。a点は基点。
{
  if( x>a.x+hit || x<a.x || y>a.y+hit || y<a.y )  return false;
  else return true;
}
bool point::on( point a, point hit )
{
  if( x>a.x+hit.x || x<a.x || y>a.y+hit.y || y<a.y )  return false;
  else return true;
}


double point::dir( point sub )
{
	const double dx = x - sub.x, dy = y - sub.y;
	double dir;
	dir = atan( dy / dx );
	if( dx < 0 && dy < 0 )  dir += M_PI;
	else if( dx > 0 && dy < 0 )  dir -= 2*M_PI;
	else if( dx < 0 && dy > 0 )  dir -= M_PI;
	
	return dir;
}
double point::_dir( point sub )
{
	const double dx = x - sub.x, dy = y - sub.y;
	double dir = atan( abs(dy) / abs(dx) );
	
	if( dx > 0 && dy > 0 )  dir = dir;				// Quadrant 1
	else if( dx < 0 && dy > 0 ) dir = M_PI - dir;	// Quadrant 2
	else if( dx < 0 && dy < 0 ) dir = dir;			// Quadrant 3 
	else if( dx > 0 && dy < 0 ) dir = M_PI - dir;	// Quadrant 4
	else dir = 0;
	
	return dir;
}
double point::__dir( point sub )
{
	const double dx = x - sub.x, dy = y - sub.y;
	double dir = atan( abs(dy) / abs(dx) );
	
	if( dx > 0 && dy > 0 )  dir = dir;				// Quadrant 1
	else if( dx < 0 && dy > 0 ) dir = M_PI - dir;	// Quadrant 2
	else if( dx < 0 && dy < 0 ) dir = -M_PI + dir;	// Quadrant 3 
	else if( dx > 0 && dy < 0 ) dir = - dir;		// Quadrant 4
	else dir = 0;
	
	return dir;
}


point sincos( double angle )
{
	//if( angle == -1 ) return point(0,0);
	return point( cos(angle), sin(angle) );
}


class cyclo
{
  public:
	
	static const int lim = 100;
	
	point	pos;
	double	r;
	
	void	set( double argx, double argy, double argr ){ pos.x = argx; pos.y = argy; r = argr; }
	void	set( point argp, double argr ){ pos = argp; r = argr; }
	void	setDiff( double argx, double argy, double argr ){ pos.x += argx; pos.y += argy; r += argr; }
	
	cyclo( double argx, double argy, double argr ){ pos.x = argx; pos.y = argy; r = argr; }
	cyclo(){ pos.x = 0; pos.y = 0; r = 0; }
};