#include <cmath>

class WaveMultiplier
{
public:
	const double PI = 3.14159;

	//3 oscillations (i.e. cycles) for both sine waves.  Cycles are tileable
	//  _        _        _            ___                   ___                   ___           
	// / \      / \      / \        __/   \__             __/   \__             __/   \__        
	//    \   /    \   /    \   /            \__     __/           \__     __/           \__     __/
	//     \_/      \_/      \_/                \___/                 \___/                 \___/
	double SineMultiplier(const long currSample, const double baseFrequency, const double verticalShift)
	{
		//Sine naturally starts at amplitude 0
		//Scale == stretchOrSquish,  Shift == TranslateLeft_OR_Right_OR_Up_OR_Down
		//Amplitude = verticalScale
		//y = Amplitude*sin( horizScale(horizValX+horizShift) ) + VerticalShift
		//SineMultiplier = sin( horizScale(horizValX+horizShift) )  ==  sin( 2PiF(timeValX+0) )
		return sin(  2*PI*baseFrequency/*constants*/ * currSample/*variable*/  ) + verticalShift;
		//2Pi radians/1cycle * frequency cycles/second  ==  rad/s  ==  1/s
	}


	//   __    __    __       1 Cycle= __     or _    _ or     __ or...      2 Cycles= __     __
	//__|  |__|  |__|  |                 |__|     |__|     |__|                          |__|   |__|
	int BalancedSquareMultiplier(const long currSample, const int numSamplesPerCycle, const double verticalShift)
	{
		const int numSamplesSpentPerOnOffState = numSamplesPerCycle>>1;	// numSamplesPerCycle>>1 == numSamplesPerCycle/2
		//Starts at amplitude 0 because why not, Sine starts at 0 too
		if(currSample%numSamplesPerCycle < numSamplesSpentPerOnOffState)
			{return 0+verticalShift;}
		else{return 1+verticalShift;}
	}


	//   _____    _____    _____       1 Cycle= ____     or __    __ or _    ___ or...      2 Cycles= ____    ____
	//__|     |__|     |__|     |                   |__|      |__|       |__|                             |__|    |__|
	int SquareMultiplier(const long currSample, const int numSamplesSpentOn, const int numSamplesSpentOff, const double verticalShift)
	{
		const int numSamplesToCompleteOneCycle = numSamplesSpentOff+numSamplesSpentOn;
		//Starts at amplitude 0 because why not, Sine starts at 0 too
		if(currSample%numSamplesToCompleteOneCycle < numSamplesSpentOff)
			{return 0+verticalShift;}
		else{return 1+verticalShift;}
	}


	//      _/|     _/|     _/|    Angled Sawtooth
	//    _/   \  _/   \  _/   \   3 cycles
	//   /      |/      |/      |
	double AngledSawtoothMultiplier(const long currSample, const int numSamplesWhereIncreasing, const int numSamplesWhereDecreasing, const double verticalShift)
	{
		//Starts at amplitude 0 because why not, Sine starts at 0 too
		const int numSamplesToCompleteOneCycle = numSamplesWhereIncreasing+numSamplesWhereDecreasing;	//E.g. 75incr,25decr = 100 samplesPerCycle
		const int numSampleInCurrCycle = currSample%numSamplesToCompleteOneCycle;		//E.g. currSample=120, numSampleInCurrCycle=120%100==20 (on the 2nd sawtooth)

		if(numSampleInCurrCycle < numSamplesWhereIncreasing)
		{
			//Next line must have numSampleInCurrCycle (ensured to fit in a single cycle), NOT currSample (may not fit in a single cycle)
			const int numGoingUpTo_NumSamplesSpentIncreasing = numSampleInCurrCycle%numSamplesWhereIncreasing;		//E.g. 20%75 == 20
			//^^^ Guarantees that, when divided by numSamplesSpentIncreasing, multiplier<=1.
			const double normalizedSampleNum_incr = (double)(numGoingUpTo_NumSamplesSpentIncreasing)/numSamplesWhereIncreasing;	//E.g. 20/75
			return normalizedSampleNum_incr + verticalShift;	//E.g. 20<75,  meaning return 20/75;
		}
		else
		{
			//Next line must have numSampleInCurrCycle (ensured to fit in a single cycle), NOT currSample (may not fit in a single cycle)
			const double normalizedSampleNum_decr = ((double)(numSamplesToCompleteOneCycle)-numSampleInCurrCycle)/numSamplesWhereDecreasing;
			//^^^   E.g. (100-95)/25 or (100-75)/25
			return normalizedSampleNum_decr + verticalShift;	//E.g. 95>=75, meaning return 5/25;  or  75>=75, meaning return 25/25;
		}
	}


