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
			
				DrawFormatString( datam.x, datam.y+10, color, "速度 = Now[%3.2f] Max[%3.2f] Min[%3.2f]", acc, maxacc, minacc );
				DrawFormatString( datam.x, datam.y+30, color, "レバー角度 = Band[%3.2f] Max[%3.2f] Min[%3.2f]",
					deg(max_lever_angle-min_lever_angle), deg(max_lever_angle), deg(min_lever_angle) );
				DrawFormatString( datam.x, datam.y+50, color, "コンロッド角度 = Band[%3.2f] Max[%3.2f] Min[%3.2f]", 
					deg(max_conrod_angle-min_conrod_angle), deg(max_conrod_angle), deg(min_conrod_angle) );
				DrawFormatString( datam.x, datam.y+70, color, "最適化曲線 端部振上げ高さ = 後[%3.2f] 前[%3.2f]", 
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
	static double scroll = 0;/////////////////////////////////////////////フォントサイズに応じたスクロール
	
	DrawBox( 12-3, cfg.screen.y-12-cfg.fontsize-3, cfg.screen.x-12-icon::size+3, cfg.screen.y-12+3, black, TRUE );
	DrawBox( 12-4, cfg.screen.y-12-cfg.fontsize-4, cfg.screen.x-12-icon::size+4, cfg.screen.y-12+4, gray, FALSE );

	if( cntr >= 50 ) scroll += 4;
	if( menu[mopen].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "拡張子問わずLinksで保存したファイルのみ開けます。ドラック＆ドロップでも可能。", clrhelp );
	if( menu[msave].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "保存してない場合は、名前を付けて保存をします", clrhelp );
	if( menu[msaveas].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "拡張子は保存/読込みには関係無いので何でも構いません。", clrhelp );
	if( menu[mnew].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "DXF95/2000年形式で保存されたファイルのポリラインと円のみ読み込めます　回転の中心を原点に指定して下さい", clrhelp );
	if( menu[mpara].getFocus() ) check = true, DrawString( 12-scroll, cfg.screen.y-12-cfg.fontsize, "パラメータ表は，数値をホイール回転などにより操作する表です　ダブルクリックで直接入力，ホイールクリックで回転時の増加量が変化します", clrhelp );

	if( menu[mstop].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "調整をする際には回転を停止させホイールで角度を変えると確認しやすいです", clrhelp );
	if( menu[mcmd].getFocus() ) check = true, DrawString( 12-scroll, cfg.screen.y-12-cfg.fontsize, "AutoCADのコマンドラインに貼付けると，この画面と同じように作図されます　SHIFTを押しながらクリックすると脚のみが、CTRLではシルエットが作図されます。", clrhelp );
	if( menu[mdxfout].getFocus() ) check = true, DrawString( 12-scroll, cfg.screen.y-12-cfg.fontsize, "この画面と同じものがDXF出力されます　SHIFTを押しながらクリックすると脚のみが、CTRLではシルエットが作図されます。", clrhelp );
	if( menu[mlnkside].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "リンク機構を他の向きに変更する。", clrhelp );
	if( menu[mdxf].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "最適化曲線表示と読込んだDXF表示（読込んでない場合は何も表示されません）を切替えします", clrhelp );
	if( menu[mturn].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "回転方向を切替えます", clrhelp );
	
	if( menu[manibmp].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "範囲を選択して連番BMPで出力（GIFアニメーション向け）", clrhelp );
	if( menu[morbit].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "最適化曲線の端部軌道を表示します（DXF時には表示されません）", clrhelp );
	
	if( menu[mresol].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "ウィンドウサイズを変更します（config.iniファイルで細かく設定可）", clrhelp );
	if( menu[mgraph].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "表から選択する任意の点の加速度/速度をグラフとして表示します", clrhelp );

	if( menu[mpara].get() )
	{
		if( table[tupdown].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "計算により求められた上下動です　これに限って編集はできません", clrhelp );
		if( table[tresol].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "円一周辺りの計算回数です　多いほど精度と計算負荷が高くなります", clrhelp );
		if( table[ttheta].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "現在の基準クランクの角度です　自動で変わりますが編集も可能です", clrhelp );
		if( table[tobject].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "オブジェクト（リンク）の数です　大量にすればシルエットが得られます", clrhelp );
		if( table[tscale].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "表示倍率です　拡大しても計算の精度には関係しません", clrhelp );
		if( table[ttilt].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "リンク機構を一時的に傾けることができます　傾けたままでは保存が出来ません", clrhelp );
		if( table[troll].getFocus() ) check = true, DrawString( 12-scroll, cfg.screen.y-12-cfg.fontsize, "毎分回転数です　ゆっくりしたい場合は，停止して回転角度を変化させると便利です　負荷が高い場合は限界になります", clrhelp );
		if( table[tcrankr].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "クランク半径です　大きくしすぎるとリンク機構が成立しづらくなります", clrhelp );
		if( table[tconrod].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "コンロッド長さです　スライダリンクの場合は計算に影響しません", clrhelp );
		if( table[tleverr].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "レバー長さです　０にするとスライダリンクに差し変わります", clrhelp );
		if( table[tleverx].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "レバー（スライダ）支点のX座標です　クランク中心が原点で，右が＋です", clrhelp );
		if( table[tlevery].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "レバー（スライダ）支点のY座標です　クランク中心が原点で，下が＋です", clrhelp );
		if( table[tshiftx].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "軌道を表示する為にクランクからずらす距離のX成分です", clrhelp );
		if( table[tshifty].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "軌道を表示する為にクランクからずらす距離のY成分です", clrhelp );
		if( table[tmini].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "最適化曲線の前部の幅です　大きいと上下動が少なくなりやすくなります", clrhelp );
		if( table[tmax].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "最適化曲線の後部の幅です　ピッチが大きいと繰り上がりの関係で変化しない場合があります", clrhelp );
		if( table[tstep].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "最適化曲線の精度です　縞々部分の長さとも言えます", clrhelp );
		if( table[theight].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "最適化する際のクランク中心から地面までの距離です　距離が長いと足も大きくなります", clrhelp );
		if( table[tgraph].getFocus() ) check = true, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "表示するグラフを選択します。アイコンからON/OFFできます。", clrhelp );
	}
	if( !check ) scroll = cntr = 0, DrawString( 12, cfg.screen.y-12-cfg.fontsize, "ヘルプ：アイコンをクリックし機能を呼び出すか，パラメータ上でホイールを回転させ値を変更して下さい", clrhelp );
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
	menu[mopen].init	( m.x+i*(icon::size+span), m.y, IDB_OPEN, "ファイルを開く"  );i++;
	menu[msave].init	( m.x+i*(icon::size+span), m.y, IDB_SAVE, "上書き保存" );i++;
	menu[msaveas].init	( m.x+i*(icon::size+span), m.y, IDB_SAVEAS, "名前を付けて保存" );i++;
	menu[mnew].init		( m.x+i*(icon::size+span), m.y, IDB_NEW, "DXFファイルを読み込む" );i++;
	//menu[mconfig].init	( m.x+i*(icon::size+span), m.y, IDB_PROPERTY, "設定変更" );i++;
	menu[mpara].init	( m.x+i*(icon::size+span), m.y, IDB_MENUON,"パラメータ表を表示", IDB_CLOSE, "パラメータ表を隠す" );i++;
	
	int space=10;
	menu[mstop].init	( m.x+i*(icon::size+span)+space, m.y, IDB_START, "回転開始", IDB_STOP, "回転停止" );i++;
	menu[mcmd].init	( m.x+i*(icon::size+span)+space, m.y, IDB_PASTE, "AutoCADコマンドをクリップボードに貼付け（SHIFT、CTRLで変化）" );i++;
	menu[mdxfout].init	( m.x+i*(icon::size+span)+space, m.y, IDB_PASTEDXF, "DXFファイルを出力（SHIFT、CTRLでオプション出力）" );i++;
	menu[mlnkside].init	( m.x+i*(icon::size+span)+space, m.y, IDB_LSIDE, "リンクサイド変更" );i++;
	menu[mdxf].init	( m.x+i*(icon::size+span)+space, m.y, IDB_DXF, "DXF表示", IDB_OPT, "最適化曲線表示" );i++;
	menu[mturn].init	( m.x+i*(icon::size+span)+space, m.y, IDB_LTURN, "時計回り", IDB_RTURN, "反時計回り" );i++;

	space=20;
	//menu[mrelord].init	( m.x+i*(icon::size+span)+space, m.y, IDB_RELOAD, "最新の状態に更新" );i++;
	//menu[mturbo].init	( m.x+i*(icon::size+span)+space, m.y, IDB_TURBO, "高速描画モードへ", IDB_START, "通常描画モードへ" );i++;
	menu[manibmp].init	( m.x+i*(icon::size+span)+space, m.y, IDB_ANIME, "アニメーションを連番ビットマップで保存" );i++;
	menu[morbit].init	( m.x+i*(icon::size+span)+space, m.y, IDB_ORBIT, "端部軌道を表示" );i++;
	menu[mresol].init	( m.x+i*(icon::size+span)+space, m.y, IDB_RESOL, "ウィンドウサイズ変更" );i++;
	menu[mgraph].init	( m.x+i*(icon::size+span)+space, m.y, IDB_GRAPH, "グラフを表示", IDB_CLOSE, "グラフを隠す" );i++;
	
	
	span=6;
	menu[mstatus].init	( cfg.screen.x-(icon::size+span), cfg.screen.y-(icon::size+span), 
		IDB_HELPON, "ステータスバーを表示", IDB_CLOSE, "ステータスバーを隠す" );
	
	i=0;
	
	//if( key_code[KEY_INPUT_LSHIFT] && key_code[KEY_INPUT_B] && key_code[KEY_INPUT_D] )

	if( limiter )
	{
		menu[marcmode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "アークスライダモードへ", "", "通常モードへ" );
		menu[mdsmode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "デュアルスライダモードへ", "", "通常モードへ" );
		menu[mvtmode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "ヴァーティカルモードへ(不可逆)", "", "通常モードへ" );
		menu[mdcmode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "ダブルクランクモードへ", "", "通常モードへ" );
		menu[mmmlmode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "長さ表示モードへ", "", "通常モードへ" );

		menu[mfomode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "脚前部軌道表示" );
		menu[mromode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "脚後部軌道表示" );
		menu[msomode].init	( i++*(icon::size+2*span), cfg.screen.y-(icon::size+span), "", "隠し軌道表示" );
	}
	
	i=0;
	span=4;
	table[tupdown].init	( t.x, t.y+i*(cfg.fontsize+span), &updown, "上下動" );i++;
	table[tupdown].setLock();
	table[tresol].init	( t.x, t.y+i*(cfg.fontsize+span), &resol, "解円度" );i++;
	table[ttheta].init	( t.x, t.y+i*(cfg.fontsize+span), &theta, "回転角度" );i++;
	table[tobject].init	( t.x, t.y+i*(cfg.fontsize+span), &object, "オブジェクト数" );i++;
	table[tscale].init	( t.x, t.y+i*(cfg.fontsize+span), &scale, "表示倍率" );i++;
	table[ttilt].init	( t.x, t.y+i*(cfg.fontsize+span), &tilt, "傾き" );i++;
	table[troll].init	( t.x, t.y+i*(cfg.fontsize+span), &roll, timeover?"毎分回転数(限界)":"毎分回転数" );i++;
	table[tcrankr].init	( t.x, t.y+i*(cfg.fontsize+span), &crankr, "クランク半径" );i++;
	table[tconrod].init	( t.x, t.y+i*(cfg.fontsize+span), &conrod, "コンロッド長さ" );i++;
	table[tleverr].init	( t.x, t.y+i*(cfg.fontsize+span), &leverr, leverr?"レバー長さ":"スライダ(レバー長さ)" );i++;
	if( !doublecrank )
	{
		table[tleverx].init	( t.x, t.y+i*(cfg.fontsize+span), &lever.x, leverr?"レバーＸ座標":"スライダＸ座標" );i++;
		table[tlevery].init	( t.x, t.y+i*(cfg.fontsize+span), &lever.y, leverr?"レバーＹ座標":"スライダＹ座標" );i++;
	}else{
		table[tleverx].mun();
		table[tlevery].mun();
	}
	table[tshiftx].init	( t.x, t.y+i*(cfg.fontsize+span), &shift.x, "シフトＸ距離" );i++;
	table[tshifty].init	( t.x, t.y+i*(cfg.fontsize+span), &shift.y, "シフトＹ距離" );i++;
	if( !dxf ){ 
		table[tmini].init	( t.x, t.y+i*(cfg.fontsize+span), &mini, "最適化曲線前幅" );i++;
		table[tmax].init	( t.x, t.y+i*(cfg.fontsize+span), &max, "最適化曲線後幅" );i++;
		table[tstep].init	( t.x, t.y+i*(cfg.fontsize+span), &step, "最適化曲線ピッチ" );i++;
		table[theight].init	( t.x, t.y+i*(cfg.fontsize+span), &height, "最適化曲線高さ" );i++;
	}else{ 
		table[tmini].mun(); table[tmax].mun(); table[tstep].mun(); table[theight].mun(); 
	}
	if( mode == mdualslider || mode == mdualhecken ){ 
		table[tsubconrod].init	( t.x, t.y+i*(cfg.fontsize+span), &subconrod, "サブコンロッド長さ" );i++;
		table[tsubleverx].init	( t.x, t.y+i*(cfg.fontsize+span), &sublever.x, "サブレバーＸ座標" );i++;
		table[tsublevery].init	( t.x, t.y+i*(cfg.fontsize+span), &sublever.y, "サブレバーＹ座標" );i++;
	}else{ 
		table[tsubconrod].mun(); table[tsubleverx].mun(); table[tsublevery].mun(); 
	}
	if( doublecrank )
	{
		table[tsubcrankx].init	( t.x, t.y+i*(cfg.fontsize+span), &subcrank.x, "サブクランクＸ座標" );i++;
		table[tsubcranky].init	( t.x, t.y+i*(cfg.fontsize+span), &subcrank.y, "サブクランクＹ座標" );i++;
		table[tsubcrankr].init	( t.x, t.y+i*(cfg.fontsize+span), &subcrankr, "サブクランク半径" );i++;
		table[tthetaplus].init	( t.x, t.y+i*(cfg.fontsize+span), &thetaplus, "サブクランク位相" );i++;
		table[tthetaplus].setMagni( 5 );
	}else{
		table[tsubcrankx].mun();
		table[tsubcranky].mun();
		table[tsubcrankr].mun();
		table[tthetaplus].mun();
	}
	if( exist_graph ) table[tgraph].init( t.x, t.y+i*(cfg.fontsize+span), &graph_mode, "グラフ描画パラメータ" ), i++;
	else table[tgraph].mun();
		
	
	//table[tdxf].init	( t.x, t.y+i*(cfg.fontsize+span), &dxf, "最適化曲線を表示", "ＤＸＦを表示" );i++;
	//table[tturn].init	( t.x, t.y+i*(cfg.fontsize+span), &turn, "反時計回りにする", "時計回りにする" );i++;
	
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
	// rebirth中は、リロードアイコンを表示・検知しない。
	
	//pushback switch
	if( menu[mopen].get() ){
		
 		if( helpmode ) messageOK( "新しいファイルと開くと現在の数値は破棄されます。" );
		if( openFileDialog( name, "Links形式ファイル(*.links)\0*.links\0全てのファイル(*.*)\0*.*\0\0" ) ) open( name );
		rebirth = true;
	}
	if( menu[msaveas].get() ){
		if( !tilt || 1 ){   // DEBUG TODO 
			char buf[maxFileName];
 			saveFileDialog( name, "Links形式ファイル(*.links)\0*.links\0全てのファイル(*.*)\0*.*\0\0" );  save( name );
			sprintf( buf, "%s  [ %s ] ", cfg.title, name );
			setWindowText( buf );
		}else  messageOK( "申し訳ありませんが、傾きが0以外の時は保存できません。\n今後改善予定です。" );
	}
	if( menu[msave].get() ){
		if( !tilt || 1 ){   // DEBUG TODO 
 			save( name );
		}else  messageOK( "申し訳ありませんが、傾きが0以外の時は保存できません。\n今後改善予定です。" );
	}
	if( menu[mnew].get() )  loadDXF();
	
	if( menu[mconfig].get() ) messageOK( "申し訳ありません。設定はまだご利用になれません。" );
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
		sprintf( msg, "[ %s ] を読込みますがよろしいですか？", buf );
		if( messageYN( msg, "D&D確認" ) )
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
	
	fout << resol << "\t解円度\n";
	fout << theta << "\t回転角度\n";
	fout << object << "\tオブジェクト数\n";
	fout << scale << "\t表示倍率\n";
	fout << tilt << "\t傾き\n";
	fout << roll << "\t毎分回転数\n";
	
	fout << dxf << "\tＤＸＦファイル表示\n";
	fout << side << "\tリンクサイド\n";
	
	fout << crankr << "\tクランク半径\n";
	fout << conrod << "\tコンロッド長さ\n";
	fout << leverr << "\tレバー長さ\n";
	fout << lever.x << "\tレバーＸ座標\n";	
	fout << lever.y << "\tレバーＹ座標\n";
	fout << shift.x << "\tシフトＸ距離\n";
	fout << shift.y << "\tシフトＹ距離\n";
	
	fout << height << "\t最適化曲線高さ\n";
	fout << mini << "\t最適化曲線前幅\n";
	fout << max << "\t最適化曲線後幅\n";
	fout << step << "\t最適化曲線ピッチ\n";
	
	fout << subcrankr << "\tサブクランク半径\n";
	fout << subcrank.x << "\tサブクランクＸ座標\n";
	fout << subcrank.y << "\tサブクランクＹ座標\n";
	fout << subconrod << "\tサブコンロッド長さ\n";
	fout << subleverr << "\tサブレバー長さ\n";
	fout << sublever.x << "\tサブレバーＸ座標\n";	
	fout << sublever.y << "\tサブレバーＹ座標\n";
	fout << subshift.x << "\tサブシフトＸ距離\n";
	fout << subshift.y << "\tサブシフトＹ距離\n";
	
	fout << conrodDXF << "\tコンロッド用ＤＸＦファイル\n";
	fout << leverDXF << "\tレバー用ＤＸＦファイル\n";
	fout << crankDXF << "\tクランク用ＤＸＦファイル\n";
	fout << subDXF << "\tサブ用ＤＸＦファイル\n";
	fout << bgDXF << "\tバックグラウンド用ＤＸＦファイル\n";
	
	
	fout << "eof";
	
	//strcpy( cfg.name, name );
	strcpy( cfg.name, filename );
}


