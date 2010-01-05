/******************************************************************************
 *
 *  File Name........: ice9.h
 *
 *  Description......:
 *
 *  Created by vin on 01/10/02
 *
 *  Revision History.:
 *
 *  $Log: ice9.h,v $
 *  Revision 1.3  2002/01/18 20:47:09  vin
 *  Three working phases: all the way to CG'g C code.
 *  Robustness not verified.
 *
 *  Revision 1.2  2002/01/16 22:14:03  vin
 *  The basics of semantics are in and working.
 *
 *
 *  $Id: ice9.h,v 1.3 2002/01/18 20:47:09 vin Exp $
 *
 *****************************************************************************/

#ifndef ICE9_H
#define ICE9_H

#define YYLMAX	256
#define LINE	(yynewlines)
extern int yynewlines, ErrorLimit, DoSemantic;
extern void Fatal(int, char *, ...);
extern void FatalS(int, char *, ...);
extern void Warning(int, char *, ...);
extern void CompilerError(int, char*, ...);
extern int Verbose(int, const char *, ...);


#endif /* ICE9_H */
/*........................ end of ice9.h ....................................*/