	//     /|  /|  /|   Forward Sawtooth
	//    / | / | / |   3 cycles
	//   /  |/  |/  |
	double FSawtoothMultiplier(const long currSample, const int numSamplesWhereOn, const double verticalShift)
	{
		//Starts at amplitude 0 because why not, Sine starts at 0 too
		const int numGoingUpTo_NumSamplesWhereOn = currSample%numSamplesWhereOn;	//Guarantees that, when divided by numSamplesWhereOn, multiplier<=1.
		return (double)(numGoingUpTo_NumSamplesWhereOn)/numSamplesWhereOn + verticalShift;
	}


	//   |\  |\  |\     Backward Sawtooth
	//   | \ | \ | \    3 cycles
	//   |  \|  \|  \.
	double BSawtoothMultiplier(const long currSample, const int numSamplesWhereOn, const double verticalShift)
	{
		//Starts at amplitude 1 because start of tooth
		const int numGoingUpTo_NumSamplesWhereOn = currSample%numSamplesWhereOn;	//Guarantees that, when divided by numSamplesWhereOn, multiplier<=1.
		return ((double)(numSamplesWhereOn)-numGoingUpTo_NumSamplesWhereOn)/numSamplesWhereOn + verticalShift;
	}



	//To increase in frequency (get higher pitch), the ratio needs to be >1.  E.g. ratio==(2:1) or 4:3, NOT 1:2 nor 3:4.   Treat the : as a division symbol.
	//Pythagorean Tuning is a 3-limit tuning system (i.e. largest prime factor in any of its ratios is 3, though it doesn't *have* to have it)
	//E.g. 2^1:2^0 (2:1) or 3^2:2^4 (9:8) or 3^4:2^7 (108:64)
	double GetPythagFreqMultiplier(float musicalInterval)
	{
		//The more complicated the fraction, the uglier it sounds (compared to the baseFrequency)
		//Outside of one octave, the pythagorean ratios get DISGUSTING and sound HORRIBLE
			 if(musicalInterval==1){return 1.0;}			//Unison (no pitch movement)					(Do->Do)(e.g. WhiteKey->WhiteKey)
		else if(musicalInterval==2){return 256.0/243;}		// .5 whole tones up (1 semitones up)  minor 2nd		(Do->Ra)(e.g. WhiteKey->BlackKey)
		else if(musicalInterval==3){return 9.0/8;}			//  1 whole tone  up (2 semitones up)  major 2nd		(Do->Re)(e.g. WhiteKey->WhiteKey) "2nd"
		else if(musicalInterval==4){return 32.0/27;}		//1.5 whole tones up (3 semitones up)  minor 3rd		(Do->Me)(e.g. WhiteKey->BlackKey)
		else if(musicalInterval==5){return 81.0/64;}		//2   whole tones up (4 semitones up)  major 3rd		(Do->Mi)(e.g. WhiteKey->WhiteKey)
		else if(musicalInterval==6){return 4.0/3;}			//2.5 whole tones up (5 semitones up)  perfect 4th      (Do->Fa)(e.g. WhiteKey->WhiteKey) "4th"
		else if(musicalInterval==7){return 729.0/512;}		//3   whole tones up (6 semitones up)  augmented 4th	(Do->^Fi)(e.g. WhiteKey->BlackKey) "Tritone"
		else if(musicalInterval==7.5){return 1024.0/729;}	//3   whole tones up (6 semitones up)  diminished 5th	(Do->vFi)(e.g. WhiteKey->BlackKey) "Tritone"
		else if(musicalInterval==8){return 3.0/2;}			//3.5 whole tones up (7 semitones up)  perfect 5th		(Do->So)(e.g. WhiteKey->WhiteKey) "Fifth"
		else if(musicalInterval==9){return 128.0/81;}		//4   whole tones up (8 semitones up)  minor 6th		(Do->Le)(e.g. WhiteKey->BlackKey)
		else if(musicalInterval==10){return 27.0/16;}		//4.5 whole tones up (9 semitones up)  major 6th		(Do->La)(e.g. WhiteKey->WhiteKey)
		else if(musicalInterval==11){return 16.0/9;}		//5   whole tones up (10 semitones up) minor 7th		(Do->Te)(e.g. WhiteKey->BlackKey)
		else if(musicalInterval==12){return 243.0/128;}		//5.5 whole tones up (11 semitones up) major 7th		(Do->Ti)(e.g. WhiteKey->WhiteKey)
		else if(musicalInterval==13){return 2.0;}			//6   whole tones up (12 semitones up) perfect 8th		(Do->Do)(e.g. WhiteKey->WhiteKey) "Octave"
		else{return 0.0;}
	}


