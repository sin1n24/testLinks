
class dxff
{
	int linecntr, cyclocntr, ver;
	point	*pline;
	cyclo	*pcircle;
	std::ifstream fin;
	
	static const int length = maxFileName;  // 一行の最大長さ(一度に読み込む長さ)
	char buf[length];
	bool already10, alreadyPoly;

	bool pl_circle();
	bool polyline();
	bool pl_curve( int );
	void version();
	
  public:
	
	bool read( const char[], point[], cyclo[] );
	
	
	bool read123( point&);

	dxff();
	
};

dxff::dxff()
{
	//std::locale::global( std::locale( "japanese" ) );
	already10 = alreadyPoly =  false;
}

bool dxff::read( const char fname[], point lines[], cyclo ens[] )
{
	linecntr = 0;
	cyclocntr = 0;
	pline = lines;
	pcircle = ens;
	
	fin.open( fname );
	if( !fin )  return false;
	

	while( fin.getline( buf, length ) )
	{
		if ( !strcmp( buf, "$ACADVER" ) ) version();

  		if ( !strcmp( buf, "AcDbCircle" ) ) pl_circle();
		
		if ( ver == 2012 && !strcmp( buf, "AcDb2dPolyline" ) ) polyline();
		if ( ver == 2015 && !strcmp( buf, "AcDbPolyline" ) ) polyline();
		if( alreadyPoly ) polyline();
	}
	pline[linecntr] = pline[linecntr-1];
	pcircle[cyclocntr].r = 0;
	
	return true;
}

void dxff::version()
{
	fin.getline( buf, length );// 1
	fin.getline( buf, length );// data
	if( !strcmp( buf, "AC1012" ) ) ver = 2012;
	else if( !strcmp( buf, "AC1015" ) ) ver = 2015;
	else ver = 0;
}

bool dxff::read123( point &pt)
{
	while( !already10 && fin.getline( buf, length ) && strcmp( buf, " 10" ) ){
		if( (ver == 2012 && !strcmp( buf, "SEQEND" )) || (ver == 2015 && !strcmp( buf, "ENDSEC" )) ){
			return false;
		}else if((ver == 2012 && !strcmp( buf, "AcDb2dPolyline" )) || (ver == 2015 && !strcmp( buf, "AcDbPolyline" )) ){
			alreadyPoly = true;
			return false;
		}
	}already10 = false;
		//既読でなく、正常に読込め、bufが 10やSEQENDでない間、読み飛ばす
	
	fin.getline( buf, length );// 10
	pt.x = atof( buf );
	
	fin.getline( buf, length );// 20
	fin.getline( buf, length );
	pt.y = - atof( buf );

	if( ver == 2012 ){
		fin.getline( buf, length );// 30 読み飛ばし
		fin.getline( buf, length );
	}
	return true;
}

bool dxff::pl_circle()
{
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
	alreadyPoly = false;
	point dummy;
	if( ver == 2012 && !read123( dummy ) ) return false;
	while( fin.getline( buf, length) && strcmp( buf, " 70" )  && strcmp( buf, "VERTEX" ) );
	fin.getline( buf, length );
	int first = ( atoi(buf) == 1 ) ? linecntr : -1;
	already10 = !strcmp( buf, " 10" );
	if( !strcmp( buf, "VERTEX" ) ) first = -1;
	
	while( !fin.eof() )
	{
		if( read123( pline[linecntr] ) )
		{
			pline[linecntr].heel = true;
			if( pline[linecntr] != pline[linecntr-1] ) linecntr++;
			
			fin.getline( buf, length );
			already10 = !strcmp( buf, " 10" );
			if( !strcmp( buf, " 42" ) && cfg.plspan ){
				if( !pl_curve( first ) ){
					break;
				}
			}
			
		}else{ // SEQEND
			break;
		}
	}
	if( first != -1 ) pline[linecntr++] = pline[first];
	pline[linecntr-1].heel = false;
	return true;
}

