
class dxff
{
	int linecntr, cyclocntr;
	point	*pline;
	cyclo	*pcircle;
	std::ifstream fin;
	
	static const int length = maxFileName;  // 一行の最大長さ(一度に読み込む長さ)
	
  public:
	
	bool read( const char[], point[], cyclo[] );
	bool pl_circle();
	bool polyline();
	bool pl_curve( int );
	
	
	bool read123( point& );

	dxff();
	
};

dxff::dxff()
{
	std::locale::global( std::locale( "japanese" ) );
}

bool dxff::read( const char fname[], point lines[], cyclo ens[] )
{
	char buf[length];
	
	linecntr = 0;
	cyclocntr = 0;
	pline = lines;
	pcircle = ens;
	
	fin.open( fname );
	if( !fin )  return false;
	
	while( fin.getline( buf, length ) )
	{
  		if ( !strcmp( buf, "AcDbCircle" ) ) pl_circle();
		
		if ( !strcmp( buf, "AcDb2dPolyline" ) ) polyline();
	}
	pline[linecntr] = pline[linecntr-1];
	pcircle[cyclocntr].r = 0;
	
	return true;
}

bool dxff::read123( point &pt )
{
	char buf[length];
	
	while( fin.getline( buf, length ) && strcmp( buf, " 10" ) && strcmp( buf, "SEQEND" ) );
		//正常に読込め、bufが 10やSEQENDでない間、読み飛ばす
	if( !strcmp( buf, "SEQEND" ) ) return false;
	
	fin.getline( buf, length );// 10
	pt.x = atof( buf );
	
	fin.getline( buf, length );// 20
	fin.getline( buf, length );
	pt.y = - atof( buf );
	
	fin.getline( buf, length );// 30 読み飛ばし
	fin.getline( buf, length );
	
	return true;
}

bool dxff::pl_circle()
{
	char buf[length];
	
	point c;
	double r;
	
	//pline[linecntr-1].heel = false;
	
	if( !read123( c ) ) return false;
	
	fin.getline( buf, length );// 40
	fin.getline( buf, length );
	r = atof( buf );
	
	pcircle[cyclocntr++].set( c, r?r:1 );
	
	/*　擬似円（上下動に影響あり）
	for( int cnt=0; cnt<32; cnt++ ){
		pline[linecntr].x = c.x + r*cos( cnt*2*M_PI/32 );
		pline[linecntr].y = c.y + r*sin( cnt*2*M_PI/32 );
		pline[linecntr++].heel = true;
	}
	pline[linecntr-1].heel = false;*/
	
	
	return true;
}

bool dxff::polyline()
{
	char buf[length];
	point dummy;
	if( !read123( dummy ) ) return false;
	fin.getline( buf, length );
	int first = ( !strcmp( buf, " 70" ) )? linecntr : -1;
	
	while( !fin.eof() )
	{
		if( read123( pline[linecntr] ) )
		{
			pline[linecntr].heel = true;
			if( pline[linecntr] != pline[linecntr-1] ) linecntr++;
			
			fin.getline( buf, length );
			if( !strcmp( buf, " 42" ) && cfg.plspan )
				if( !pl_curve( first ) ) return true;
			
		}else{ // SEQEND
			if( first != -1 ) pline[linecntr++] = pline[first];
			pline[linecntr-1].heel = false;
			return true;
		}
	}
	return true;
}

bool dxff::pl_curve( int first )
{
	char buf[length];
	
	bool spline = false;
	bool seq = false;
	point oldend;
	
	do{	
		fin.getline( buf, length ); 
		const double alpha = 4 * fabs( atan( atof(buf) ) );
		if( !alpha ) break;//{ printfDx( "%d:%s (%d)\n", linecntr, buf, spline ); break; }
		const double cw = (atof(buf) < 0)?1:-1;
		point start, end, center;
		
		if( spline ) start = oldend;
		else start = pline[linecntr-1];
		
		if( read123( end ) )
		{
			fin.getline( buf, length );
			spline = !strcmp( buf, " 42" );
			oldend = end;
		}else{
			end.heel = false;
			if( first != -1 ) end = pline[first], seq = true;
			else return false;
		}
		double len = start.dist( end ), delta = cfg.plspan;
		double r = len / ( 2 * sin( alpha/2 ) );
		double beta = cw*(M_PI-alpha)/2;
		double dt = atan( (end.y-start.y)/(end.x-start.x) ) + ((end.x-start.x >= 0)?0.:M_PI);
		
		center.x = start.x + r * cos( dt + beta );
		center.y = start.y + r * sin( dt + beta );
		
		/*
		point diff = end - start;
		double len = start.dist( end );
		double delta = cfg.plspan;
		double r = len / 2 / sin( alpha/2 );
		
		//printfDx( "%d : %f,%f - %f ..%f\n", i, diff.x, diff.y, r, alpha*180/M_PI );ScreenFlip();
		
		double cA = 2*diff.x/(len), sA = 2*diff.y/(len);
		double cB = cos( (M_PI-alpha)/2 ), sB = sin( (M_PI-alpha)/2 );
		
		center.x = start.x + cw * r * ( cA*cB - sA*sB );
		center.y = start.y + cw * r * ( sA*cB + cA*sB );
		*/
		
		int memoi = linecntr;
		while( !pline[linecntr-1].dist( end, delta ) )
		{
			pline[linecntr] = pline[linecntr-1].rota( center, cw * delta/r );
			pline[linecntr++].heel = true;
			
			if( linecntr - memoi > 256 ) break;
		}
		if( pline[linecntr] != end ) pline[linecntr++] = end;
		if( seq ) return false;
		
	}while( spline );
	
	return true;
}


void fline( std::ofstream &fout, point a, point b )
{
	fout << "_line\n" << a.x << "," << -a.y << "\n" << b.x << "," << -b.y << "\n\n";
}

void fpoint( std::ofstream &fout, point a )
{
	fout << "_point\n" << a.x << "," << -a.y << "\n\n";
}

void fcircle( std::ofstream &fout, point a, double r )
{
	fout << "_circle\n" << a.x << "," << -a.y << "\n" << r << "\n";
}

void fpline( std::ofstream &fout, point pline[] )
{
	
	fout << "_pline\n";
	for( int i=0; i<point::lim-1; i++ )
	{
		if( pline[i] == pline[i+1] )
		{
			if( i ) fout << pline[i].x << "," << -pline[i].y << "\nc\n";
			else fout << "\n\n";
			return;
		}
		if( cfg.dxfheel ){
			fout << pline[i].x << "," << -pline[i].y << "\n"; // next
		}else{
			if( pline[i].heel )
				fout << pline[i].x << "," << -pline[i].y << "\n"; // next
			else
			{
				if( i ) fout << pline[i].x << "," << -pline[i].y << "\nc\n\n"; // next end start(next)  //\nc\n
				else fout << "\n\n\n";
			}
		}
		
	}
}