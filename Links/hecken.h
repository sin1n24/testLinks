enum{ mnew, mopen, msaveas, msave, mconfig/*, mrelord*/, mstop, mpara, mstatus, mdsmode, mvtmode, mcmd, marcmode,
	mturbo, mdcmode, mmmlmode, mfomode, mromode, msomode, manibmp, mlnkside, mturn, mdxf, morbit, mdxfout, mresol, mgraph, menulim };
enum{ tcrankr, tcrankx, tcranky, tleverr, tleverx, tlevery, tshiftx, tshifty, tconrod, tsubconrod, 
	tsubcrankr, tsubcrankx, tsubcranky, tthetaplus, tsubleverr, tsubleverx, tsublevery, tsubshiftx, tsubshifty,
	ttheta, tobject, tscale, tturn, /*tside, tdxf, */tupdown, troll, ttilt, theight, tmini, tmax, tstep, tresol, tgraph, tablelim };

enum{ gleverjoint, gtoe, gground, gcrankjoint, gsubcrankjoint, gsubleverjoint, gsubtoe, glim };

bool limiter = true;

class hecken
{
	static const int  colorlim = 60;
	
	// link parameter
	point	crank, lever, shift, subcrank, sublever, subshift, optimizraw[point::lim], bgraw[point::lim], 
		rearleg_orbit[point::lim], frontleg_orbit[point::lim], secret_orbit[point::lim], 
		conrodraw[point::lim], leverraw[point::lim], crankraw[point::lim], subraw[point::lim];
	cyclo	conroden[cyclo::lim], leveren[cyclo::lim], cranken[cyclo::lim], suben[cyclo::lim], bgen[cyclo::lim];
	double	crankr, leverr, subcrankr, subleverr, conrod, subconrod, theta, mini, max, step, height, 
		object, resol, roll, tilt, phase, thetaplus;
	bool	next, side, rebirth, turn, dxf, revcalc;
	enum	{ mhecken, mslider, mdualhecken, mdualslider, mdualcrank }mode;
	bool	doublecrank, arcmode;
	
	// reckoned parameter (should be decrease)
	point	crankjoint, leverjoint, toe, subcrankjoint, subleverjoint, subtoe, ground, graphline, graphtext, 
		secret_orbited[point::lim], orbit[point::lim], optimized[point::lim], conroded[point::lim], 
		levered[point::lim], cranked[point::lim], subed[point::lim], bged[point::lim];
	cyclo	conrodened[cyclo::lim], leverened[cyclo::lim], crankened[cyclo::lim], subened[cyclo::lim];
	double	updown, scale, tiltold, min_frontleg_orbit, min_reartleg_orbit, graph_mode;
	int		color[colorlim], optmax ;
	bool	timeover, lock, dxfcrank, dxflever, dxfconrod, dxfsub, dxfbg, turbomode, helpmode;
	bool	exist_frontleg_orbit, exist_rearleg_orbit, exist_secret_orbit, exist_maxmin_length, exist_graph;
	
	// interface
	icon	menu[menulim];
	value	table[tablelim];
	char	name[maxFileName], cmdpath[maxFileName], dxfpath[maxFileName], conrodDXF[maxFileName], 
			leverDXF[maxFileName], crankDXF[maxFileName], subDXF[maxFileName], bgDXF[maxFileName], key_code[256];
	
	void	initInterface( point, point );
	
	void	init();
	void	correct();
	void	turbo();
	bool	open( char[] );
	void	save( char[] );
	void	saveCmd( bool, bool = false );
	void	saveCmdV( bool = false );
	void	saveAnime( graphic& );
	void	loadDXF();
	
	// access once
	void	gui( graphic& );
	void	drawOnce( graphic&, bool, bool = false );
	void	drawHelp( graphic& );
	
	// access every object
	void	get( double, bool = false );
	void	draw( graphic&, int, int );
	void	trace();
	void	sim();
	void	gripe( graphic &g );
	
	void	sander( point raw[] );
	
	// access every polyline (that was judged)
	int	grounding( point, point, point[], point[], bool );
	int	grounding( point, point, cyclo[], cyclo[], bool );
	
  public:
	
	void	proceed( graphic &g );
	
	hecken( char[] );
	~hecken();
};


const double round=360, unround=2*M_PI;

void hecken::proceed( graphic &g )
{
	int gray;
	if( cfg.BGcolor > 128 ) gray = GetColor( 200, 200, 200 );
	else gray = GetColor( 50, 50, 50 );
	GetHitKeyStateAll( key_code );

	if( next )  theta = fmod( theta + (turn?1:-1) * round/resol, round );
	
	if( tilt - tiltold ){
		lever = lever.rota( crank, rad( tilt - tiltold ) );
		sublever = sublever.rota( crank, rad( tilt - tiltold ) );
		for( int i=0; i<point::lim; i++ ){
			const bool buf = bgraw[i].heel;
			bgraw[i] = bgraw[i].rota( crank, rad( tilt - tiltold ) );
			bgraw[i].heel = buf;
		}
	}
	tiltold = tilt;
	
	
	drawOnce( g, true );
	
	for( int i=0; i<object; i++ )
	{
		get( theta + i*round/object );
		draw( g, color[i*colorlim/(int)object], gray );
	}
	
	drawOnce( g, false );

	if( menu[mstatus].get() ) drawHelp( g );

	if( rebirth )
	{
		trace();
		if( !dxf )  sander( optimizraw );
		sim();
		if( menu[mpara].get() )  initInterface( point(6,4), point(6,8+icon::size) );
		//update = true;
		rebirth = false;
	}
	
	timeover = wait( 60000 / roll / resol );
}

void hecken::get( double deg, bool groundlock )
{
	crankjoint.x = crank.x + crankr * cos( rad(deg) );
	crankjoint.y = crank.y + crankr * sin( rad(deg) );
	
	if( doublecrank )
	{
		lever.x = subcrank.x + subcrankr * cos( rad(deg)+rad(thetaplus) );
		lever.y = subcrank.y + subcrankr * sin( rad(deg)+rad(thetaplus) );
	}
		
	
	const double dix = crankjoint.x - lever.x;
	const double diy = crankjoint.y - lever.y;
	
	const double part = hypot( dix, diy );
	
	if( !leverr || mode == mslider )
	{
		leverjoint.x = crankjoint.x - conrod * dix / part;
		leverjoint.y = crankjoint.y - conrod * diy / part;
		lock = false;
	}
	else
	{
		const double cosI = dix/part, sinI = (side?1:-1)*diy/part;
		const double cosO = ( pow(leverr,2) - pow(conrod,2) - pow(part,2) )
							/ ( -2 * conrod * part);
		const double sinO = sqrt( (1.0>pow(cosO,2) ? 1:0 ) * (1.0 - pow(cosO,2)) );
		lock = !(1.0 > pow(cosO,2));
		
		leverjoint.x = crankjoint.x - ( cosO * cosI + sinO * sinI ) * conrod;
		leverjoint.y = crankjoint.y + (side?1:-1)*( sinO * cosI - cosO * sinI ) * conrod;
	}
	
	toe = offset( crankjoint, leverjoint, shift, conrod );
	
	if( mode == mdualslider || mode == mdualhecken )
	{
		const double dsx = toe.x - sublever.x;
		const double dsy = toe.y - sublever.y;
		
		const double parts = hypot( dsx, dsy );
		
		if( !leverr || mode == mdualslider )
		{
			subleverjoint.x = toe.x - subconrod * dsx / parts;
			subleverjoint.y = toe.y - subconrod * dsy / parts;
		}
		else
		{
			const double cosI = dsx/parts, sinI = (side?1:-1)*dsy/parts;
			const double cosO = ( pow(subleverr,2) - pow(subconrod,2) - pow(parts,2) )
								/ ( -2 * subconrod * parts);
			const double sinO = sqrt( (1.0>pow(cosO,2) ? 1:0 ) * (1.0 - pow(cosO,2)) );
			lock = !(1.0 > pow(cosO,2));
			
			subleverjoint.x = toe.x - ( cosO * cosI + sinO * sinI ) * subconrod;
			subleverjoint.y = toe.y + (side?1:-1)*( sinO * cosI - cosO * sinI ) * subconrod;
		}
	}
	
	
	if( dxf ){
		if( dxfconrod ) grounding( crankjoint, leverjoint, conrodraw, conroded, groundlock ), 
						grounding( crankjoint, leverjoint, conroden, conrodened, groundlock );
		if( dxflever ) grounding( lever, leverjoint, leverraw, levered, groundlock ), 
						grounding( lever, leverjoint, leveren, leverened, groundlock );
		if( dxfcrank ) grounding( crank, crankjoint, crankraw, cranked, groundlock ), 
						grounding( crank, crankjoint, cranken, crankened, groundlock );
		if( dxfsub && !(mode == mhecken || mode == mslider) )
			grounding( toe, subleverjoint, subraw, subed, groundlock ), 
			grounding( toe, subleverjoint, suben, subened, groundlock );
	}
	else
	{
		if( mode == mdualslider ) 
			optmax = grounding( toe, subleverjoint, optimizraw, optimized, groundlock );
		else optmax = grounding( crankjoint, leverjoint, optimizraw, optimized, groundlock );
	}
	if( exist_secret_orbit ) grounding( crankjoint, leverjoint, secret_orbit, secret_orbited, groundlock );
}

