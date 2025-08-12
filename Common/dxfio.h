/*bool readDXF( const char fname[], point pline[] )  // for R13/LT95
{
	std::ifstream fin( fname );
	if( !fin )  return false;
	
	const int length = maxFileName;  // 一行の最大長さ(一度に読み込む長さ)
	char buf[length];
	int i = 0;
	
	while( fin.getline( buf, length ) ) // file loop
	{
		if ( !strcmp( buf, "AcDbCircle" ) ) // circle loop
		{
			while( fin.getline( buf, length ) )
			{
				if( !strcmp( buf, " 10" ) )
				{
					point circle;
					double r;
					
					fin.getline( buf, length );
					circle.x = atof( buf );
					
					fin.getline( buf, length );// 20
					fin.getline( buf, length );
					circle.y = - atof( buf );
					
					fin.getline( buf, length );// 30 読み飛ばし
					fin.getline( buf, length );
					
					fin.getline( buf, length );// 40
					fin.getline( buf, length );
					r = atof( buf );
					
					for( int cnt=0; cnt<=32; cnt++ ){
						pline[i].x = circle.x + r*cos( cnt*2*M_PI/32 );
						pline[i].y = circle.y + r*sin( cnt*2*M_PI/32 );
						pline[i].heel = true;
						i++;
					}
					pline[i-1].heel = false;
					break;
				}
			}
		}
		
		if ( !strcmp( buf, "AcDb2dPolyline" ) ) // pline loop
		{
			int first;
			
			while( fin.getline( buf, length ) ) // 原点確認情報無視
			{
				if( !strcmp( buf, " 30" ) ){
					fin.getline( buf, length );// data of 30
					fin.getline( buf, length );
					first = ( !strcmp( buf, " 70" ) )? i : -1;
					break;
				}
			}
			
			while( fin.getline( buf, length ) ) // pline facter loop
			{
				if( !strcmp( buf, "SEQEND" ) ) // 切れ目の前でheel
				{
					if( first != -1 ) pline[i++] = pline[first];
					pline[i-1].heel = false;
					break;
				}
				if( !strcmp( buf, " 10" ) )
				{
					
					fin.getline( buf, length );
					pline[i].x = atof( buf );
					
					fin.getline( buf, length );// 20 読み飛ばし
					fin.getline( buf, length );
					pline[i].y = - atof( buf );
					
					pline[i].heel = true;
					
					if( pline[i] == pline[i-1] ); else i++; // 連続点回避(point!=未定義)
					
					
					fin.getline( buf, length );// 30 読み飛ばし
					fin.getline( buf, length );
					
					
					fin.getline( buf, length );
					if( !strcmp( buf, " 42" ) && cfg.plspan )
					{
						bool spline = false;
						bool seq = false;
						
						do{	
							static point oldend;
							
							fin.getline( buf, length ); 
							const double alpha = 4 * fabs( atan( atof(buf) ) );
							if( !alpha )break;//{ printfDx( "%d:%s (%d)\n", i, buf, spline ); break; }
							const bool cw = atof(buf) < 0;
							point start, end, center;
							
							if( spline ) start = oldend;
							else start = pline[i-1];
							while( fin.getline( buf, length ) )
							{
								if( !strcmp( buf, " 10" ) )
								{
									fin.getline( buf, length );
									end.x = atof( buf );
									
									fin.getline( buf, length );// 20 読み飛ばし
									fin.getline( buf, length );
									end.y = - atof( buf );
									//end.heel = true;
									
									fin.getline( buf, length );// 30 読み飛ばし
									fin.getline( buf, length );
									
									fin.getline( buf, length );
									if( !strcmp( buf, " 42" ) ) spline = true;
									else spline = false;
									oldend = end;
									
									break;
								}
								if( !strcmp( buf, "SEQEND" ) )
								{
									//if( first != -1 ) end = pline[first];
									end = pline[first];
									end.heel = false;
									seq = true;
								}
							}
							double len = start.dist( end ), delta = cfg.plspan;
							double r = len / ( 2 * sin( alpha/2 ) );
							double beta = (cw?1:-1)*(M_PI-alpha)/2;
							double dt = atan( (end.y-start.y)/(end.x-start.x) ) + ((end.x-start.x >= 0)?0.:M_PI);
							
							center.x = start.x + r * cos( dt + beta );
							center.y = start.y + r * sin( dt + beta );
							
							
							int memoi = i;
							while( !pline[i-1].dist( end, delta ) )
							{
								pline[i] = pline[i-1].rota( center, (cw?1:-1)*delta/r );
								pline[i++].heel = true;
								
								if( i - memoi > 64 ) break;
							}
							pline[i++] = end;
							if( seq ) break;
						}while( spline );
					}// end if (42)
				} // after 10
			} // pline facter loop-end
		} // pline loop
	}
	pline[i] = pline[i-1];
	
	return true;
}*/
