/**
 * @file TimestampTools.h
 * @ingroup DataManagement
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 * @copyright All right reserved.
 */

#ifndef __TIMESTAMP_TOOLS_H__
#define __TIMESTAMP_TOOLS_H__

#include <time.h>
#include <sys/timeb.h>

#include <stdio.h>
#include <string>

#if defined WIN32 || defined WIN64
	#ifndef DRAWING_TIMEB
		#define DRAWING_TIMEB
		typedef struct _timeb TimeB;		/*!< @brief define a TimeB structure */
		#pragma warning( disable : 4996 )
	#endif
#else
	#ifndef DRAWING_TIMEB
		#define DRAWING_TIMEB
		typedef struct timeb TimeB;			/*!< @brief define a TimeB structure */
	#endif
#endif

inline void operator-=(TimeB &TimeToModify, int Milliseconds );

/** @brief Add Milliseconds to a specified TimeB. This is a non cascadable += operator.
 *
 * @param TimeToModify [in, out] The time to change.
 * @param Milliseconds [in] the time in ms to add.
 */
inline void operator+=(TimeB &TimeToModify, int Milliseconds )
{
	if ( Milliseconds < 0 )
	{
		TimeToModify -= -Milliseconds ;
		return;
	}

	int SecondsToAdd = Milliseconds/1000;
	int MillisecondsToAdd = Milliseconds%1000;

	TimeToModify.millitm += MillisecondsToAdd;
	if ( TimeToModify.millitm >= 1000 )
	{
		SecondsToAdd += TimeToModify.millitm/1000;
		TimeToModify.millitm = TimeToModify.millitm%1000;
	}
	
	TimeToModify.time += SecondsToAdd;
}

/** @brief Remove Milliseconds to a specified TimeB. This is a non cascadable -= operator.
 *
 * @param TimeToModify [in, out] The time to change.
 * @param Milliseconds [in] the time in ms to remove.
 */
inline void operator-=( TimeB &TimeToModify, int Milliseconds )
{
	if ( Milliseconds < 0 )
	{
		TimeToModify += -Milliseconds;
		return;
	}

	int SecondsToMin = Milliseconds/1000;
	int MillisecondsToMin = Milliseconds%1000;

	if ( MillisecondsToMin > TimeToModify.millitm )
	{
		int tmp = (int)TimeToModify.millitm - MillisecondsToMin;
		SecondsToMin += 1;
		TimeToModify.millitm = 1000+tmp%1000;
	}
	else
	{
		TimeToModify.millitm -= MillisecondsToMin;
	}
	
	TimeToModify.time -= SecondsToMin;
}

/** @brief Compare 2 TimeB. Return values are :
 *         - <0 if t1 is less than t2
 *         - >0 if t1 is greater than t2
 *         - 0 if t1 equals t2
 *
 * @param TimeToModify [in] The time to change.
 * @param Milliseconds [in] the time in ms to remove.
 */
inline int CompareTime( const TimeB &t1, const TimeB &t2 )
{
	return (int)((t1.time - t2.time)*1000 + (t1.millitm-t2.millitm));
}

#endif // __TIMESTAMP_TOOLS_H__