int hecken::grounding( point datam, point sub, point raw[], point ed[], bool groundlock )
{
	const double dist = hypot( datam.x-sub.x, datam.y-sub.y );
	int i;
	for( i=0; i<point::lim-1; i++ )
	{
		ed[i] = offset( datam, sub, raw[i], dist );
		ed[i].heel = raw[i].heel;
		
		if( ground.y < ed[i].y && !lock && !groundlock ){
			ground = ed[i];
		}
		if( raw[i] == raw[i+1] ){
			ed[i+1] = ed[i];
			break;
		}
	}
	return i;
}
int hecken::grounding( point datam, point sub, cyclo raw[], cyclo ed[], bool groundlock )
{
	const double dist = hypot( datam.x-sub.x, datam.y-sub.y );
	int i;
	for( i=0; i<cyclo::lim-1; i++ )
	{
		ed[i].pos = offset( datam, sub, raw[i].pos, dist );
		ed[i].r = raw[i].r;
		
		if( ground.y < ed[i].pos.y && !lock && !groundlock ){
			ground = ed[i].pos;
		}
		if( !raw[i].r ){
			ed[i+1].r = 0;
			break;
		}
	}
	return i;
}

void hecken::draw( graphic &g, int color, int gray )
{
	const int focuced = GetColor(255,255,255);

	
	
	g.line( crank, crankjoint, table[tcrankr].getFocus() ? focuced :(dxfcrank?gray:color) );
	
	if( lock ) return;
	
	g.line( leverjoint, crankjoint, table[tconrod].getFocus() ? focuced :(dxfconrod?gray:color) );
	g.line( crankjoint, toe, table[tshiftx].getFocus() || table[tshifty].getFocus() ? focuced :(dxfconrod?gray:color) );
	if( mode == mdualslider ) g.line( subleverjoint, toe, dxfsub?gray:color );
	if( leverr ) g.line( lever, leverjoint, table[tleverr].getFocus() ? focuced :(dxflever?gray:color) );
	if( doublecrank ) g.line( subcrank, lever, dxfconrod?gray:color );
	
	
	if( dxf ){
		if( object == 1) color = GetColor(255,0,0);
		if( dxfconrod ) g.pline( conroded, color ), g.pcircle( conrodened, color );
		if( object == 1) color = GetColor(0,0,255);
		if( dxflever && leverr ) g.pline( levered, color ), g.pcircle( leverened, color );
		if( object == 1) color = GetColor(0,255,0);
		if( dxfcrank ) g.pline( cranked, color ), g.pcircle( crankened, color );
		if( object == 1) color = GetColor(0,255,255);
		if( dxfsub && !(mode == mhecken || mode == mslider) ) g.pline( subed, color ), g.pcircle( subened, color );
	}else
	{
		g.pline( optimized, color );

		if( menu[mpara].get() )
		{
			if( table[tmini].getFocus() ) g.line( optimized[0], crankjoint, focuced );
			if( table[tmax].getFocus() ) g.line( optimized[optmax], crankjoint, focuced );
			if( table[tstep].getFocus() ) for( int i=0; i<optmax; i+=2 ) g.line( optimized[i], optimized[i+1], focuced );
		}
	}

	
	if( exist_secret_orbit ) g.pline( secret_orbited, gray );
}


void hecken::drawOnce( graphic &g, bool first, bool anibmp )
{
	int gray;
	const int focuced = GetColor(255,255,255);
	if( cfg.BGcolor > 128 ) gray = GetColor( 200, 200, 200 );
	else gray = GetColor( 50, 50, 50 );

	if( first )
	{
		g.circle( crank, crankr, gray );
		if( shift.x || shift.y ) g.pline( orbit, gray );
		if( doublecrank ) g.circle( subcrank, subcrankr, gray );
		if( leverr ) g.circle( lever, leverr, gray );
		if( dxfbg ) g.pline( bgraw, gray ), g.pcircle( bgen, gray );
		if( exist_secret_orbit ) g.pline( secret_orbit, gray );
		if( exist_frontleg_orbit ) g.pline( frontleg_orbit, gray );
		if( exist_rearleg_orbit ) g.pline( rearleg_orbit, gray );
	}
	else
	{	
		if( object > 1 ) g.line( point(-1024,ground.y), point(1024,ground.y), gray ), g.line( ground+point(0,5), ground-point(0,5), gray );
		ground = point(0,0);
		
		if( !anibmp ) gui( g );

		
		//if( object == 1 )

		
		if( menu[mpara].get() )
		{
			if( table[tcrankx].getFocus() || table[tcranky].getFocus() ) g.circle( crank, 6/g.getScale(), focuced, true );
			if( table[tleverx].getFocus() || table[tlevery].getFocus() ) g.circle( lever, 6/g.getScale(), focuced, true );
			if( table[theight].getFocus() ) g.line( 0,0, 0,height, focuced );
			if( table[tgraph].getFocus() )
			{
				if( graph_mode == gcrankjoint ) g.circle( crankjoint, 6/g.getScale(), focuced, true );
				else if( graph_mode == gleverjoint ) g.circle( leverjoint, 6/g.getScale(), focuced, true );
				else if( graph_mode == gtoe ) g.circle( toe, 6/g.getScale(), focuced, true );
				else if( graph_mode == gsubcrankjoint ) g.circle( subcrankjoint, 6/g.getScale(), focuced, true );
				else if( graph_mode == gsubleverjoint ) g.circle( subleverjoint, 6/g.getScale(), focuced, true );
				else if( graph_mode == gsubtoe ) g.circle( subtoe, 6/g.getScale(), focuced, true );
				else if( graph_mode == gground ) g.circle( ground, 6/g.getScale(), focuced, true );
			}
		}

		if( menu[mgraph].get() )
		{
			const point datam = cfg.screen - point(450,100);
			const int color = GetColor(200,200,200), max_color = GetColor(100,200,100), min_color = GetColor(200,100,100);
			static point old;
			static int i=0;
			double acc, lever_angle = leverjoint.__dir( lever ), conrod_angle = crankjoint.__dir( leverjoint );
			static double max_lever_angle = lever_angle, min_lever_angle = lever_angle, max_conrod_angle = conrod_angle, min_conrod_angle = conrod_angle, 
				maxacc = acc, minacc = acc, minfo, minlo, line[500];

			if( graph_mode == gcrankjoint ) acc = old.dist( crankjoint )*100; 
			else if( graph_mode == gleverjoint ) acc = old.dist( leverjoint )*100;
			else if( graph_mode == gtoe ) acc = old.dist( toe )*100;
			else if( graph_mode == gground ) acc = old.dist( ground )*100;
			else if( graph_mode == gsubcrankjoint ) acc = old.dist( subcrankjoint )*100;
			else if( graph_mode == gsubleverjoint ) acc = old.dist( subleverjoint )*100;
			else if( graph_mode == gsubtoe ) acc = old.dist( subtoe )*100;
 

			if( key_code[KEY_INPUT_LSHIFT] )
			{
				//DrawLine( datam.x, datam.y+25, datam.x + angle*3, datam.y+25, color );
				int shift = -180 , dim = 65;
				DrawCircle( datam.x-shift, datam.y-dim, dim+2, color, 0 );
				DrawLine( datam.x-shift, datam.y-dim, datam.x-shift + cos(lever_angle)*dim, datam.y-dim+sin(lever_angle)*dim, color );
				DrawLine( datam.x-shift, datam.y-dim, datam.x-shift + cos(min_lever_angle)*dim, datam.y-dim+sin(min_lever_angle)*dim, min_color );
				DrawLine( datam.x-shift, datam.y-dim, datam.x-shift + cos(max_lever_angle)*dim, datam.y-dim+sin(max_lever_angle)*dim, max_color );

				shift = -330 , dim = 65;
				DrawCircle( datam.x-shift, datam.y-dim, dim+2, color, 0 );
				DrawLine( datam.x-shift, datam.y-dim, datam.x-shift + cos(conrod_angle)*dim, datam.y-dim+sin(conrod_angle)*dim, color );
				DrawLine( datam.x-shift, datam.y-dim, datam.x-shift + cos(min_conrod_angle)*dim, datam.y-dim+sin(min_conrod_angle)*dim, min_color );
				DrawLine( datam.x-shift, datam.y-dim, datam.x-shift + cos(max_conrod_angle)*dim, datam.y-dim+sin(max_conrod_angle)*dim, max_color );

				DrawLine( datam.x+400, datam.y-dim-dim-12, datam.x+400 - acc, datam.y-dim-dim-12, color ); 
			
				DrawFormatString( datam.x, datam.y+10, color, "���x = Now[%3.2f] Max[%3.2f] Min[%3.2f]", acc, maxacc, minacc );
				DrawFormatString( datam.x, datam.y+30, color, "���o�[�p�x = Band[%3.2f] Max[%3.2f] Min[%3.2f]",
					deg(max_lever_angle-min_lever_angle), deg(max_lever_angle), deg(min_lever_angle) );
				DrawFormatString( datam.x, datam.y+50, color, "�R�����b�h�p�x = Band[%3.2f] Max[%3.2f] Min[%3.2f]", 
					deg(max_conrod_angle-min_conrod_angle), deg(max_conrod_angle), deg(min_conrod_angle) );
				DrawFormatString( datam.x, datam.y+70, color, "�œK���Ȑ� �[���U�グ���� = ��[%3.2f] �O[%3.2f]", 
					height - min_reartleg_orbit , height - min_frontleg_orbit );
			}

			if( i < resol ) line[i++] = acc; else i = 0;
			for( int k=0; k<=i-2; k++ ){
				DrawLine( k*2,datam.y-200+line[k], k*2+2, datam.y-200+line[k+1], color );
			}


			if( !menu[mstop].get() ) max_lever_angle = min_lever_angle = lever_angle, max_conrod_angle = min_conrod_angle = conrod_angle, maxacc = minacc = acc, i = 0;
			if( max_lever_angle < lever_angle ) max_lever_angle = lever_angle;
			if( min_lever_angle > lever_angle ) min_lever_angle = lever_angle;
			if( max_conrod_angle < conrod_angle ) max_conrod_angle = conrod_angle;
			if( min_conrod_angle > conrod_angle ) min_conrod_angle = conrod_angle;
			if( maxacc < acc ) maxacc = acc;
			if( minacc > acc ) minacc = acc;

			if( graph_mode == gcrankjoint ) old = crankjoint; 
			else if( graph_mode == gleverjoint ) old = leverjoint;
			else if( graph_mode == gtoe ) old = toe;
			else if( graph_mode == gground ) old = ground;
			else if( graph_mode == gsubcrankjoint ) old = subcrankjoint;
			else if( graph_mode == gsubleverjoint ) old = subleverjoint;
			else if( graph_mode == gsubtoe ) old = subtoe;
		}


	}
}

