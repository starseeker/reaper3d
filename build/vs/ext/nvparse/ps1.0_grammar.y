%start WholeEnchilada
%{

/*

	This is a parser for the DX8 PS1.0 pixel shaders.  I intend
	to use it to set NV_texture_shader* and NV_register_combiners*
	state in OpenGL, but the parse tree could be used for any
	other purpose.

	Cass Everitt
	7-19-01

*/

void yyerror(char* s);
int yylex ( void );

#ifdef _WIN32
# include <windows.h>
#endif

#include <stdio.h>
#include <malloc.h>

#include "ps1.0_program.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"

#include <list>
#include <vector>

using namespace std;
using namespace ps10;

//#define DBG_MESG(msg, line)  	errors.set(msg, line)
#define DBG_MESG(msg, line)


%}
%union 
{
	int ival;
	float fval;
	
	string * sval;
	constdef * cdef;
	vector<constdef> * consts;
	vector<string> * line;
	list<vector<string> > * lines;
}

%token <ival> HEADER NEWLINE
%token <fval> NUMBER
%token <sval> REG
%token <sval> DEF
%token <sval> ADDROP
%token <sval> BLENDOP

%type <cdef> Def
%type <consts> Defs
%type <line> AddrOp BlendOp
%type <lines> AddrOps BlendOps


%%

WholeEnchilada :

	HEADER NEWLINE Defs AddrOps BlendOps
	{
		DBG_MESG("dbg: WholeEnchilada", line_number);		
		ps10::invoke($3, $4, $5);
	}
	|

	HEADER NEWLINE AddrOps BlendOps
	{
		DBG_MESG("dbg: WholeEnchilada", line_number);		
		ps10::invoke( 0, $3, $4);
	}
	|

	HEADER NEWLINE Defs BlendOps
	{
		DBG_MESG("dbg: WholeEnchilada", line_number);		
		ps10::invoke($3, 0, $4);
	}
	|

	HEADER NEWLINE BlendOps
	{
		DBG_MESG("dbg: WholeEnchilada", line_number);		
		ps10::invoke( 0, 0, $3);
	}
	;


Defs :

	Def
	{
		$$ = new vector<constdef>;
		$$->push_back(* $1);
		delete $1;
	}
	|

	Defs Def
	{
		$$ = $1;
		$$->push_back(* $2);
		delete $2;
	}
	;



Def :

	DEF REG ',' NUMBER ',' NUMBER ',' NUMBER  ',' NUMBER NEWLINE
	{
		$$ = new constdef;
		$$->reg = * $2;
		$$->r = $4;
		$$->g = $6;
		$$->b = $8;
		$$->a = $10;
		delete $2;
	}


AddrOps:

	AddrOp
	{
		$$ = new list<vector<string> >;
		$$->push_back(* $1);
		delete $1;
	}
	|

	AddrOps AddrOp
	{
		$$ = $1;
		$$->push_back(* $2);
		delete $2;
	}
	;


AddrOp :

	ADDROP REG NEWLINE
	{
		$$ = new vector<string>;
		$$->push_back(* $1);
		$$->push_back(* $2);
		delete $1;
		delete $2;
	}
	|

	ADDROP REG ',' REG NEWLINE
	{
		$$ = new vector<string>;
		$$->push_back(* $1);
		$$->push_back(* $2);
		$$->push_back(* $4);
		delete $1;
		delete $2;
		delete $4;
	}
	|

	ADDROP REG ',' REG ',' REG NEWLINE
	{
		$$ = new vector<string>;
		$$->push_back(* $1);
		$$->push_back(* $2);
		$$->push_back(* $4);
		$$->push_back(* $6);
		delete $1;
		delete $2;
		delete $4;
		delete $6;
	}
	;


BlendOps:

	BlendOp
	{
		$$ = new list<vector<string> >;
		$$->push_back(* $1);
		delete $1;
	}
	|

	BlendOps BlendOp
	{
		$$ = $1;
		$$->push_back(* $2);
		delete $2;
	}
	;


BlendOp :


	BLENDOP REG ',' REG NEWLINE
	{
		$$ = new vector<string>;
		$$->push_back(* $1);
		$$->push_back(* $2);
		$$->push_back(* $4);
		delete $1;
		delete $2;
		delete $4;
	}
	|

	BLENDOP REG ',' REG ',' REG NEWLINE
	{
		$$ = new vector<string>;
		$$->push_back(* $1);
		$$->push_back(* $2);
		$$->push_back(* $4);
		$$->push_back(* $6);
		delete $1;
		delete $2;
		delete $4;
		delete $6;
	}
	|

	BLENDOP REG ',' REG ',' REG ',' REG NEWLINE
	{
		$$ = new vector<string>;
		$$->push_back(* $1);
		$$->push_back(* $2);
		$$->push_back(* $4);
		$$->push_back(* $6);
		$$->push_back(* $8);
		delete $1;
		delete $2;
		delete $4;
		delete $6;
		delete $8;
	}
	;




%%

void yyerror(char* s)
{
	errors.set("parser: syntax error", line_number);
}
