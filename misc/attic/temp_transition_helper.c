#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

const char * yes[] = { "SENTINEL_WILL_BE_REPLACED_BY_CMDLINE" }; // "CH32X03x", etc. element 0 is filled in by command-line
const char * no[] = { "CH32V10x", "CH32V30x",  "CH32V20x", "CH32X03x", "CH32V003" };

char * WhitePull( const char ** sti )
{
	const char * st = *sti;
	int len = 0;
	while( ( *st == ' ' || *st == '\t' || *st == '(' ) && *st ) { st++; }
	const char * sts = st;
	while( *st != ' ' && *st != '\t' && *st != '\n' && *st != ')' && *st != '(' && *st != 0 ) { st++; len++; }
	if( *st == ')' ) { st++; }
	char * ret = malloc( len + 1 );
	memcpy( ret, sts, len );
	ret[len] = 0;
	*sti = st;
	return ret;
}

int NYI( const char * s )
{
	int ret = 2;
	char * wp = WhitePull( &s );
	int i;
	for( i = 0; i < sizeof(yes)/sizeof(yes[0]); i++ )
		if( strcmp( yes[i], wp ) == 0 ) ret = 1;
	if( ret != 1 )
		for( i = 0; i < sizeof(no)/sizeof(no[0]); i++ )
			if( strcmp( no[i], wp ) == 0 ) ret = 0;
	free( wp );
	return ret;
}

int EvalSpec( const char * spl )
{
	int rsofar = 0;
	int i;
	int lastv = 0;
	int lasto = -1;
	int ret = 0;
cont:
	char * wp = WhitePull( &spl );
	int def = -1;
	if( strcmp( wp, "defined" ) == 0 ) def = 1;
	if( strcmp( wp, "!defined" ) == 0 ) def = 2;
	if( def < 0 ) return 2;
	char * wpn = WhitePull( &spl );
	i = NYI( wpn );
//printf( "SPIN: %s/%s/%d/%d/%d\n", wp, wpn, i, def, lasto );
	if( i == 2 ) return 2;

	if( def == 2 ) i = !i;

	if( lasto == 1 )
	{
		ret = lastv || i;
	}
	else if( lasto == 2 )
		ret = lastv && i;
	else
		ret = i;

	char * wpa = WhitePull( &spl );
//printf( "WPA: \"%s\"\n", wpa );
	lastv = ret;
	lasto = -1;
//printf( "RET: %d\n", ret );
	if( strcmp( wpa, "||" ) == 0 ) { lasto = 1; goto cont; }
	else if( strcmp( wpa, "&&" ) == 0 ) { lasto = 2; goto cont; }
	else return ret;
}

// 0 for no
// 1 for yes
// 2 for indeterminate
int NoYesInd( const char * preprocc )
{
	int ret;
	int ofs = 0;
	if( strncmp( preprocc, "#if ", 4 ) == 0 ) ofs = 4;
	if( strncmp( preprocc, "#elif ", 6 ) == 0 ) ofs = 6;
	if( ofs )
	{
		ret = EvalSpec( preprocc + ofs );
		//printf( "SPEC: %d\n", ret );
	}
	else if( strncmp( preprocc, "#ifdef ", 7 ) == 0 )
	{
		const char * ep = preprocc + 6;
		char * wp = WhitePull( &ep );
		ret = NYI( wp );
		free( wp );
	}
	else if( strncmp( preprocc, "#ifndef ", 8 ) == 0 )
	{
		const char * ep = preprocc + 6;
		char * wp = WhitePull( &ep );
		ret = NYI( wp );
		if( ret < 2 ) ret = !ret;
		free( wp );
	}
	else
		ret = 2;
	//printf( "%d-> %s\n", ret, preprocc );
	return ret;
}

const char * sslineis( const char * line, const char * match )
{
	while( *line == ' ' || *line == '\t' ) line++;
	const char * linestart = line;
	while( *line && *match == *line ) { line++; match++; }
	if( *match == 0 )
		return linestart;
	else
		return 0;
}

int main( int argc, char ** argv )
{
	if( argc != 3 )
	{
		fprintf( stderr, "Syntax: transition [#define to trigger on] [file to convert]\nNo'd architectures:\n" );
		int i;
		for( i = 0; i < sizeof(no)/sizeof(no[0]); i++ )
		{
			fprintf( stderr, "\t%s\n", no[i] );
		}
		return -1;
	}

	yes[0] = argv[1];

	FILE * f = fopen( argv[2], "r" );
	if( !f )
	{
		fprintf( stderr, "Error: Could not open \"%s\"\n", argv[2] );
		return -2;
	}
	char line[1024];
	char * l;


	int depth = 0;

	// 0 = no
	// 1 = yes
	// 2 = indeterminate
	// 3 = super no. (I.e. after a true #if clause)
	int yesnoind[1024];
	yesnoind[0] = 1;

	while( l = fgets( line, sizeof(line)-1, f ) )
	{
		const char * ss = 0;
		int nyi = yesnoind[depth];
		int waspre = 0;

		if( (ss = sslineis( line, "#if " ) ) || (ss = sslineis( line, "#ifdef " ) ) || (ss = sslineis( line, "#ifndef " ) ) )
		{
			waspre = 1;
			//printf( "CHECK: %d/%s\n", depth, l );
			nyi = NoYesInd( ss );
			depth++;
			yesnoind[depth] = nyi;
		}
		else if( (ss = sslineis( line, "#elif " ) ) )
		{
			if( nyi != 2 )
			{
				waspre = 1;
				if( nyi == 1 )
				{
					nyi = 3;
				}
				else
				{
					nyi = NoYesInd( ss );
				}
				//printf( "ELIF check: %s %d\n", ss, nyi );
				yesnoind[depth] = nyi;
			}
		}
		else if( (ss = sslineis( line, "#else" ) ) )
		{
			if( nyi != 2 )
			{
				waspre = 1;
				if( yesnoind[depth] == 1 )
					nyi = 3;
				else
					nyi = !yesnoind[depth];
				yesnoind[depth] = nyi;
			}
		}
		else if( (ss = sslineis( line, "#endif" ) ) )
		{
			waspre = 1;
			depth--;
			if( depth < 0 )
			{
				fprintf( stderr, "UNTERMD IF\n" );
			}
		}

		int thisv = nyi;
		int i;
		for( i = 0; i <= depth; i++ )
		{
			//printf( "%d", yesnoind[i] ); 
			if( yesnoind[i] == 0 || yesnoind[i] == 3 ) thisv = 0;
		}
		//printf( ">>%s", l );

		if( thisv != 0 && thisv != 3 && ( thisv != 1 || !waspre ) )
		{
			printf( "%s", l );
		}
	}
}


