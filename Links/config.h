
const int maxFileName = 256;


int limit( int now, int min, int max )
{
	if( now > max ) return max;
	if( now < min ) return min;
	return now;
}
double limit( double now, double min, double max )
{
	if( now > max ) return max;
	if( now < min ) return min;
	return now;
}

class config
{
	void init();
 public:
	int fontsize, BGcolor;
	point screen;
	double plspan;
	bool bold, dxfheel, first;
	char name[maxFileName], path[maxFileName], title[maxFileName], 
			cfgpath[maxFileName], cfgname[maxFileName];

	config();
	~config();
}cfg;


config::config()
{
	init();
	strcpy( cfgname, "config.ini" );
	strcpy( title, "Links" );


	char buf[maxFileName];

	strcpy( buf, GetCommandLine() );////////////////////////////////////////////////////////////�֐���
	if( buf[0] == '"' ){
		strcpy( path, buf+1 ); 
		for( int i=2; i<=strlen(path); i++ )
			if( path[i]=='"'){ path[i] = '\0'; break; }
	}else{
		strcpy( path, buf );
	}

	for( int i=strlen(path); i>=0; i--)
	{
		if( path[i] == '\\' )
		{
			path[i+1] = '\0';
			break;
		}
	}
	strcpy( cfgpath, path );
	strcat( cfgpath, cfgname );
	
	//std::locale::global( std::locale( "japanese" ) );
	std::ifstream fin( cfgpath );
	if( fin ){
		fin.getline( buf, maxFileName ); screen.x = atof( buf );
		fin.getline( buf, maxFileName ); screen.y = atof( buf );
		fin.getline( buf, maxFileName ); fontsize = atoi( buf );
		fin.getline( buf, maxFileName ); bold = atoi( buf );
		fin.getline( buf, maxFileName );// vertex = atoi( buf );
		fin.getline( buf, maxFileName ); plspan = atof( buf );
		fin.getline( buf, maxFileName ); BGcolor = atoi( buf );
		fin.getline( name, maxFileName, '\t' );
		
		
		if( 400 > screen.x || 1500 < screen.x ) screen.x = 800;
		if( 300 > screen.y || 1200 < screen.y ) screen.y = 600;
		fontsize = limit( fontsize, 8,32 );
		plspan = limit( plspan, 0.0, 10.0);
		BGcolor = limit( BGcolor, 0, 255 );
		
		first = false;
	}
	else first = true;
}

void config::init()
{
	screen.x = 800;
	screen.y = 600;
	fontsize = 16;
	bold = false;
	plspan = 2;
	dxfheel = false;
	BGcolor = 0;
}

config::~config()
{
	//std::locale::global( std::locale("japanese" ));
	std::ofstream fout( cfgpath );	


	fout << screen.x << "\t��ʉ����B�W��800�B\n";
	fout << screen.y << "\t��ʏc���B�W��600�B\n";
	fout << fontsize << "\t�t�H���g�傫���B�W��16�B12�`18���x�B\n";
	fout << bold << "\t�t�H���g�����\���B0�Œʏ�A����ȊO�̐��l�ŋ����\���B\n";
	fout << plspan << "\t���ݒ�ł����C�݊����̈׈ێ��D�i�|�����C���̒��_�\���j\n";
	fout << plspan << "\tDXF�t�@�C���ǂݍ��݂̉~�ʐ��x�B0�ŉ~�ʖ������������B�W��2�B\n";
	fout << BGcolor << "\t�w�i�l�i�O���[�X�P�[���j0�ō��A255�Ŕ��B\n";
	fout << name << "\t�����Ǎ��t�@�C���p�X�B���΃p�X�B���{��B\n";
}