bool hecken::open( char filename[] )
{
	if( filename[0] == '"' ){ strcpy( name, filename + 1 ); name[strlen(name)-1] = '\0'; } // ""はずし
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
		if( menu[mopen].get() ) messageOK( "ファイル書式エラー\nLinksで保存したファイルか確認して下さい．" );
		else messageOK( "ファイルを開けませんでした。" );
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
	if( fout.bad() || fout.fail() ) messageOK( "エラーが発生しました。ファイル生成失敗。\nファイルが使用中でないか確認して下さい。" );

	if( DXF )  fout << " 0\nSECTION\n 2\nENTITIES\n";
	
	if( maxObject )
	{
		int maxNum = resol/5;		// 暫定対処
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
		
		DrawFormatString( 150,25,white, "保存範囲を2回のクリックで示してください。キャンセルはエスケープキーです。" );
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
		
 		if( helpmode ) messageOK( "各種ＤＸＦファイルを読み込みます。\n"
			"いいえで空のファイルを読み込み、キャンセルで現在のままになります。" );
		
		int buf;
		if( (buf = messageYNC( "コンロッド用ＤＸＦファイルを読み込みますか？" )) == IDYES ){
			dxff dxfx;
			openFileDialog( conrodDXF, filter );
			dxfconrod = dxfx.read( conrodDXF, conrodraw, conroden );
		}else if( buf == IDNO ){ conrodDXF[0] = '\0'; dxfconrod = false; }

		if( (buf = messageYNC( 
			(arcmode?"アークスライダ軌道用DXFファイルを読込みますか？":"レバー用ＤＸＦファイルを読み込みますか？")
			)) == IDYES  ){
			dxff dxfx;
			openFileDialog( leverDXF, filter );
			dxflever = dxfx.read( leverDXF, leverraw, leveren );
		}else if( buf == IDNO ){ leverDXF[0] = '\0'; dxflever = false; }

		if( (buf = messageYNC( "クランク用ＤＸＦファイルを読み込みますか？" )) == IDYES  ){
			dxff dxfx;
			openFileDialog( crankDXF, filter );
			dxfcrank = dxfx.read( crankDXF, crankraw, cranken );
		}else if( buf == IDNO ){ crankDXF[0] = '\0'; dxfcrank = false; }

		if( (mode == mdualhecken || mode == mdualslider) && 
			((buf = messageYNC( "サブ用ＤＸＦファイルを読み込みますか？" )) == IDYES ) ){
			dxff dxfx;
			openFileDialog( subDXF, filter );
			dxfsub = dxfx.read( subDXF, subraw, suben );
		}else if( buf == IDNO ){ subDXF[0] = '\0'; dxfsub = false; }

		if( (buf = messageYNC( "バックグラウンド用ＤＸＦファイルを読み込みますか？" )) == IDYES  ){
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
	strcat( cmdpath, "cadcmd.txt" );//////////////////////////////////////////////////関数化＋新しいファイルを開くと情報が古くなるので読み直すようにする
	
	strcpy( dxfpath, cfg.path );
	strcat( dxfpath, "links.dxf" );
	
	for( int i=0; i<10; i++ ){ // linked colorlim=60
		const int x=25;  // 0 <= 彩度x <= 25 
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
