//Steven Lynch  7/21/2022, 7/24/2022-7/26/2022 (worked on this for four days)
//  https://www.youtube.com/watch?v=rHqkeLxAsTc       Details the .wav format
//  https://en.wikipedia.org/wiki/Pythagorean_tuning
//  https://en.wikipedia.org/wiki/Just_intonation
//  https://en.wikipedia.org/wiki/Five-limit_tuning
#include <iostream>	//input output streams
#include <string>	//getline(stream, storageForLine)
#include <fstream>	//file stream
#include "WaveMultiplier.cpp"
using namespace std;

const int numBitsInByte = 8;
const int MAX_AMPLITUDE   = 32767;	//2^15 - 1  (16 bit signed integer, where the 1st bit is a sign bit)  Not sure if this *can* be bigger.
float TOTAL_DURATION_IN_SECONDS = 7;			//Change me! :)
const unsigned int NUM_REPEATED_CYCLES = 0;		//Change me! :)
const unsigned int NUM_TIME_INTERVALS  = 25;	//Change me! :)		Max # of notes (where all notes have the shortest time-length)
const bool DO_READ_FROM_FILE = true;			//Change me! :)


//RIFF chunk
const string CHUNK_ID	  = "RIFF";
const string CHUNK_SIZE	  = "----";	//Placeholder to be reassigned after all audio data is written.
//^^^ CHUNK_SIZE = Size of entire file except the 8 bytes that CHUNK_ID and CHUNK_SIZE take up.
const string AUDIO_FORMAT = "WAVE";	//File Type


//FMT sub-chunk
const string subChunk1_id = "fmt ";	//" " is because the id needs to be 4 bytes long
const int subChunk1_size = 16;		//16 for PCM. Size of the rest of the subchunk following this #
const int audioFormat = 1;			//1 for PCM
const int numChannels = 2;			//1 for monospeaker, 2 for stereo, etc
int samplingRate  = 100000;			//Hertz, how many samples (snippets of a soundwave) are taken per second. Samples/Second
//^^^ More samples==higher fidelity==higher quality sound.		More samples==larger file

const int bitsPerSample  = subChunk1_size;
const int bytesPerSample = bitsPerSample/numBitsInByte;	//What about this truncating/floor()ing ints not divisible by 8?
const int byteRate = numChannels*samplingRate*bytesPerSample;	// Bytes/Second
//^^^ #uniqueSpeakers * samples/second * bytes/sample
const int blockAlign = numChannels*bytesPerSample;		//#bytes for one sample (including all channels)


//Data sub-chunk
const string subChunk2_id   = "data";
const string subChunk2_size = "----";


void WriteAsBytes(ofstream &outputFile, int value, int sizeInBytes)
{
	//pointerToChar addressOfValue = integerAddressOfValue.toCharPointer
	const char* addressOfValue = reinterpret_cast<const char*>(&value);
	outputFile.write( addressOfValue, sizeInBytes );
}