void hecken::drawHelp( graphic &g )
{
	const int black = GetColor( 0, 0, 0 );
	const int gray = GetColor( 50, 50, 50 );
	const int clrhelp = GetColor( 255, 255, 255 );
	bool check=false;
	static int cntr = 0;
	static double scroll = 0;/////////////////////////////////////////////�t�H���g�T�C�Y�ɉ������X�N���[��
	
	DrawBox( 12-3, cfg.screen.y-12-cfg.fontsize-3, cfg.screen.x-12-icon::size+3, cfg.screen.y-12+3, black, TRUE );
	DrawBox( 12-4, cfg.screen.y-12-cfg.fontsize-4, cfg.screen.x-12-icon::size+4, cfg.screen.y-12+4, gray, FALSE );

	if( cntr >= 50 ) scroll += 4;
	if( menu[mopen].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�g���q��킸Links�ŕۑ������t�@�C���̂݊J���܂��B�h���b�N���h���b�v�ł��\�B", clrhelp );
	if( menu[msave].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�ۑ����ĂȂ��ꍇ�́A���O��t���ĕۑ������܂�", clrhelp );
	if( menu[msaveas].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�g���q�͕ۑ�/�Ǎ��݂ɂ͊֌W�����̂ŉ��ł��\���܂���B", clrhelp );
	if( menu[mnew].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "DXF95/2000�N�`���ŕۑ����ꂽ�t�@�C���̃|�����C���Ɖ~�̂ݓǂݍ��߂܂��@��]�̒��S�����_�Ɏw�肵�ĉ�����", clrhelp );
	if( menu[mpara].getFocus() ) check = true, DrawString( 12-scroll, cfg.screen.y-12-cfg.fontsize, "�p�����[�^�\�́C���l���z�C�[����]�Ȃǂɂ�葀�삷��\�ł��@�_�u���N���b�N�Œ��ړ��́C�z�C�[���N���b�N�ŉ�]���̑����ʂ��ω����܂�", clrhelp );

	if( menu[mstop].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "����������ۂɂ͉�]���~�����z�C�[���Ŋp�x��ς���Ɗm�F���₷���ł�", clrhelp );
	if( menu[mcmd].getFocus() ) check = true, DrawString( 12-scroll, cfg.screen.y-12-cfg.fontsize, "AutoCAD�̃R�}���h���C���ɓ\�t����ƁC���̉�ʂƓ����悤�ɍ�}����܂��@SHIFT�������Ȃ���N���b�N����Ƌr�݂̂��ACTRL�ł̓V���G�b�g����}����܂��B", clrhelp );
	if( menu[mdxfout].getFocus() ) check = true, DrawString( 12-scroll, cfg.screen.y-12-cfg.fontsize, "���̉�ʂƓ������̂�DXF�o�͂���܂��@SHIFT�������Ȃ���N���b�N����Ƌr�݂̂��ACTRL�ł̓V���G�b�g����}����܂��B", clrhelp );
	if( menu[mlnkside].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�����N�@�\�𑼂̌����ɕύX����B", clrhelp );
	if( menu[mdxf].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�œK���Ȑ��\���ƓǍ���DXF�\���i�Ǎ���łȂ��ꍇ�͉����\������܂���j��ؑւ����܂�", clrhelp );
	if( menu[mturn].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "��]������ؑւ��܂�", clrhelp );
	
	if( menu[manibmp].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�͈͂�I�����ĘA��BMP�ŏo�́iGIF�A�j���[�V���������j", clrhelp );
	if( menu[morbit].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�œK���Ȑ��̒[���O����\�����܂��iDXF���ɂ͕\������܂���j", clrhelp );
	
	if( menu[mresol].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�E�B���h�E�T�C�Y��ύX���܂��iconfig.ini�t�@�C���ōׂ����ݒ�j", clrhelp );
	if( menu[mgraph].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�\����I������C�ӂ̓_�̉����x/���x���O���t�Ƃ��ĕ\�����܂�", clrhelp );

	if( menu[mpara].get() )
	{
		if( table[tupdown].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�v�Z�ɂ�苁�߂�ꂽ�㉺���ł��@����Ɍ����ĕҏW�͂ł��܂���", clrhelp );
		if( table[tresol].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�~����ӂ�̌v�Z�񐔂ł��@�����قǐ��x�ƌv�Z���ׂ������Ȃ�܂�", clrhelp );
		if( table[ttheta].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "���݂̊�N�����N�̊p�x�ł��@�����ŕς��܂����ҏW���\�ł�", clrhelp );
		if( table[tobject].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�I�u�W�F�N�g�i�����N�j�̐��ł��@��ʂɂ���΃V���G�b�g�������܂�", clrhelp );
		if( table[tscale].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�\���{���ł��@�g�債�Ă��v�Z�̐��x�ɂ͊֌W���܂���", clrhelp );
		if( table[ttilt].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�����N�@�\���ꎞ�I�ɌX���邱�Ƃ��ł��܂��@�X�����܂܂ł͕ۑ����o���܂���", clrhelp );
		if( table[troll].getFocus() ) check = true, DrawString( 12-scroll, cfg.screen.y-12-cfg.fontsize, "������]���ł��@������肵�����ꍇ�́C��~���ĉ�]�p�x��ω�������ƕ֗��ł��@���ׂ������ꍇ�͌��E�ɂȂ�܂�", clrhelp );
		if( table[tcrankr].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�N�����N���a�ł��@�傫����������ƃ����N�@�\���������Â炭�Ȃ�܂�", clrhelp );
		if( table[tconrod].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�R�����b�h�����ł��@�X���C�_�����N�̏ꍇ�͌v�Z�ɉe�����܂���", clrhelp );
		if( table[tleverr].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "���o�[�����ł��@�O�ɂ���ƃX���C�_�����N�ɍ����ς��܂�", clrhelp );
		if( table[tleverx].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "���o�[�i�X���C�_�j�x�_��X���W�ł��@�N�����N���S�����_�ŁC�E���{�ł�", clrhelp );
		if( table[tlevery].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "���o�[�i�X���C�_�j�x�_��Y���W�ł��@�N�����N���S�����_�ŁC�����{�ł�", clrhelp );
		if( table[tshiftx].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�O����\������ׂɃN�����N���炸�炷������X�����ł�", clrhelp );
		if( table[tshifty].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�O����\������ׂɃN�����N���炸�炷������Y�����ł�", clrhelp );
		if( table[tmini].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�œK���Ȑ��̑O���̕��ł��@�傫���Ə㉺�������Ȃ��Ȃ�₷���Ȃ�܂�", clrhelp );
		if( table[tmax].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�œK���Ȑ��̌㕔�̕��ł��@�s�b�`���傫���ƌJ��オ��̊֌W�ŕω����Ȃ��ꍇ������܂�", clrhelp );
		if( table[tstep].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�œK���Ȑ��̐��x�ł��@�ȁX�����̒����Ƃ������܂�", clrhelp );
		if( table[theight].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�œK������ۂ̃N�����N���S����n�ʂ܂ł̋����ł��@�����������Ƒ����傫���Ȃ�܂�", clrhelp );
		if( table[tgraph].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�\������O���t��I�����܂��B�A�C�R������ON/OFF�ł��܂��B", clrhelp );
	}
	if( !check ) scroll = cntr = 0, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "�w���v�F�A�C�R�����N���b�N���@�\���Ăяo�����C�p�����[�^��Ńz�C�[������]�����l��ύX���ĉ�����", clrhelp );
	else cntr++;
}

void hecken::turbo()
{
	
}

void hecken::trace()
{
	int optmax = 0;
	min_frontleg_orbit = INT_MAX, min_reartleg_orbit = INT_MAX;
	while( optimized[optmax] != optimized[optmax+1] ) optmax++;
	
	for( int i=0; i<=resol; i++ ){
		get( theta + i*round/resol, true );
		orbit[i] = toe, orbit[i].heel = !lock;
		if( exist_frontleg_orbit ){
			frontleg_orbit[i] = optimized[0];
			frontleg_orbit[i].heel = !lock;
			if( min_frontleg_orbit > optimized[0].y ) min_frontleg_orbit = optimized[0].y;
		}
		if( exist_rearleg_orbit ){
			rearleg_orbit[i] = optimized[optmax];
			rearleg_orbit[optmax].heel = !lock;
			if( min_reartleg_orbit > optimized[optmax].y ) min_reartleg_orbit = optimized[optmax].y;
		}
		if( exist_secret_orbit ){
			secret_orbit[i] = sublever.rota( crankjoint, -crankjoint.dir( leverjoint )+M_PI/2 ) - crankjoint;
			secret_orbit[i].heel = !lock;
		}
	}
	orbit[(int)resol+1] = orbit[(int)resol];
	if( exist_frontleg_orbit ) frontleg_orbit[(int)resol+1] = frontleg_orbit[(int)resol];
	if( exist_rearleg_orbit ) rearleg_orbit[(int)resol+1] = rearleg_orbit[(int)resol];
	if( exist_secret_orbit ) secret_orbit[(int)resol+1] = secret_orbit[(int)resol];
}

void hecken::sim()
{
	double max=0, mini=100000;
	double len, lmax=0, lmini=100000;

	for( int i=0; i<=resol; i++ )
	{
		for( int o=0; o<object; o++ )
			get( theta + i*round/resol + o*round/object, false );
		
		//sum += ground.y;
		if( max < ground.y )  max = ground.y;
		if( mini > ground.y )  mini = ground.y;
		
		len = toe.dist( sublever );
		if( lmax < len )  lmax = len;
		if( lmini > len )  lmini = len;
		
		ground.y = 0;
	}
	updown = max-mini;
	
	if( exist_maxmin_length ){
		clsDx(); printfDx( "mini=%f, max=%f, len=%f", lmini, lmax, len ); WaitTimer( 100 );
	}
}

void hecken::gripe( graphic &g )
{
	
}

void hecken::sander( point raw[] )
{
	double len[point::lim];
	for( int i=0; i<point::lim; i++ )  len[i] = raw[i].x = raw[i].y = 10000;
	
	bool dualmode = ( mode == mdualslider || mode == mdualhecken );
	
	if( menu[mvtmode].get() )
	{
		if( shift.x >= 0 ) shift.x = -100;
		shift.y = 0;
		
		for( int i=0; i<=resol; i++ ){
			get( theta + i*round/resol, true );
			const double dx = toe.x - crankjoint.x;
			const double dy = toe.y - crankjoint.y;
			const double a = dy / dx;
			
			short leg_cnt=0;
			for( double line_cnt=mini; line_cnt<=max; line_cnt+=step ){
				const double dist = hypot( dx, dy );
				const double xw = toe.x - line_cnt * dy / dist;
				const double yw = toe.y + line_cnt * dx / dist;
				
				const double b = yw - xw * a;
				
				const double xv = ( crank.y + height - b ) / a;
				const double yv = crank.y + height;
				
				const double g = hypot( xw-xv, yw-yv ) - dist; 
				
				if( raw[leg_cnt].x > g )  raw[leg_cnt].x = g;
				raw[leg_cnt].y = -line_cnt;
				raw[leg_cnt].heel = true;
				leg_cnt++;
			}
			raw[leg_cnt] = raw[leg_cnt-1];
		}
	}
	else for( int i=0; i<=resol; i++ )
	{
		get( theta + i*round/resol, true );
		
		point temp_leverjoint = dualmode ? subleverjoint : leverjoint, 
			temp_crankjoint = dualmode ? toe : crankjoint;
		double temp_conrod = dualmode ? subconrod : conrod;
		
		double slope = temp_leverjoint.dir( temp_crankjoint );
		
		int leg_cnt=0;
		for( double line_ang=slope+M_PI*0.5+mini*M_PI/180; line_ang<=slope+M_PI*1.5-max*M_PI/180; line_ang+=step*M_PI/180 )
		{
			double length = (crank.x+height-temp_leverjoint.y) / sin(line_ang); 
			
			if( length < len[leg_cnt] && length > 0 )
			{
				len[leg_cnt] = length;
				raw[leg_cnt] = sincos(line_ang-slope-M_PI/2)*length;
				raw[leg_cnt].y -= temp_conrod;
			}
			raw[leg_cnt].heel = true;
			leg_cnt++;
		}
		raw[leg_cnt] = raw[leg_cnt-1];
	
	}
	
	
	/*
	else for( int i=0; i<=resol; i++ )
	{
		get( theta + i*round/resol, true );
		
		point temp_leverjoint = dualmode ? subleverjoint : leverjoint, 
			temp_crankjoint = dualmode ? toe : crankjoint;
		double temp_conrod = dualmode ? subconrod : conrod;
		
		point delta = temp_leverjoint - temp_crankjoint;
		double slope = delta.y / delta.x;
		
		int leg_cnt=0;
		for( double line_cnt=mini; line_cnt<=max; line_cnt+=step )
		{
			point first( temp_leverjoint.x - line_cnt * delta.y / temp_conrod,
				temp_leverjoint.y + line_cnt * delta.x / temp_conrod );
			
			double intercept = first.y - first.x * slope;
			
			point second( (crank.y + height - intercept) / slope, crank.y + height );
			
			double length = first.dist( second ) - temp_conrod; 
			
			if( raw[leg_cnt].y > length )  raw[leg_cnt].y = length;
			raw[leg_cnt].x = line_cnt;
			raw[leg_cnt].heel = true;
			leg_cnt++;
		}
		raw[leg_cnt] = raw[leg_cnt-1];
	}
	else for( int i=0; i<=resol; i++ )
	{
		get( theta + i*round/resol, true );
		point delta = dualmode ? subleverjoint - toe : leverjoint - crankjoint;
		double slope = delta.y / delta.x;
		
		int leg_cnt=0;
		for( double line_cnt=mini; line_cnt<=max; line_cnt+=step )
		{
			point first( (dualmode?subleverjoint.x:leverjoint.x) - line_cnt * delta.y / (dualmode?subconrod:conrod),
				(dualmode?subleverjoint.y:leverjoint.y) + line_cnt * delta.x / (dualmode?subconrod:conrod) );
			
			double intercept = first.y - first.x * slope;
			
			point second( (crank.y + height - intercept) / slope, crank.y + height );
			
			double length = first.dist( second ) - (dualmode?subconrod:conrod); 
			
			if( raw[leg_cnt].y > length )  raw[leg_cnt].y = length;
			raw[leg_cnt].x = line_cnt;
			raw[leg_cnt].heel = true;
			leg_cnt++;
		}
		raw[leg_cnt] = raw[leg_cnt-1];
	}*/
	/*else for( int i=0; i<=resol; i++ )
	{
		get( theta + i*round/resol, true );
		const double dx = (dualmode?subleverjoint.x:leverjoint.x) - (dualmode?toe.x:crankjoint.x);
		const double dy = (dualmode?subleverjoint.y:leverjoint.y) - (dualmode?toe.y:crankjoint.y);
		const double a = dy / dx;
		
		int leg_cnt=0;
		for( double line_cnt=mini; line_cnt<=max; line_cnt+=step ){
			const double xw = (dualmode?subleverjoint.x:leverjoint.x) - line_cnt * dy / (dualmode?subconrod:conrod);
			const double yw = (dualmode?subleverjoint.y:leverjoint.y) + line_cnt * dx / (dualmode?subconrod:conrod);
			
			const double b = yw - xw * a;
			
			const double xv = ( crank.y + height - b ) / a;
			const double yv = crank.y + height;
			
			const double g = hypot( xw-xv, yw-yv ) - (dualmode?subconrod:conrod); 
			
			if( raw[leg_cnt].y > g )  raw[leg_cnt].y = g;
			raw[leg_cnt].x = line_cnt;
			raw[leg_cnt].heel = true;
			leg_cnt++;
		}
		raw[leg_cnt] = raw[leg_cnt-1];
	}*/
}


void hecken::initInterface( point m, point t )
{
	int i=0;
	int span=6;
	menu[mopen].init	( m.x+i*(icon::size+span), m.y, IDB_OPEN, "�t�@�C�����J��"  );i++;
	menu[msave].init	( m.x+i*(icon::size+span), m.y, IDB_SAVE, "�㏑���ۑ�" );i++;
	menu[msaveas].init	( m.x+i*(icon::size+span), m.y, IDB_SAVEAS, "���O��t���ĕۑ�" );i++;
	menu[mnew].init		( m.x+i*(icon::size+span), m.y, IDB_NEW, "DXF�t�@�C����ǂݍ���" );i++;
	//menu[mconfig].init	( m.x+i*(icon::size+span), m.y, IDB_PROPERTY, "�ݒ�ύX" );i++;
	menu[mpara].init	( m.x+i*(icon::size+span), m.y, IDB_MENUON,"�p�����[�^�\��\��", IDB_CLOSE, "�p�����[�^�\���B��" );i++;
	
	int space=10;
	menu[mstop].init	( m.x+i*(icon::size+span)+space, m.y, IDB_START, "��]�J�n", IDB_STOP, "��]��~" );i++;
	menu[mcmd].init	( m.x+i*(icon::size+span)+space, m.y, IDB_PASTE, "AutoCAD�R�}���h���N���b�v�{�[�h�ɓ\�t���iSHIFT�ACTRL�ŕω��j" );i++;
	menu[mdxfout].init	( m.x+i*(icon::size+span)+space, m.y, IDB_PASTEDXF, "DXF�t�@�C�����o�́iSHIFT�ACTRL�ŃI�v�V�����o�́j" );i++;
	menu[mlnkside].init	( m.x+i*(icon::size+span)+space, m.y, IDB_LSIDE, "�����N�T�C�h�ύX" );i++;
	menu[mdxf].init	( m.x+i*(icon::size+span)+space, m.y, IDB_DXF, "DXF�\��", IDB_OPT, "�œK���Ȑ��\��" );i++;
	menu[mturn].init	( m.x+i*(icon::size+span)+space, m.y, IDB_LTURN, "���v���", IDB_RTURN, "�����v���" );i++;

	space=20;
	//menu[mrelord].init	( m.x+i*(icon::size+span)+space, m.y, IDB_RELOAD, "�ŐV�̏�ԂɍX�V" );i++;
	//menu[mturbo].init	( m.x+i*(icon::size+span)+space, m.y, IDB_TURBO, "�����`�惂�[�h��", IDB_START, "�ʏ�`�惂�[�h��" );i++;
	menu[manibmp].init	( m.x+i*(icon::size+span)+space, m.y, IDB_ANIME, "�A�j���[�V������A�ԃr�b�g�}�b�v�ŕۑ�" );i++;
	menu[morbit].init	( m.x+i*(icon::size+span)+space, m.y, IDB_ORBIT, "�[���O����\��" );i++;
	menu[mresol].init	( m.x+i*(icon::size+span)+space, m.y, IDB_RESOL, "�E�B���h�E�T�C�Y�ύX" );i++;
	menu[mgraph].init	( m.x+i*(icon::size+span)+space, m.y, IDB_GRAPH, "�O���t��\��", IDB_CLOSE, "�O���t���B��" );i++;
	
	
	span=6;
	menu[mstatus].init	( cfg.screen.x-(icon::size+span), cfg.screen.y-(icon::size+span), 
		IDB_HELPON, "�X�e�[�^�X�o�[��\��", IDB_CLOSE, "�X�e�[�^�X�o�[���B��" );
	
	i=0;
	
	//if( key_code[KEY_INPUT_LSHIFT] && key_code[KEY_INPUT_B] && key_code[KEY_INPUT_D] )

	if( limiter )
	{
		menu[marcmode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "�A�[�N�X���C�_���[�h��", "", "�ʏ탂�[�h��" );
		menu[mdsmode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "�f���A���X���C�_���[�h��", "", "�ʏ탂�[�h��" );
		menu[mvtmode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "���@�[�e�B�J�����[�h��(�s�t)", "", "�ʏ탂�[�h��" );
		menu[mdcmode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "�_�u���N�����N���[�h��", "", "�ʏ탂�[�h��" );
		menu[mmmlmode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "�����\�����[�h��", "", "�ʏ탂�[�h��" );

		menu[mfomode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "�r�O���O���\��" );
		menu[mromode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "�r�㕔�O���\��" );
		menu[msomode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "�B���O���\��" );
	}
	
	i=0;
	span=4;
	table[tupdown].init	( t.x, t.y+i*(cfg.fontsize+span), &updown, "�㉺��" );i++;
	table[tupdown].setLock();
	table[tresol].init	( t.x, t.y+i*(cfg.fontsize+span), &resol, "���~�x" );i++;
	table[ttheta].init	( t.x, t.y+i*(cfg.fontsize+span), &theta, "��]�p�x" );i++;
	table[tobject].init	( t.x, t.y+i*(cfg.fontsize+span), &object, "�I�u�W�F�N�g��" );i++;
	table[tscale].init	( t.x, t.y+i*(cfg.fontsize+span), &scale, "�\���{��" );i++;
	table[ttilt].init	( t.x, t.y+i*(cfg.fontsize+span), &tilt, "�X��" );i++;
	table[troll].init	( t.x, t.y+i*(cfg.fontsize+span), &roll, timeover?"������]��(���E)":"������]��" );i++;
	table[tcrankr].init	( t.x, t.y+i*(cfg.fontsize+span), &crankr, "�N�����N���a" );i++;
	table[tconrod].init	( t.x, t.y+i*(cfg.fontsize+span), &conrod, "�R�����b�h����" );i++;
	table[tleverr].init	( t.x, t.y+i*(cfg.fontsize+span), &leverr, leverr?"���o�[����":"�X���C�_(���o�[����)" );i++;
	if( !doublecrank )
	{
		table[tleverx].init	( t.x, t.y+i*(cfg.fontsize+span), &lever.x, leverr?"���o�[�w���W":"�X���C�_�w���W" );i++;
		table[tlevery].init	( t.x, t.y+i*(cfg.fontsize+span), &lever.y, leverr?"���o�[�x���W":"�X���C�_�x���W" );i++;
	}else{
		table[tleverx].mun();
		table[tlevery].mun();
	}
	table[tshiftx].init	( t.x, t.y+i*(cfg.fontsize+span), &shift.x, "�V�t�g�w����" );i++;
	table[tshifty].init	( t.x, t.y+i*(cfg.fontsize+span), &shift.y, "�V�t�g�x����" );i++;
	if( !dxf ){ 
		table[tmini].init	( t.x, t.y+i*(cfg.fontsize+span), &mini, "�œK���Ȑ��O��" );i++;
		table[tmax].init	( t.x, t.y+i*(cfg.fontsize+span), &max, "�œK���Ȑ��㕝" );i++;
		table[tstep].init	( t.x, t.y+i*(cfg.fontsize+span), &step, "�œK���Ȑ��s�b�`" );i++;
		table[theight].init	( t.x, t.y+i*(cfg.fontsize+span), &height, "�œK���Ȑ�����" );i++;
	}else{ 
		table[tmini].mun(); table[tmax].mun(); table[tstep].mun(); table[theight].mun(); 
	}
	if( mode == mdualslider || mode == mdualhecken ){ 
		table[tsubconrod].init	( t.x, t.y+i*(cfg.fontsize+span), &subconrod, "�T�u�R�����b�h����" );i++;
		table[tsubleverx].init	( t.x, t.y+i*(cfg.fontsize+span), &sublever.x, "�T�u���o�[�w���W" );i++;
		table[tsublevery].init	( t.x, t.y+i*(cfg.fontsize+span), &sublever.y, "�T�u���o�[�x���W" );i++;
	}else{ 
		table[tsubconrod].mun(); table[tsubleverx].mun(); table[tsublevery].mun(); 
	}
	if( doublecrank )
	{
		table[tsubcrankx].init	( t.x, t.y+i*(cfg.fontsize+span), &subcrank.x, "�T�u�N�����N�w���W" );i++;
		table[tsubcranky].init	( t.x, t.y+i*(cfg.fontsize+span), &subcrank.y, "�T�u�N�����N�x���W" );i++;
		table[tsubcrankr].init	( t.x, t.y+i*(cfg.fontsize+span), &subcrankr, "�T�u�N�����N���a" );i++;
		table[tthetaplus].init	( t.x, t.y+i*(cfg.fontsize+span), &thetaplus, "�T�u�N�����N�ʑ�" );i++;
		table[tthetaplus].setMagni( 5 );
	}else{
		table[tsubcrankx].mun();
		table[tsubcranky].mun();
		table[tsubcrankr].mun();
		table[tthetaplus].mun();
	}
	if( exist_graph ) table[tgraph].init( t.x, t.y+i*(cfg.fontsize+span), &graph_mode, "�O���t�`��p�����[�^" ), i++;
	else table[tgraph].mun();
		
	
	//table[tdxf].init	( t.x, t.y+i*(cfg.fontsize+span), &dxf, "�œK���Ȑ���\��", "�c�w�e��\��" );i++;
	//table[tturn].init	( t.x, t.y+i*(cfg.fontsize+span), &turn, "�����v���ɂ���", "���v���ɂ���" );i++;
	
	table[ttheta].setMagni( 5 );
	table[tscale].setMagni( 0.2 );
	table[tmini].setMagni( 2 );
	table[tmax].setMagni( 2 );
}

void hecken::gui( graphic &g )
{
	const int gray = GetColor( 50, 50, 50 ), keyleft = key_code[KEY_INPUT_LEFT] || key_code[KEY_INPUT_RIGHT], 
		keyup = key_code[KEY_INPUT_UP], keydown = key_code[KEY_INPUT_DOWN];
	static int oldkey = keyleft || keyup || keydown;
	int vrw;

	if( !g.w )
		if( !oldkey && keyup ) vrw = 1;
		else if( !oldkey && keydown ) vrw = -1;
		else vrw = 0;
	else vrw = g.w; 

	if( menu[mpara].get() ) 
		for( int i=0; i<tablelim; i++ )
		{
			rebirth = table[i].set( g.real, g.LL, g.C || (!oldkey&&keyleft), vrw )? (table[i].getActive()?true:rebirth) : rebirth;
		}
	for( int i=0; i<menulim; i++ ) menu[i].set( g.real, g.L );
	oldkey = keyleft || keyup || keydown;
	// rebirth���́A�����[�h�A�C�R����\���E���m���Ȃ��B
	
	//pushback switch
	if( menu[mopen].get() ){
		
 		if( helpmode ) messageOK( "�V�����t�@�C���ƊJ���ƌ��݂̐��l�͔j������܂��B" );
		if( openFileDialog( name, "Links�`���t�@�C��(*.links)\0*.links\0�S�Ẵt�@�C��(*.*)\0*.*\0\0" ) ) open( name );
		rebirth = true;
	}
	if( menu[msaveas].get() ){
		if( !tilt || 1 ){   // DEBUG TODO 
			char buf[maxFileName];
 			saveFileDialog( name, "Links�`���t�@�C��(*.links)\0*.links\0�S�Ẵt�@�C��(*.*)\0*.*\0\0" );  save( name );
			sprintf( buf, "%s  [ %s ] ", cfg.title, name );
			setWindowText( buf );
		}else  messageOK( "�\���󂠂�܂��񂪁A�X����0�ȊO�̎��͕ۑ��ł��܂���B\n������P�\��ł��B" );
	}
	if( menu[msave].get() ){
		if( !tilt || 1 ){   // DEBUG TODO 
 			save( name );
		}else  messageOK( "�\���󂠂�܂��񂪁A�X����0�ȊO�̎��͕ۑ��ł��܂���B\n������P�\��ł��B" );
	}
	if( menu[mnew].get() )  loadDXF();
	
	if( menu[mconfig].get() ) messageOK( "�\���󂠂�܂���B�ݒ�͂܂������p�ɂȂ�܂���B" );
	if( menu[mcmd].get() )
	{
		if( key_code[KEY_INPUT_LSHIFT] || key_code[KEY_INPUT_RSHIFT] ) saveCmdV();
		else if( key_code[KEY_INPUT_LCONTROL] || key_code[KEY_INPUT_RCONTROL] ) saveCmd( true );
		else saveCmd( false );
	}
	if( menu[mdxfout].get() )
	{
		if( key_code[KEY_INPUT_LSHIFT] || key_code[KEY_INPUT_RSHIFT] ) saveCmdV( true );
		else if( key_code[KEY_INPUT_LCONTROL] || key_code[KEY_INPUT_RCONTROL] ) saveCmd( true, true );
		else saveCmd( false, true );
	}
	
	if( menu[mturbo].get() ) turbo(), turbomode = !turbomode;
	if( menu[mlnkside].get() ){ side = !side; rebirth = true; }

	if( menu[manibmp].get() )  saveAnime( g );

	if( dxf != menu[mdxf].get() ) rebirth = true;
	if( exist_graph != menu[mgraph].get() ) rebirth = true;

	//switch
	dxf = menu[mdxf].get();
	turn = menu[mturn].get();
	helpmode = menu[mstatus].get();
	doublecrank = menu[mdcmode].get();
	arcmode = menu[marcmode].get();
	exist_maxmin_length = menu[mmmlmode].get();
	exist_graph = menu[mgraph].get();
	if( menu[morbit].get() ){ exist_frontleg_orbit = !exist_frontleg_orbit; exist_rearleg_orbit = !exist_rearleg_orbit; rebirth = true; }
	
	if( menu[mfomode].get() ){ exist_frontleg_orbit = !exist_frontleg_orbit; rebirth = true; }
	if( menu[mromode].get() ){ exist_rearleg_orbit = !exist_rearleg_orbit; rebirth = true; }
	if( menu[msomode].get() ){ exist_secret_orbit = !exist_secret_orbit; rebirth = true; }
	
	if( menu[mresol].get() )
	{
		const short resx[] = {800, 1024, 1280},  resy[] = {600, 768, 768};
		static char i=0;
		if( ++i==3 ) i=0;
		cfg.screen = point(resx[i], resy[i]); 
		SetGraphMode( cfg.screen.x, cfg.screen.y, 16 ); 
		SetDrawScreen( DX_SCREEN_BACK );
		rebirth = true;
	}

	next = menu[mstop].get();
	mode = menu[mdsmode].get() ? mdualslider : mhecken;
//	if( (mode == mhecken || mode == mslider)&& menu[mdsmode].get() ) rebirth = true;
//	if( (mode == mdualhecken || mode == mdualslider)&& !menu[mdsmode].get() ) rebirth = true;
	
	correct();
	g.setScale( scale );
	
	char buf[maxFileName];
	if( !GetDragFilePath( buf ) )
	{
		char msg[maxFileName];
		sprintf( msg, "[ %s ] ��Ǎ��݂܂�����낵���ł����H", buf );
		if( messageYN( msg, "D&D�m�F" ) )
		{
			open( buf );
			DragFileInfoClear();
			rebirth = true;
		}
	}
}

void hecken::correct()
{
	if( resol < 20 ) resol = 20;
	if( resol > 1000 ) resol = 1000;
	if( roll < 10 ) roll = 10;
	if( object < 1 ) object = 1;
	if( object >= colorlim ) object = colorlim;
	if( step <= 0 ) step = 1;
	//if( fmod( xxx, 1 ) ) xxx = (int)(xxx + 0.4);
	if( !conrod ) conrod = 1;
	if( !subconrod ) subconrod = 1;
	if( !fmod( thetaplus, 360 ) ) thetaplus+=0.0001;
	if( scale < 1 ) scale = 1;
	if( graph_mode < 0 ) graph_mode = 0;
	else if( graph_mode > glim ) graph_mode = glim;

}

void hecken::init()
{
	crank.x = 0;  crank.y = 0;  crankr = 25;
	lever.x = -50+crank.x;  lever.y = -50+crank.y;  leverr = 50;
	conrod = 50;  theta = 0;
	shift.x = 0;  shift.y = 50;
	scale = 3; tilt = 0;
	object = 3;  resol = 100;  roll = 15;
	mini = 42; max = 42; step = 5; height = 100;
	side = false; turn = true; dxf = true; rebirth = true;
	dxfcrank = false; dxflever = false; dxfconrod = false; dxfsub = false;
	doublecrank = false;
	exist_graph = false;
	arcmode = false;
	mode = mslider;
	tiltold = 0;
	helpmode = cfg.first;
	
	exist_frontleg_orbit = exist_rearleg_orbit = exist_secret_orbit = exist_maxmin_length = false;
	min_frontleg_orbit = INT_MAX, min_reartleg_orbit = INT_MAX;

	dxfconrod = false;
	
	conrodDXF[0] = '\0';
	leverDXF[0] = '\0';
	crankDXF[0] = '\0';
	subDXF[0] = '\0';
	bgDXF[0] = '\0';
	
	sublever.x = 0;
	sublever.y = 20;

	menu[mturn].active();
}

void hecken::save( char filename[] )
{
	std::ofstream fout( filename );
	
	fout << resol << "\t���~�x\n";
	fout << theta << "\t��]�p�x\n";
	fout << object << "\t�I�u�W�F�N�g��\n";
	fout << scale << "\t�\���{��\n";
	fout << tilt << "\t�X��\n";
	fout << roll << "\t������]��\n";
	
	fout << dxf << "\t�c�w�e�t�@�C���\��\n";
	fout << side << "\t�����N�T�C�h\n";
	
	fout << crankr << "\t�N�����N���a\n";
	fout << conrod << "\t�R�����b�h����\n";
	fout << leverr << "\t���o�[����\n";
	fout << lever.x << "\t���o�[�w���W\n";	
	fout << lever.y << "\t���o�[�x���W\n";
	fout << shift.x << "\t�V�t�g�w����\n";
	fout << shift.y << "\t�V�t�g�x����\n";
	
	fout << height << "\t�œK���Ȑ�����\n";
	fout << mini << "\t�œK���Ȑ��O��\n";
	fout << max << "\t�œK���Ȑ��㕝\n";
	fout << step << "\t�œK���Ȑ��s�b�`\n";
	
	fout << subcrankr << "\t�T�u�N�����N���a\n";
	fout << subcrank.x << "\t�T�u�N�����N�w���W\n";
	fout << subcrank.y << "\t�T�u�N�����N�x���W\n";
	fout << subconrod << "\t�T�u�R�����b�h����\n";
	fout << subleverr << "\t�T�u���o�[����\n";
	fout << sublever.x << "\t�T�u���o�[�w���W\n";	
	fout << sublever.y << "\t�T�u���o�[�x���W\n";
	fout << subshift.x << "\t�T�u�V�t�g�w����\n";
	fout << subshift.y << "\t�T�u�V�t�g�x����\n";
	
	fout << conrodDXF << "\t�R�����b�h�p�c�w�e�t�@�C��\n";
	fout << leverDXF << "\t���o�[�p�c�w�e�t�@�C��\n";
	fout << crankDXF << "\t�N�����N�p�c�w�e�t�@�C��\n";
	fout << subDXF << "\t�T�u�p�c�w�e�t�@�C��\n";
	fout << bgDXF << "\t�o�b�N�O���E���h�p�c�w�e�t�@�C��\n";
	
	
	fout << "eof";
	
	//strcpy( cfg.name, name );
	strcpy( cfg.name, filename );
}


bool hecken::open( char filename[] )
{
	if( filename[0] == '"' ){ strcpy( name, filename + 1 ); name[strlen(name)-1] = '\0'; } // ""�͂���
	else strcpy( name, filename );
	
	char buf[maxFileName];
	std::ifstream fin( name );
	fin.getline( buf, maxFileName ); resol = atof( buf );
	fin.getline( buf, maxFileName ); theta = atof( buf );
	fin.getline( buf, maxFileName ); object = atof( buf );
	fin.getline( buf, maxFileName ); scale = atof( buf );
	fin.getline( buf, maxFileName ); tilt = atof( buf );
	fin.getline( buf, maxFileName ); roll = atof( buf );
	
	fin.getline( buf, maxFileName ); dxf = atoi( buf );
	fin.getline( buf, maxFileName ); side = atoi( buf );
	
	fin.getline( buf, maxFileName ); crankr = atof( buf );
	fin.getline( buf, maxFileName ); conrod = atof( buf );
	fin.getline( buf, maxFileName ); leverr = atof( buf );
	fin.getline( buf, maxFileName ); lever.x = atof( buf );
	fin.getline( buf, maxFileName ); lever.y = atof( buf );
	fin.getline( buf, maxFileName ); shift.x = atof( buf );
	fin.getline( buf, maxFileName ); shift.y = atof( buf );
	
	fin.getline( buf, maxFileName ); height = atof( buf );
	fin.getline( buf, maxFileName ); mini = atof( buf );
	fin.getline( buf, maxFileName ); max = atof( buf );
	fin.getline( buf, maxFileName ); step = atof( buf );
	
	fin.getline( buf, maxFileName ); subcrankr = atof( buf );
	fin.getline( buf, maxFileName ); subcrank.x = atof( buf );
	fin.getline( buf, maxFileName ); subcrank.y = atof( buf );
	fin.getline( buf, maxFileName ); subconrod = atof( buf );
	fin.getline( buf, maxFileName ); subleverr = atof( buf );
	fin.getline( buf, maxFileName ); sublever.x = atof( buf );
	fin.getline( buf, maxFileName ); sublever.y = atof( buf );
	fin.getline( buf, maxFileName ); subshift.x = atof( buf );
	fin.getline( buf, maxFileName ); subshift.y = atof( buf );
	
	fin.getline( conrodDXF, maxFileName, '\t' ); fin.ignore( maxFileName, '\n' );
	fin.getline( leverDXF, maxFileName, '\t' ); fin.ignore( maxFileName, '\n' );
	fin.getline( crankDXF, maxFileName, '\t' ); fin.ignore( maxFileName, '\n' );
	fin.getline( subDXF, maxFileName, '\t' ); fin.ignore( maxFileName, '\n' );
	fin.getline( bgDXF, maxFileName, '\t' ); fin.ignore( maxFileName, '\n' );
	
	
	{
		dxff dxfx[5];
		
		dxfconrod = dxfx[0].read( conrodDXF, conrodraw, conroden );
		dxflever = dxfx[1].read( leverDXF, leverraw, leveren );
		dxfcrank = dxfx[2].read( crankDXF, crankraw, cranken );
		dxfsub = dxfx[3].read( subDXF, subraw, suben );
		dxfbg = dxfx[4].read( bgDXF, bgraw, bgen );
	}
	
	/*
	dxfconrod = readDXF( conrodDXF, conrodraw );
	dxflever = readDXF( leverDXF, leverraw );
	dxfcrank = readDXF( crankDXF, crankraw );
	dxfsub = readDXF( subDXF, subraw );
	dxfbg = readDXF( bgDXF, bgraw );
	*/
	tiltold = 0;

	fin.getline( buf, maxFileName );
	if( strcmp( buf, "eof" ) ){
		if( menu[mopen].get() ) messageOK( "�t�@�C�������G���[\nLinks�ŕۑ������t�@�C�����m�F���ĉ������D" );
		else messageOK( "�t�@�C�����J���܂���ł����B" );
		init(); name[0] = '\0';
	}
	
	if( name[0] != '\0' ){
		sprintf( buf, "%s  [ %s ] ", cfg.title, name );
		setWindowText( buf );
		return false;
	}else{
		setWindowText( cfg.title );
		return true;
	}
	
	rebirth = true;
}

void hecken::saveCmd( bool maxObject, bool DXF )
{
	std::ofstream fout( DXF ? dxfpath : cmdpath );
	if( fout.bad() || fout.fail() ) messageOK( "�G���[���������܂����B�t�@�C���������s�B\n�t�@�C�����g�p���łȂ����m�F���ĉ������B" );

	if( DXF )  fout << " 0\nSECTION\n 2\nENTITIES\n";
	
	if( maxObject )
	{
		int maxNum = resol/5;		// �b��Ώ�
		for( int i=0; i<maxNum; i++ )		
		{
			get( theta + i*round/maxNum, false );
			if( dxfconrod ) fpline( fout, conroded, DXF );
			if( dxflever && leverr ) fpline( fout, levered, DXF );
			if( dxfcrank ) fpline( fout, cranked, DXF );
			if( dxfsub && !(mode == mhecken || mode == mslider) ) fpline( fout, subed, DXF );
		}
		if( !DXF )  fout << "_region\nall\n\n_union\nall\n\n\n\n";
	}
	else
	{
		for( int i=0; i<object; i++ )
		{
			get( theta + i*round/object, false );
			
			if( !dxf || (!dxfconrod && !dxflever && !dxfcrank && !dxfsub) ){
				fpline( fout, optimized, DXF );
				fline( fout, crank, crankjoint, DXF );
				fline( fout, leverjoint, crankjoint, DXF );
				fline( fout, crankjoint, toe, DXF );
				if( mode == mdualslider ) fline( fout, subleverjoint, toe, DXF );
				if( leverr ) fline( fout, lever, leverjoint, DXF );
			}else{
				if( dxfconrod ) fpline( fout, conroded, DXF );
				if( dxflever && leverr ) fpline( fout, levered, DXF );
				if( dxfcrank ) fpline( fout, cranked, DXF );
				if( dxfsub && !(mode == mhecken || mode == mslider) ) fpline( fout, subed, DXF );
			}
			if( exist_secret_orbit ) fpline( fout, secret_orbited, DXF );
		}

		fcircle( fout, crank, crankr, DXF );
		if( shift.x || shift.y ) fpline( fout, orbit, DXF );
		if( leverr ) fcircle( fout, lever, leverr, DXF );
		else fcircle( fout, lever, 1, DXF );
		//else fpoint( fout, lever, DXF ); TODO:fpoint
		//if( subleverr ) fcircle( fout, sublever, subleverr, DXF );
		//else fpoint( fout, sublever, DXF );                                              -------------------relation subsysytem------------------
		if( dxfbg ) fpline( fout, bgraw, DXF );
		
		//fline( fout, point(-1024,ground.y), point(1024,ground.y), DXF );
		if( shift.x || shift.y ) fline( fout, ground+point(0,5), ground-point(0,5), DXF );
	}
	if( DXF )  fout << " 0\nENDSEC\n 0\nEOF\n";
	fout.close();
	
	if( !DXF )
	{
		std::ifstream fin( cmdpath, std::ios::binary );

		std::string str;
		while( fin.good() ) str += fin.get();
		str.erase( str.size()-1,1 );
		clipboard( str.c_str() );
	}
}

void hecken::saveCmdV( bool DXF)
{
	std::ofstream fout( cmdpath );
	
	fpline( fout, optimizraw );
	fpoint( fout, lever-crank );
	
	fout.close();
	
	std::ifstream fin( cmdpath, std::ios::binary );

	std::string str;
	while( fin.good() ) str += fin.get();
	str.erase( str.size()-1,1 );
	clipboard( str.c_str() );
}

void hecken::saveAnime( graphic &g )
{
		const int white = GetColor( 255,255,255 ), black = GetColor( 0,0,0 ), gray = GetColor( 50, 50, 50 );;
		
		DrawFormatString( 150,25,white, "�ۑ��͈͂�2��̃N���b�N�Ŏ����Ă��������B�L�����Z���̓G�X�P�[�v�L�[�ł��B" );
		ScreenFlip();
		while( g.l ) g.getmouse();
		
		for(;;){ g.getmouse(); if( ProcessMessage() || key_code[KEY_INPUT_ESCAPE] || g.L ) break; }
		if( key_code[KEY_INPUT_ESCAPE] ) goto end;
		point p1 = g.real, p1v = g.vir;
		for(;;)
		{
			g.getmouse();
			point p = g.vir;
			g.rect( p1v, p-p1v, white );
			ScreenFlip(); wait( 20 );
			g.rect( p1v, p-p1v, black );
			
			if( ProcessMessage() || key_code[KEY_INPUT_ESCAPE] || g.L ) break;
		}
		if( key_code[KEY_INPUT_ESCAPE] ) goto end;
		
		point p2 = g.real;
		char str[maxFileName], path[maxFileName];

		strcpy( path, cfg.path );
		strcat( path, "BmpAnime" );
		CreateDirectory( path, NULL );
		
		for( int i=0; i<=resol; i++ )
		{
			ClsDrawScreen();
			drawOnce( g, true, true );
			
			for( int o=0; o<object; o++ )
			{
				get( theta + o*round/object + i*round/resol );
				draw( g, color[o*colorlim/(int)object], gray );
			}
			
			drawOnce( g, false, true );
			ScreenFlip();
			
			sprintf( str, "%s%s%03d%s", path,"\\", i, ".bmp" );
			saveDrawScreen( p1, p2, str );
		}
end:;
}

void hecken::loadDXF()
{
	
		char filter[] = "DXF(*.dxf)\0*.dxf\0\0";
		
 		if( helpmode ) messageOK( "�e��c�w�e�t�@�C����ǂݍ��݂܂��B\n"
			"�������ŋ�̃t�@�C����ǂݍ��݁A�L�����Z���Ō��݂̂܂܂ɂȂ�܂��B" );
		
		int buf;
		if( (buf = messageYNC( "�R�����b�h�p�c�w�e�t�@�C����ǂݍ��݂܂����H" )) == IDYES ){
			dxff dxfx;
			openFileDialog( conrodDXF, filter );
			dxfconrod = dxfx.read( conrodDXF, conrodraw, conroden );
		}else if( buf == IDNO ){ conrodDXF[0] = '\0'; dxfconrod = false; }

		if( (buf = messageYNC( 
			(arcmode?"�A�[�N�X���C�_�O���pDXF�t�@�C����Ǎ��݂܂����H":"���o�[�p�c�w�e�t�@�C����ǂݍ��݂܂����H")
			)) == IDYES  ){
			dxff dxfx;
			openFileDialog( leverDXF, filter );
			dxflever = dxfx.read( leverDXF, leverraw, leveren );
		}else if( buf == IDNO ){ leverDXF[0] = '\0'; dxflever = false; }

		if( (buf = messageYNC( "�N�����N�p�c�w�e�t�@�C����ǂݍ��݂܂����H" )) == IDYES  ){
			dxff dxfx;
			openFileDialog( crankDXF, filter );
			dxfcrank = dxfx.read( crankDXF, crankraw, cranken );
		}else if( buf == IDNO ){ crankDXF[0] = '\0'; dxfcrank = false; }

		if( (mode == mdualhecken || mode == mdualslider) && 
			((buf = messageYNC( "�T�u�p�c�w�e�t�@�C����ǂݍ��݂܂����H" )) == IDYES ) ){
			dxff dxfx;
			openFileDialog( subDXF, filter );
			dxfsub = dxfx.read( subDXF, subraw, suben );
		}else if( buf == IDNO ){ subDXF[0] = '\0'; dxfsub = false; }

		if( (buf = messageYNC( "�o�b�N�O���E���h�p�c�w�e�t�@�C����ǂݍ��݂܂����H" )) == IDYES  ){
			dxff dxfx;
			openFileDialog( bgDXF, filter );
			dxfbg = dxfx.read( bgDXF, bgraw, bgen );
		}else if( buf == IDNO ){ bgDXF[0] = '\0'; dxfbg = false; }
}

hecken::hecken( char filename[] = "links.txt" )
{
	//std::locale::global( std::locale( "japanese" ) );

	init();
	initInterface( point(6,4), point(4,8+icon::size) );
	
	if( open( filename ) )  menu[mstatus].active();
	
	//initialize (necessary)
	ground.y = 0;
	thetaplus = 180;
	next = true; rebirth = true; revcalc = true;
	//strcpy( name, filename ); allready copyed at open()
	get( 0 );
	sander( optimizraw );
	trace();
	sim();
	
	menu[mpara].active();
	for( int i=0; i<tablelim; i++ ){
		table[i].setColor( GetColor( 255-cfg.BGcolor, 255-cfg.BGcolor, 255-cfg.BGcolor ) );
		table[i].set3Color( GetColor( 255, 100, 100 ), GetColor( 100, 255, 100 ), GetColor( 150, 150, 255 ) );
		table[i].setBGcolor( GetColor( cfg.BGcolor, cfg.BGcolor, cfg.BGcolor ) );
	}
	for( int i=0; i<menulim; i++ ){
		menu[i].setColor( GetColor( 255-cfg.BGcolor, 255-cfg.BGcolor, 255-cfg.BGcolor ) );
		menu[i].setBGcolor( GetColor( cfg.BGcolor, cfg.BGcolor, cfg.BGcolor ) );
	}
	
	strcpy( cmdpath, cfg.path );
	strcat( cmdpath, "cadcmd.txt" );//////////////////////////////////////////////////�֐����{�V�����t�@�C�����J���Ə�񂪌Â��Ȃ�̂œǂݒ����悤�ɂ���
	
	strcpy( dxfpath, cfg.path );
	strcat( dxfpath, "links.dxf" );
	
	for( int i=0; i<10; i++ ){ // linked colorlim=60
		const int x=25;  // 0 <= �ʓxx <= 25 
		color[00+i] = GetColor( 255-9*x, 255+(i-9)*x, 255 );
		color[10+i] = GetColor( 255-9*x, 255, 255-i*x );
		color[20+i] = GetColor( 255+(i-9)*x, 255, 255-9*x );
		color[30+i] = GetColor( 255, 255-i*x, 255-9*x );
		color[40+i] = GetColor( 255, 255-9*x, 255+(i-9)*x );
		color[50+i] = GetColor( 255-i*x, 255-9*x, 255 );
	}
}


hecken::~hecken()
{
	strcpy( cfg.name, name );
}