	//To increase in frequency (get higher pitch), the ratio needs to be >1.  E.g. ratio==(2:1) or 10:9, NOT 1:2 nor 9:10.   Treat the : as a division symbol.
	//5-limit tuning system (i.e. largest prime factor in any of its ratios is 5, though it doesn't *have* to have it)
	//E.g. 2^8:5^3 (128:125) or 3^2:2^4 (9:8) or 3^3:5^2 (27:25) or 3^2*2^2:5^2 (36:25)
	//5-Limit Tuning may be called Just Intonation even though they aren't the same.
	//Just Intonation tries to get the most "accurate" pitches and doesn't follow the 5-as-prime#-limit rule.
	double Get5LimitFreqMultiplier(float musicalInterval)
	{
		//The more complicated the fraction, the uglier it sounds (compared to the baseFrequency)
		//Outside of one octave, the ratios get slightly worse. Out of computational laziness, I'm not implementing those
			 if(musicalInterval==1){return 1.0;}		//Unison (no pitch movement)							(Do->Do)(e.g. WhiteKey->WhiteKey)
		else if(musicalInterval==2){return 16.0/15;}	// .5 whole tones up (1 semitones up)  minor 2nd		(Do->Ra)(e.g. WhiteKey->BlackKey)
		else if(musicalInterval==3){return 9.0/8;}		//  1 whole tone  up (2 semitones up)  major 2nd		(Do->Re)(e.g. WhiteKey->WhiteKey) "2nd"
		else if(musicalInterval==4){return 6.0/5;}		//1.5 whole tones up (3 semitones up)  minor 3rd		(Do->Me)(e.g. WhiteKey->BlackKey)
		else if(musicalInterval==5){return 5.0/4;}		//2   whole tones up (4 semitones up)  major 3rd		(Do->Mi)(e.g. WhiteKey->WhiteKey)
		else if(musicalInterval==6){return 4.0/3;}		//2.5 whole tones up (5 semitones up)  perfect 4th      (Do->Fa)(e.g. WhiteKey->WhiteKey) "4th"
		else if(musicalInterval==7){return 25.0/18;}	//3   whole tones up (6 semitones up)  augmented 4th	(Do->vFi)(e.g. WhiteKey->BlackKey) "Tritone"
		else if(musicalInterval==7.5){return 36.0/25;}	//3   whole tones up (6 semitones up)  diminished 5th	(Do->^Fi)(e.g. WhiteKey->BlackKey) "Tritone"
		else if(musicalInterval==8){return 3.0/2;}		//3.5 whole tones up (7 semitones up)  perfect 5th		(Do->So)(e.g. WhiteKey->WhiteKey) "Fifth"
		else if(musicalInterval==9){return 8.0/5;}		//4   whole tones up (8 semitones up)  minor 6th		(Do->Le)(e.g. WhiteKey->BlackKey)
		else if(musicalInterval==10){return 5.0/3;}		//4.5 whole tones up (9 semitones up)  major 6th		(Do->La)(e.g. WhiteKey->WhiteKey)
		else if(musicalInterval==11){return 9.0/5;}		//5   whole tones up (10 semitones up) minor 7th		(Do->Te)(e.g. WhiteKey->BlackKey)
		else if(musicalInterval==12){return 15.0/8;}	//5.5 whole tones up (11 semitones up) major 7th		(Do->Ti)(e.g. WhiteKey->WhiteKey)
		else if(musicalInterval==13){return 2.0;}		//6   whole tones up (12 semitones up) perfect 8th		(Do->Do)(e.g. WhiteKey->WhiteKey) "Octave"
		else{return 0.0;}
	}