//ENTIRE_AUDIO_LENGTH_SECONDS    samplingRateHz    #OfALLTimeIntervals     baseFrequencyHz     intonationTypeID
//musicalPitchInterval   numTimeIntervalsUsedForCurrNote   waveType   waveDuration_IncreasingOrOn   waveDuration_DecreasingOrOff  waveAmplitudeShift
//musicalPitchInterval   numTimeIntervalsUsedForCurrNote   waveType   waveDuration_IncreasingOrOn   waveDuration_DecreasingOrOff  waveAmplitudeShift\n...
void FrequencyPatternFromTxtToWav(ofstream &wav)
{
	ifstream inFile;
	inFile.open("MyFrequencies.txt");
	if( inFile.fail() ){	return;}
	std::cout << "Opened file \"MyFrequencies.txt\"\n";

	bool debug = true;

	//IDs
	const short SINE_WAVE			   = 0;
	const short BALANCED_SQUARE_WAVE   = 1;
	const short UNEQUAL_SQUARE_WAVE    = 2;
	const short ANGLED_SAWTOOTH_WAVE   = 3;
	const short FORWARD_SAWTOOTH_WAVE  = 4;
	const short BACKWARD_SAWTOOTH_WAVE = 5;
	const short PYTHAGOREAN_TUNING = 0;
	const short FIVE_LIMIT_TUNING  = 1;
	const short EQUAL_TEMPERAMENT  = 2;

	//HEADER INFORMATION
	double /*currTimeVal=-1,*/ baseFrequency=-1;
	string idc;
	getline(inFile,idc);	//I don't care about the first three lines because they will be comments
	getline(inFile,idc);
	getline(inFile,idc);

	//E.G. sample#69, 167400TotalSamples,                  44100SamplesPerSecond,  440Hz
	//E.G. .028seconds,       4.0seconds,                  44100SamplesPerSecond,  440Hz
	inFile                 >> TOTAL_DURATION_IN_SECONDS >> samplingRate         >> baseFrequency;
	
	//I DECIDED TO GO BACK TO USING SAMPLES INSTEAD OF TIME INTERVALS BECAUSE IT'S TOO COMPLICATED FOR THE USER TO SET THE MAXIMUM TIME
	//  INTERVALS WHEN THEY ALREADY SPECIFY THE MAXIMUM TOTAL AUDIO LENGTH
	//double shortestTI = TOTAL_DURATION_IN_SECONDS*1.0/numTotalTimeIntervals;	//shortestTI is the shortest usable time interval (since I refuse to multiply by any #<1)
	
	if(debug){std::cout << TOTAL_DURATION_IN_SECONDS<<" sec|"<<samplingRate<<" samples/second (Hz)|"<<baseFrequency<<" cycles/second (Hz)\n";}

	//DATA LOOP
	//E.G. C2 flute    quarter note,  D2 electronic half note,     F#2 electronic                half note,  G2 electronic                  quarter note
	//E.G. C2 sine for 22050Samples,  D2 square for 44100Samples,  F#2 square(on30,off50) for 44100Samples,  G2 sawtooth(20incr,40decr) for 22050Samples

	//2 CHOICES FOR FOR() LOOP  VVVVV
	//TIME BASED LOOP:		timeVal+=(1.0/samplingRate)
	//At 3 samplesPerSecond, timeVal is incremented by (1/3)=~=.333 seconds.		At samplingRate=5, timeVal is incremented by (1/5)==.2 seconds.
	//SAMPLE BASED LOOP:	sampleNum++
	//sampleNum/samplingRate == currSample#/#samplesPerSecond == currSecond  =  currTimeVal
	//while(endOfFile hasNotBeenReached AND writtenSamples<#totalSamples)
	int i=4;
	long numTotalSamples = samplingRate*TOTAL_DURATION_IN_SECONDS,   usedUpSamples=0;
	float waveAmpShift=0;
	while( !inFile.eof() && usedUpSamples<numTotalSamples)
	{
		if(debug){std::cout << "\nOn inputFile line"<<++i<<". "<<samplingRate<<" samples/second (Hz)| #usedUpSamples: "<<usedUpSamples<<". #totalSamples: "<<numTotalSamples<<"\n";}
		//DEFAULTS
		short intonationTypeID=-1, musicalPitchInterval=0/*xMultipliedByBaseFrequency*/;
		short waveType=SINE_WAVE, waveArg1=-1/*#samplesOnOrIncrPerCycle*/, waveArg2=-1/*#samplesOffOrDecrPerCycle*/;
		int numSamplesUsedForCurrNote = 1;	//NoteLength_Samples
		

		//		  Pythagorean         5th						28							EqualSquareWave,    15Samples,     -1IRRELEVANT                      
		//		  EqualTemperament    11th						56							AngledSawtoothWave, 15SamplesIncr, 25SamplesDecr
		inFile >> intonationTypeID >> musicalPitchInterval >> numSamplesUsedForCurrNote >> waveType         >> waveArg1    >> waveArg2;
		
		if(debug){std::cout<<" "<<intonationTypeID<<" tuningSystem  |  "<<musicalPitchInterval-1<<" semitones  |  ";}
		WaveMultiplier w;
		double tuningRatio = 0;
			 if(intonationTypeID==PYTHAGOREAN_TUNING){tuningRatio = w.GetPythagFreqMultiplier(musicalPitchInterval);			if(debug){std::cout<<"Pythag";}}
		else if(intonationTypeID==FIVE_LIMIT_TUNING){ tuningRatio = w.Get5LimitFreqMultiplier(musicalPitchInterval);			if(debug){std::cout<<"5Limit";}}
		else if(intonationTypeID==EQUAL_TEMPERAMENT){ tuningRatio = w.GetEqualTemperamentFreqMultiplier(musicalPitchInterval);	if(debug){std::cout<<"EquTmp";}}
		if(debug){std::cout << "("<<musicalPitchInterval-1<<")="<<tuningRatio<<"x   | ";}
		if(waveType!=SINE_WAVE)
		{
			waveArg1 = waveArg1/tuningRatio;	//Increase the frequency (by decreasing waveArg1, which is the time each sample spends A.on  or B.increasing or C.per cycle)
			waveArg2 = waveArg2/tuningRatio;	//Increase the frequency (by decreasing waveArg2, which is the time each sample spends A.off or B.decreasing)
		}
		usedUpSamples += numSamplesUsedForCurrNote;	//Update usedUpSamples
		if(debug){std::cout<<numSamplesUsedForCurrNote<<" samplesUsedForCurrNote\n "
		  <<waveType<<" 0sine/1,2sqr/3,4,5saw | "<<waveArg1<<" MODIFIED on/incr/cycleLength | "<<waveArg2<<" MODIFIED off/decr | "<<waveAmpShift<<" shiftInAmplitude\n";}



		//E.G. if(currTime==.13sec < .25sec*30TimeLengths){ outputToFile Amplitude_AtCurrTime;}
		//const double timeUsedForCurrNote = shortestTI*numTimeIntervalsUsedForCurrNote;												DEPRECATED
		//const long numSamplesUsedForCurrNote = long(timeUsedForCurrNote*samplingRate);	//xSeconds * ySamples/second = x*ySamples	DEPRECATED
		//const double timeUsedForCurrNote = numSamplesUsedForCurrNote/samplingRate;	//samples / (samples/sec) == seconds			DEPRECATED
		if(debug){cout << "samplingRate "<<samplingRate<<"|#SamplesUsedForCurrNote "<<numSamplesUsedForCurrNote<<"\n";}

		//Fix unsmooth amplitude transitions between sine waves of different frequencies (unsmoothness produces an audible "thunk")
		double lastAmplitudeOfCurrWave  = w.SineMultiplier(numSamplesUsedForCurrNote*tuningRatio, baseFrequency*waveArg1, waveAmpShift);
		double firstAmplitudeOfNextWave = waveAmpShift;		//sine(sample0*blah)==0, so I ignore the function, only including waveAmpShift
		//Too much vertical offset causes underflow and/or overflow and produces totally new (not bad) sounds
		//E.G. if(sampleNum==#39 < 5000samples){ outputToFile Amplitude_AtCurrTime;}
		for(long sampleNum=0; sampleNum<numSamplesUsedForCurrNote; sampleNum++)
		{
			bool Dbug = debug&&i<5;//&&(sampleNum%9999==0);	//Speeds up the execution tremendously
			if(Dbug){std::cout << "  sample# "<<sampleNum<<"/"<<numSamplesUsedForCurrNote;}
			//DEPRECATED   currTimeVal = (double)(sampleNum)/samplingRate;	//NOTE: This is NOT the global time value, this is the time value of the current note

			double sineWaveAmp=0, squareWaveAmp=0, sawtoothWaveAmp=0;
				 if(waveType==SINE_WAVE				){sineWaveAmp     = w.SineMultiplier(			sampleNum*tuningRatio, baseFrequency*waveArg1, waveAmpShift);	if(Dbug){std::cout<<"|sine="<<sineWaveAmp;}}
			else if(waveType==BALANCED_SQUARE_WAVE  ){squareWaveAmp   = w.BalancedSquareMultiplier(	sampleNum,waveArg1<<1/*x2*/, waveAmpShift);		if(Dbug){std::cout<<"|bSqr="<<squareWaveAmp;}}
			else if(waveType==UNEQUAL_SQUARE_WAVE   ){squareWaveAmp   = w.SquareMultiplier(			sampleNum,waveArg1,waveArg2, waveAmpShift);		if(Dbug){std::cout<<"|Sqr="<<squareWaveAmp;}}
			else if(waveType==ANGLED_SAWTOOTH_WAVE  ){sawtoothWaveAmp = w.AngledSawtoothMultiplier(	sampleNum,waveArg1,waveArg2, waveAmpShift);		if(Dbug){std::cout<<"|Saw="<<sawtoothWaveAmp;}}
			else if(waveType==FORWARD_SAWTOOTH_WAVE ){sawtoothWaveAmp = w.FSawtoothMultiplier(		sampleNum,waveArg1,waveAmpShift);				if(Dbug){std::cout<<"|FSaw="<<sawtoothWaveAmp;}}
			else if(waveType==BACKWARD_SAWTOOTH_WAVE){sawtoothWaveAmp = w.BSawtoothMultiplier(		sampleNum,waveArg2,waveAmpShift);				if(Dbug){std::cout<<"|BSaw="<<sawtoothWaveAmp;}}

			
		
			double finalWaveAmplitude = tuningRatio*baseFrequency/*times waveTypeModifier, which is modified to be one of either sineWaveAmp, squareWaveAmp, or sawtoothWaveAmp*/;
			if(waveType==SINE_WAVE)
				{finalWaveAmplitude *= sineWaveAmp;}
			else if(waveType==BALANCED_SQUARE_WAVE || waveType==UNEQUAL_SQUARE_WAVE)
				{finalWaveAmplitude *= squareWaveAmp;}
			else if(waveType==ANGLED_SAWTOOTH_WAVE || waveType==FORWARD_SAWTOOTH_WAVE || waveType==FORWARD_SAWTOOTH_WAVE)
				{finalWaveAmplitude *= sawtoothWaveAmp;}

			//OVERWRITE TO TEST THE FUNCTIONS
			//finalWaveAmplitude = w.FrequencyPattern_ElectronicDeathMetal(sampleNum,baseFrequency,waveArg1,tuningRatio,waveAmpShift);
			
			finalWaveAmplitude = w.SineMultiplier(sampleNum*tuningRatio, baseFrequency*waveArg1, waveAmpShift);
			//finalWaveAmplitude = w.SumOfWavesFreqMultipliers(sampleNum*tuningRatio, baseFrequency*waveArg1, waveArg1<<7, waveAmpShift);
			finalWaveAmplitude *= MAX_AMPLITUDE>>1/*divided by 2*/;

			if(sampleNum==numSamplesUsedForCurrNote-1)
				{waveAmpShift/*forNextWaveFrequency*/ = lastAmplitudeOfCurrWave-firstAmplitudeOfNextWave;}

			if(Dbug){std::cout << "|finalWaveAmplitude="<<finalWaveAmplitude;}
			WriteAsBytes(wav, finalWaveAmplitude/*channel1*/, 2);
			if(numChannels==2)
				{WriteAsBytes(wav, finalWaveAmplitude/*channel2*/, 2);	if(Dbug){std::cout << "|2channelz"<<endl;}}
		}
	}
}






