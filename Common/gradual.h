
class gradual
{
	int		mini, max, step;
	
  public:
	
	int		next();
	bool	live(){ return ( now >= max )? false : true; }
	
	int		now;
	
	void	set( int Mini, int Max, int Step )
	{ now = mini = Mini; max = Max; step = Step; }
	gradual( int Mini, int Max, int Step )
	{ now = mini = Mini; max = Max; step = Step; }
	gradual(){ max = step = 1; now = mini = 0; }
};

int gradual::next()
{
	if( now >= max ) return max;
	else return now += step;
}