bool dxff::pl_curve( int first )
{	
	bool spline = false, seq = false;
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
			already10 = !strcmp( buf, " 10" );
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


void fline( std::ofstream &fout, point a, point b, bool DXF=false )
{
	if( DXF )
		fout << " 0\nLINE\n 8\n0\n 10\n" << a.x << "\n 20\n" << -a.y << "\n 30\n0.0\n 11\n" << b.x  << "\n 21\n" << -b.y << "\n 31\n0.0\n";
	else
		fout << "_line\n" << a.x << "," << -a.y << "\n" << b.x << "," << -b.y << "\n\n";
}

void fpoint( std::ofstream &fout, point a, bool DXF=false )
{
	if( DXF )
		fout << " 0\nPOINT\n 8\n0\n 10\n" << a.x << "\n 20\n" << -a.y << "\n 30\n0.0\0\n";
	else
		fout << "_point\n" << a.x << "," << -a.y << "\n\n";
}

void fcircle( std::ofstream &fout, point a, double r, bool DXF=false )
{
	if( DXF )
		fout << " 0\nCIRCLE\n 8\n0\n 10\n" << a.x << "\n 20\n" << -a.y << "\n 30\n0.0\n 40\n" << r << "\n";
	else
		fout << "_circle\n" << a.x << "," << -a.y << "\n" << r << "\n";
}

void fpline( std::ofstream &fout, point pline[], bool DXF=false )
{
	if( DXF )
		fout << " 0\nPOLYLINE\n 8\n0\n 66\n1\n 10\n0.0\n 20\n0.0\n 30\n0.0\n";
	else
		fout << "_pline\n";

	for( int i=0; i<point::lim-1; i++ )
	{
		if( pline[i] == pline[i+1] )
		{
			if( DXF )
				fout << " 0\nVERTEX\n 8\n0\n 10\n" << pline[i].x << "\n 20\n" << -pline[i].y << "\n 30\n0.0\n 0\nSEQEND\n";
			else
				if( i ) fout << pline[i].x << "," << -pline[i].y << "\nc\n";  else fout << "\n\n";
			return;
		}
		if( cfg.dxfheel ){
			if( DXF )
				fout << " 0\nVERTEX\n 8\n0\n 10\n" << pline[i].x << "\n 20\n" << -pline[i].y << "\n 30\n0.0\n";
			else
				fout << pline[i].x << "," << -pline[i].y << "\n"; // next
		}else{
			if( pline[i].heel )
			{
				if( DXF )
					fout << " 0\nVERTEX\n 8\n0\n 10\n" << pline[i].x << "\n 20\n" << -pline[i].y << "\n 30\n0.0\n";
				else
					fout << pline[i].x << "," << -pline[i].y << "\n"; // next
			}
			else
			{
				if( DXF )
				{
					fout << " 0\nVERTEX\n 8\n0\n 10\n" << pline[i].x << "\n 20\n" << -pline[i].y << "\n 30\n0.0\n 0\nSEQEND\n";
					fout << " 0\nPOLYLINE\n 8\n0\n 66\n1\n 10\n0.0\n 20\n0.0\n 30\n0.0\n";
				}
				else
				{
					if( i ) fout << pline[i].x << "," << -pline[i].y << "\nc\n\n"; /* next end start(next) */
					else fout << "\n\n\n";
				}
			}
		}
		
	}
}

void fplineDXF( std::ofstream &fout, point pline[] )
{
	fout << " 0\nPOLYLINE\n 8\n0\n 66\n1\n 10\n0.0\n 20\n0.0\n 30\n0.0\n";
	
	for( int i=0; i<point::lim-1; i++ )
	{
		if( pline[i] == pline[i+1] )
		{
			 fout << " 0\nVERTEX\n 8\n0\n 10\n" << pline[i].x << "\n 20\n" << -pline[i].y << "\n 30\n0.0\n 0\nSEQEND\n";
			return;
		}
		if( cfg.dxfheel ){
			 fout << " 0\nVERTEX\n 8\n0\n 10\n" << pline[i].x << "\n 20\n" << -pline[i].y << "\n 30\n0.0\n";
		}else{
			if( pline[i].heel )
			 fout << " 0\nVERTEX\n 8\n0\n 10\n" << pline[i].x << "\n 20\n" << -pline[i].y << "\n 30\n0.0\n";
			else
			{
			 fout << " 0\nVERTEX\n 8\n0\n 10\n" << pline[i].x << "\n 20\n" << -pline[i].y << "\n 30\n0.0\n 0\nSEQEND\n";
			 fout << " 0\nPOLYLINE\n 8\n0\n 66\n1\n 10\n0.0\n 20\n0.0\n 30\n0.0\n";
			}
		}
		
	}

}