/*  WAVE FILE HEADER (but eliminate the spaces and newlines because binary data doesn't have either)
CHUNK_ID  CHUNK_SIZE  AUDIO_FORMAT
subChunk1_id  subChunk1_size  numChannels  samplingRate  byteRate  blockAlign  bitsPerSample
subChunk2_id  subChunk2_size
*/
int main()
{
	ofstream wav;
	wav.open("AudioFile.wav", ios::binary);
	bool deboog = true;

	if(wav.is_open())
	{
		wav << CHUNK_ID/*4bytes*/ << CHUNK_SIZE/*4bytes*/ << AUDIO_FORMAT/*4bytes*/  <<  subChunk1_id/*4bytes*/;
		WriteAsBytes(wav, subChunk1_size, 4);
		WriteAsBytes(wav, audioFormat,    2);
		WriteAsBytes(wav, numChannels,    2);
		WriteAsBytes(wav, samplingRate,   4);	//!!! CHANGED BY FILE READ
		WriteAsBytes(wav, byteRate,       4);	//!!! CHANGED BY FILE READ
		WriteAsBytes(wav, blockAlign,     2);
		WriteAsBytes(wav, bitsPerSample,  2);
		wav << subChunk2_id << subChunk2_size;
		
		long startAudioDataWrite_bitIndex = wav.tellp();
		const unsigned int numCycles = NUM_REPEATED_CYCLES+1;
		const unsigned long numTotalSamples = samplingRate*TOTAL_DURATION_IN_SECONDS;	//Samples/second * #seconds
		//^^^ E.g. samplingRate=500samples/second:  (500deg/second)*5seconds==2500deg  ==  2500samples
		if(DO_READ_FROM_FILE)
		{
			FrequencyPatternFromTxtToWav(wav);
			wav.seekp( 24, ios::beg );	//GO TO bitIndex==beginningIndex+24bytes==samplingRate
			WriteAsBytes(wav, samplingRate, 4);	//In 4 bytes (COULD GET BAD WITH long), overwrite default samplingRate to file-specified samplingRate
			WriteAsBytes(wav, byteRate,     4);	//In 4 bytes (COULD GET BAD WITH long), overwrite default byteRate     to file-specified byteRate
		}
		else
		{
			WaveMultiplier w;
			//currSample acts like the x value on the x-axis, the (degree of rotation)position out of all cycles (without modulo).
			//E.g. currSample=600deg (out of 2500deg(5 cycles of 500deg/cycle))   means   currSample==1.2cycles (out of the total 5 cycles)
			for(unsigned long currSample=0; currSample<numTotalSamples; currSample++)
			{
				const double position_time_InSomeCycle = (double)currSample/samplingRate;	//E.g. 390deg / (360deg/second)  ==  1.08333 seconds
				const double percentTimeElapsedOfTotal = position_time_InSomeCycle/TOTAL_DURATION_IN_SECONDS;	//1.08333 seconds/5 seconds  ==  timeElapsed: 21.7%
				const double percentTimeElapsedOfCurrCycle = percentTimeElapsedOfTotal/numCycles;
				//^^^ /durationInSeconds so MAX_AMPLITUDE isn't reached in 1 second but rather at (durationInSeconds/#Oscillations)

				double frequency_hz = w.FrequencyPattern1(position_time_InSomeCycle, TOTAL_DURATION_IN_SECONDS, NUM_TIME_INTERVALS, 288);
				//^^^ Frequency in Hertz (FullOscillations/Second)   Oscillation==Cycle

				
				double waveAmplitude =  percentTimeElapsedOfCurrCycle*MAX_AMPLITUDE;
				double sineWave     = w.SineMultiplier(position_time_InSomeCycle, frequency_hz, -.5);
				//^^^   sin(2*Pi*(xCycles/sec) * scaledToOneOrLess_sampleX)
				double squareWave   = w.SquareMultiplier(currSample,frequency_hz,frequency_hz, -.5);
				double sawtoothWave = w.AngledSawtoothMultiplier(currSample,frequency_hz,frequency_hz, -.5);
				//Too much vertical offset causes underflow and/or overflow and produces totally new sounds


				//double channel1_finalAmplitude = waveAmplitude*sineWave;
				double channel2_finalAmplitude = (MAX_AMPLITUDE-waveAmplitude)*sineWave;
				//remainderOfAmplitude*val  E.g. MAX_AMPLITUDE=100, chnl1=70*sineWave,chnl2=30*sineWave

				//double channel1_finalAmplitude = waveAmplitude*squareWave;
				//double channel2_finalAmplitude = (MAX_AMPLITUDE-waveAmplitude)*squareWave;
				//remainderOfAmplitude*val  E.g. MAX_AMPLITUDE=100, chnl1=70*squareWave,chnl2=30*squareWave

				double channel1_finalAmplitude = waveAmplitude*sawtoothWave;
				//double channel2_finalAmplitude = (MAX_AMPLITUDE-waveAmplitude)*sawtoothWave;
				//remainderOfAmplitude*val  E.g. MAX_AMPLITUDE=100, chnl1=70*sawtoothWave,chnl2=30*sawtoothWave

				if(deboog){cout << "currSample "<<currSample<<"|chnl1:"<<channel1_finalAmplitude<<"~chnl2:"<<channel2_finalAmplitude<<"\n";}
				WriteAsBytes(wav, channel1_finalAmplitude, 2);
				WriteAsBytes(wav, channel2_finalAmplitude, 2);
			}
		}
		long endAudioDataWrite_bitIndex = wav.tellp();	//Gets the index (place) of the current bit in the file named wav
		const long lengthOfAudioData    = endAudioDataWrite_bitIndex-startAudioDataWrite_bitIndex;
		wav.seekp( startAudioDataWrite_bitIndex-4 );	//GO TO bitIndex==startAudioDataWrite_index-4bytes
		WriteAsBytes(wav, lengthOfAudioData, 4);		//In 4 bytes (COULD GET BAD WITH long), overwrite subChunk2_size from "----" to audioSizeInBytes

		wav.seekp(4, ios::beg);		//GO TO bitIndex==beginningOfFile+4bytes
		WriteAsBytes(wav, endAudioDataWrite_bitIndex-8, 4);	//8==sizeOfHeaderInBytes?
		//^^^ In 4 bytes (COULD GET BAD WITH long), overwrite CHUNK_SIZE from "----" to fileSizeInBytes-8bytes
	}
	wav.close();	//Close the output file so further modification is not allowed
}