	double GetEqualTemperamentFreqMultiplier(short musicalInterval)
	{
		if(musicalInterval<1){return 0;}
		//Outside of one octave, the ratios get no worse (at all).
		//2xBaseFrequency is an octave. 12 notes in an octave
		if(musicalInterval>12)
			{return 2*GetEqualTemperamentFreqMultiplier(musicalInterval-12);}	//Helps reduce rounding inaccuracies

		//Newton Iteration of log(x). Log(x) has the quadratic formula of ???. I'm not using it
		//12th root of 2 (sqrRoot of 2 is 2nd root of 2).  2^(1/12)=sIM=~=1.059463094   sIM^12=~=2
		double semitoneIntervalMultiplier = 1.059463094;	//Little tiny tiny tiny tiny bit flat and inaccurate

		//In Equal Temperament, diminished fifth is the same as augmented fourth
		double mult = 1.0;
		for(int i=1; i<musicalInterval; i++)
			 {mult *= semitoneIntervalMultiplier;}
		return mult;
	}



	double SumOfWavesFreqMultipliers(const long currSample, const double baseFrequency, const short numSamplesPerCycle, const double verticalShift)
	{
		const short OCTAVE = 2;
		const double baseAmplitude = SineMultiplier(currSample,numSamplesPerCycle,						verticalShift);	//longest wave with lowest frequency
		const double overtoneAmplitude1 = SineMultiplier(currSample,numSamplesPerCycle*OCTAVE,				verticalShift);
		const double overtoneAmplitude2 = SineMultiplier(currSample,numSamplesPerCycle*OCTAVE*OCTAVE,		verticalShift);
		const double overtoneAmplitude3 = SineMultiplier(currSample,numSamplesPerCycle*OCTAVE*OCTAVE*OCTAVE,verticalShift);	//shortest wave with highest frequency
		const double overtoneAmplitude4 = SineMultiplier(currSample,numSamplesPerCycle<<4/*x16*/,verticalShift);	//shortest wave with highest frequency
		const double overtoneAmplitude5 = SineMultiplier(currSample,numSamplesPerCycle<<5/*x32*/,verticalShift);	//shortest wave with highest frequency
		const double overtoneAmplitude6 = SineMultiplier(currSample,numSamplesPerCycle<<6/*x64*/,verticalShift);	//shortest wave with highest frequency
		return baseAmplitude + overtoneAmplitude1/2 + overtoneAmplitude2/3 + overtoneAmplitude3/4 + overtoneAmplitude4/5 + overtoneAmplitude5/6 + overtoneAmplitude6/7;
	}
	double FrequencyPattern1(const double timeVal, const double maxTimeVal, const int numTimeIntervals, const double baseFrequency)
	{
		double shortestTI = maxTimeVal/numTimeIntervals;	//T is the shortest usable time interval (since I refuse to multiply by any #<1)
		if(timeVal<maxTimeVal)
		{
				if(timeVal<shortestTI*2)	return baseFrequency;		//Musical 1st
			else if(timeVal<shortestTI*4)	return baseFrequency*3/2;	//Musical 5th
			else if(timeVal<shortestTI*6)	return baseFrequency*2;		//Musical octave
			else if(timeVal<shortestTI*8)	return baseFrequency*4/3;	//Musical 3rd
			else if(timeVal<shortestTI*10)	return baseFrequency*8/3;	//Musical octave+4th
			else if(timeVal<shortestTI*12)	return baseFrequency*5/3;	//Musical 6th
			else if(timeVal<shortestTI*14)	return baseFrequency*5/4;	//Musical 3rd
		}
		return baseFrequency;
	}
	double FrequencyPattern_ElectronicDeathMetal(const long currSample, const double baseFrequency, const short numSamplesPerCycle, const double tuningRatio, const double verticalShift)
		{return tuningRatio * baseFrequency * SineMultiplier(currSample,numSamplesPerCycle*tuningRatio, verticalShift);}
};