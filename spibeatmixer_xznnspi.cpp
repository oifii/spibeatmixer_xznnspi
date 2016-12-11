////////////////////////////////////////////////////////////////
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
//xznn.com, xznnworldwide@gmail.com, carl w.b. poirier
//
//2014fev02, creation for mixing samples
//
//
//xznn.com, xznnworldwide@gmail.com, carl w.b. poirier
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <string>
#include <fstream>
#include <vector>

#include <iostream>
#include <sstream>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"

#define BUFF_SIZE	2048


#include <ctime>
#include "spiws_WavSet.h"
#include "spiws_Instrument.h"
#include "spiws_InstrumentSet.h"

#include "spiws_partition.h"
#include "spiws_partitionset.h"

#include "spiws_WavSet.h"

#include <assert.h>
#include <windows.h>



// Select sample format
#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif


//The event signaled when the app should be terminated.
HANDLE g_hTerminateEvent = NULL;
//Handles events that would normally terminate a console application. 
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);

int Terminate();

InstrumentSet* pInstrumentSet=NULL;
InstrumentSet* pInstrumentSet2=NULL;


//////////////////////////////////////////
//main
//////////////////////////////////////////
//#include "Uxtheme.h"

int main(int argc, char *argv[]);
int main(int argc, char *argv[])
{
    //Auto-reset, initially non-signaled event 
    g_hTerminateEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    //Add the break handler
    ::SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

	//ShowWindow( GetConsoleWindow(), SW_HIDE );
	/*
	HWND hWnd=GetConsoleWindow();
	RECT rcScr, rcWnd, rcClient;

	GetWindowRect(hWnd, &rcWnd);
	GetWindowRect(GetDesktopWindow(), &rcScr);
	GetClientRect(hWnd, &rcClient);

	//MoveWindow(hWnd, (rcScr.right / 2) - 330, (rcScr.bottom / 2) - 180, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top, 1);
	SetWindowLong(hWnd, GWL_STYLE, WS_POPUP);
	//SetWindowRgn(hWnd, CreateRectRgn(rcClient.left + 2, rcClient.top + 2, rcClient.right + 2, rcClient.bottom + 2), TRUE);
	ShowWindow(hWnd, 1);
	*/
	HWND hWnd=GetConsoleWindow();
	SetConsoleTitle(L"Test");
    
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hWnd, 0, 200, LWA_ALPHA);
	SetWindowLong(hWnd, GWL_STYLE, (GetWindowLong(hWnd, GWL_STYLE) &~ WS_CAPTION) | WS_POPUP);
	
	/*
     SetWindowLong (hWnd, GWL_STYLE, WS_THICKFRAME);
     SetWindowLong (hWnd, GWL_STYLE, WS_CAPTION);
     SetWindowPos  (hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	 */

	//SetWindowLong(hWnd, GWL_STYLE, (GetWindowLong(hWnd, GWL_STYLE) &~ WS_VSCROLL) | WS_POPUP);
	//ShowScrollBar(hWnd, SB_BOTH, FALSE);
	//UpdateWindow(hWnd);
	//#pragma comment(lib, "UxTheme.lib")
	//if(IsThemeActive()) 
	//{
		//::SetWindowTheme(hWnd, NULL, L"Scrollbar");
	//}
	SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	//UpdateWindow(hWnd);

    HANDLE hOut;
    CONSOLE_SCREEN_BUFFER_INFO SBInfo;
    COORD NewSBSize;
    int Status;
 
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
 
    GetConsoleScreenBufferInfo(hOut, &SBInfo);
    NewSBSize.X = SBInfo.dwSize.X;
    //NewSBSize.Y = SBInfo.dwSize.Y;
    NewSBSize.Y = 5;
 
    Status = SetConsoleScreenBufferSize(hOut, NewSBSize);
	UpdateWindow(hWnd);

	PaStreamParameters outputParameters;
    PaStream* stream;
    PaError err;

	////////////////////////
	// initialize port audio 
	////////////////////////
    err = Pa_Initialize();
    if( err != paNoError )
	{
		fprintf(stderr,"Error: Initialization failed.\n");
		Pa_Terminate();
		fprintf( stderr, "An error occured while using the portaudio stream\n" );
		fprintf( stderr, "Error number: %d\n", err );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		return -1;
	}

	outputParameters.device = Pa_GetDefaultOutputDevice(); // default output device 
	if (outputParameters.device == paNoDevice) 
	{
		fprintf(stderr,"Error: No default output device.\n");
		Pa_Terminate();
		fprintf( stderr, "An error occured while using the portaudio stream\n" );
		fprintf( stderr, "Error number: %d\n", err );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		return -1;
	}
	outputParameters.channelCount = 2;//pWavSet->numChannels;
	outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;


	//////////////////////////
	//initialize random number
	//////////////////////////
	srand((unsigned)time(0));


	/*
	//////////////////////////////
	//play 60sec sinusoidal sample
	//////////////////////////////
	WavSet* pTempWavSet = new WavSet;
	pTempWavSet->CreateSin(60, 44100, 2, 440.0, 0.5f);
	pTempWavSet->Play(&outputParameters);
	if(pTempWavSet) { delete pTempWavSet; pTempWavSet = NULL; }
	*/
	
	
	/////////////////////////////
	//loop n sinusoidal samples
	/////////////////////////////
	WavSet* pTempWavSet = new WavSet;
	pTempWavSet->CreateSin(0.3333, 44100, 2, 440.0, 0.5f);
	WavSet* pSilentWavSet = new WavSet;
	pSilentWavSet->CreateSilence(5);
	pSilentWavSet->LoopSample(pTempWavSet, 5, -1.0, 0.0); //from second 0, loop sample during 5 seconds
	pSilentWavSet->Play(&outputParameters);
	if(pTempWavSet) { delete pTempWavSet; pTempWavSet = NULL; }
	if(pSilentWavSet) { delete pSilentWavSet; pSilentWavSet = NULL; }
	

	

	//////////////////////////////////////////////////////////////////////////
	//populate InstrumentSet according to input folder (folder of sound files)
	//////////////////////////////////////////////////////////////////////////
	//string wavfolder = "D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\raveaudio_wav\\dj-oifii_minimal-deep-electro-house-techno";
	string wavfolder = "D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\xznnspi\\samples_fbm";
	//string wavfolder = "D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\raveaudio_wav\\dj-oifii_ibiza";

	//WavSet* pFBM3Bars = new WavSet;
	//pFBM3Bars->ReadWavFile("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\xznnspi\\samples_fbm\\fbm_3bars.wav");
	WavSet* pFBM4Bars = new WavSet;
	pFBM4Bars->ReadWavFile("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\xznnspi\\samples_fbm\\fbm_4bars.wav"); //done
	WavSet* pFBM3ClavesBars = new WavSet;
	pFBM3ClavesBars->ReadWavFile("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\xznnspi\\samples_fbm\\fbm-claves_3bars.wav");
	WavSet* pFBMHH2Bars = new WavSet;
	pFBMHH2Bars->ReadWavFile("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\xznnspi\\samples_fbm\\fbm-hh_2bars.wav"); //done
	WavSet* pFBMLow1Beat = new WavSet;
	pFBMLow1Beat->ReadWavFile("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\xznnspi\\samples_fbm\\fbm-low_1beat.wav"); //done
	WavSet* pFBMLow2Bars = new WavSet;
	pFBMLow2Bars->ReadWavFile("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\xznnspi\\samples_fbm\\fbm-low_2bars.wav"); //done
	WavSet* pFBMOff1Bar = new WavSet;
	pFBMOff1Bar->ReadWavFile("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\xznnspi\\samples_fbm\\fbm-off_1bar.wav"); //done
	WavSet* pFBMSnare1Bar = new WavSet;
	pFBMSnare1Bar->ReadWavFile("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\xznnspi\\samples_fbm\\fbm-snare_1bar.wav"); //done

	WavSet* pPlayaWide = new WavSet;
	pPlayaWide->ReadWavFile("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\xznnspi\\samples_playa\\playa_wide(distortionless).wav"); //done

	WavSet* pBeanBrotherChat = new WavSet;
	pBeanBrotherChat->ReadWavFile("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\xznnspi\\samples_speech\\beanbrotherchat.wav"); //done

	/*
	InstrumentSet* pInstrumentSet=new InstrumentSet;
	InstrumentSet* pInstrumentSet2=new InstrumentSet;
	*/
	pInstrumentSet=new InstrumentSet;
	//pInstrumentSet2=new InstrumentSet;
	//InstrumentSet* pInstrumentSet3=new InstrumentSet;
	//if(pInstrumentSet!=NULL && pInstrumentSet2!=NULL && pInstrumentSet3!=NULL)
	//if(pInstrumentSet!=NULL && pInstrumentSet2!=NULL)
	if(pInstrumentSet!=NULL)
	{
		pInstrumentSet->Populate(wavfolder.c_str());
		//pInstrumentSet2->Populate(wavfolder2.c_str());
		//pInstrumentSet3->Populate(wavfolder3.c_str());
	}
	else
	{
		assert(false);
		cout << "exiting, instrumentset could not be allocated" << endl;
		Pa_Terminate();
		return -1;
	}
		

	/*
	///////////////////////////////////
	//play all notes of all instruments
	///////////////////////////////////
	//int numberofinstrumentsinplayback=3;
	int numberofinstrumentsinplayback=1; //one instrument at a time
	float fSecondsPlay = 600.0f;
	int iCONCATENATEATTACKSflag = 0; //0 false, 1 true
	pInstrumentSet->Play(&outputParameters, fSecondsPlay, numberofinstrumentsinplayback, iCONCATENATEATTACKSflag); //each instrument will play its loaded samples sequentially
	
	int dummy=0;
	*/


	//////////////////////////////////////////////////////////////////////////////////////////////
	//pick random instrument, pick random wavset, loop wavset for loopduration_s seconds and play,
	//then repeat all of the above 100 times
	//////////////////////////////////////////////////////////////////////////////////////////////
	if(0)
	{
		Instrument* pAnInstrument = NULL;
		WavSet* pAWavSet = NULL;
		for(int i=0; i<100; i++) //repeat 100 times
		{
			if(pInstrumentSet && pInstrumentSet->HasOneInstrument()) 
			{
				//vector<Instrument*>::iterator it;
				//for(it=pInstrumentSet->instrumentvector.begin();it<pInstrumentSet->instrumentvector.end();it++)
				//{
				
					cout << endl;
					//pAnInstrument = *it;
					pAnInstrument = pInstrumentSet->GetInstrumentRandomly();
					assert(pAnInstrument);
					cout << "instrument name: " << pAnInstrument->instrumentname << endl;
					pAWavSet = pAnInstrument->GetWavSetRandomly();
					cout << "sound filename: " << pAWavSet->GetName() << endl;
					assert(pAWavSet);
					//pAWavSet->Play(&outputParameters);

					/*
					float loopduration_s = 2 * pAWavSet->GetWavSetLength() + 0.050f; //0.050 sec to ensure loopduration_s is larger than sample
					//WavSet* pSilentWavSet = new WavSet;
					pSilentWavSet = new WavSet;
					pSilentWavSet->CreateSilence(loopduration_s); 
					pSilentWavSet->LoopSample(pAWavSet, loopduration_s, -1.0, 0.0); //from second 0, loop sample during loopduration_s seconds
					//pSilentWavSet->Play(&outputParameters);
					*/
					WavSet* pSilentWavSet = new WavSet;
					while(pSilentWavSet->GetWavSetLength()<10.0)
					{
						if(pSilentWavSet->GetWavSetLength()==0.0)
						{
							//copy
							pSilentWavSet->Copy(pAWavSet);
						}
						else
						{
							//concatenate
							pSilentWavSet->Concatenate(pAWavSet);
						}
					}
				
					////////////
					//play
					////////////
					pSilentWavSet->Play(&outputParameters);
				
					//pSilentWavSet->Erase();
					//pSilentWavSet->Play(&outputParameters);
					if(pSilentWavSet)
					{
						delete pSilentWavSet;
						pSilentWavSet = NULL;
					}
				//}
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	//pick random instrument, pick random wavset, loop wavset for loopduration_s seconds and play,
	//then repeat all of the above 100 times
	//////////////////////////////////////////////////////////////////////////////////////////////
	if(1)
	{
		WavSet* pFinalSilentWavSet1 = new WavSet;
		pFinalSilentWavSet1->CreateSilence(0.001f); //1ms initial length so we can concatenate later

		cout << endl;
		cout << endl;
		cout << "*****************************************************" << endl;
		cout << "creating track 1, with a random alternance of samples" << endl;
		cout << "*****************************************************" << endl;
		cout << endl;

		float barduration_s = pFBMOff1Bar->GetWavSetLength();
		Instrument* pAnInstrument = NULL;
		WavSet* pAWavSet = NULL;
		for(int i=0; i<100; i++) //repeat 100 times
		{
			if(pInstrumentSet && pInstrumentSet->HasOneInstrument()) 
			{
				//vector<Instrument*>::iterator it;
				//for(it=pInstrumentSet->instrumentvector.begin();it<pInstrumentSet->instrumentvector.end();it++)
				//{
				
					cout << endl;
					//pAnInstrument = *it;
					pAnInstrument = pInstrumentSet->GetInstrumentRandomly();
					assert(pAnInstrument);
					cout << "instrument name: " << pAnInstrument->instrumentname << endl;
					pAWavSet = pAnInstrument->GetWavSetRandomly();
					cout << "sound filename: " << pAWavSet->GetName() << endl;
					assert(pAWavSet);
					//pAWavSet->Play(&outputParameters);

					/*
					float loopduration_s = 2 * pAWavSet->GetWavSetLength() + 0.050f; //0.050 sec to ensure loopduration_s is larger than sample
					//WavSet* pSilentWavSet = new WavSet;
					pSilentWavSet = new WavSet;
					pSilentWavSet->CreateSilence(loopduration_s); 
					pSilentWavSet->LoopSample(pAWavSet, loopduration_s, -1.0, 0.0); //from second 0, loop sample during loopduration_s seconds
					//pSilentWavSet->Play(&outputParameters);
					*/
					int random_integer;
					int lowest=1, highest=10;
					int range=(highest-lowest)+1;
					random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));
					float random_float = (float) random_integer;
					//float loopduration_s = 10.0f;
					//float loopduration_s = random_float*barduration_s;
					float loopduration_s = random_float;
					cout << "loop duration in sec: " << loopduration_s << endl;
					WavSet* pSilentWavSet = new WavSet;
					while(pSilentWavSet->GetWavSetLength()<loopduration_s)
					{
						if(pSilentWavSet->GetWavSetLength()==0.0)
						{
							//copy
							pSilentWavSet->Copy(pAWavSet);
						}
						else
						{
							//concatenate
							pSilentWavSet->Concatenate(pAWavSet);
						}
					}
				
					//////
					//play
					//////
					//pSilentWavSet->Play(&outputParameters);

					//////////////////////////////
					//concatenate into final ouput
					//////////////////////////////
					pFinalSilentWavSet1->Concatenate(pSilentWavSet);

					//pSilentWavSet->Erase();
					//pSilentWavSet->Play(&outputParameters);
					if(pSilentWavSet) { delete pSilentWavSet; pSilentWavSet = NULL; }
				//}
			}
		}

		pFinalSilentWavSet1->WriteWavFile("testoutput-random.wav");
		//pFinalSilentWavSet1->Play(&outputParameters);

		float trackduration_s = pFinalSilentWavSet1->GetWavSetLength();
		cout << endl;
		cout << "track 1 duration (in sec): " << trackduration_s << endl;

		WavSet* pFinalSilentWavSet12 = new WavSet;
		if(0)
		{
			//float barduration_s = pFBMOff1Bar->GetWavSetLength();
			cout << endl;
			cout << endl;
			cout << "*********************************************************" << endl;
			cout << "creating track 2, with an alternance of silence and playa" << endl; //playa is the name of the ambient sample
			cout << "*********************************************************" << endl;
			cout << endl;
			WavSet* pFinalSilentWavSet2 = new WavSet;
			pFinalSilentWavSet2->CreateSilence(0.001f); //1ms initial length so we can concatenate later
			bool playaflag=0;
			while(pFinalSilentWavSet2->GetWavSetLength()<trackduration_s)
			{
				int random_integer;
				int lowest=3, highest=10;
				int range=(highest-lowest)+1;
				random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));
				float random_float = (float)random_integer;

				float segmentduration_s = random_float*barduration_s;

				if(playaflag==0) playaflag=1;
				  else playaflag=0;

				WavSet* pSilentWavSet2 = new WavSet;
				if(playaflag==0)
				{
					pSilentWavSet2->CreateSilence(segmentduration_s);
					cout << "silence duration in sec: " << segmentduration_s << endl;
				}
				else
				{
					pSilentWavSet2->Copy(pPlayaWide, segmentduration_s, 0.0f);
					cout << "playa duration in sec: " << segmentduration_s << endl;
				}
				//concatenate
				pFinalSilentWavSet2->Concatenate(pSilentWavSet2);
				if(pSilentWavSet2) { delete pSilentWavSet2; pSilentWavSet2 = NULL; }
			}

			cout << endl;
			cout << "track 2 duration (in sec): " << pFinalSilentWavSet2->GetWavSetLength() << endl;

			pFinalSilentWavSet12->Mix(0.8, pFinalSilentWavSet1, 0.5, pFinalSilentWavSet2);
			if(pFinalSilentWavSet1) { delete pFinalSilentWavSet1; pFinalSilentWavSet1=NULL; }
			if(pFinalSilentWavSet2) { delete pFinalSilentWavSet2; pFinalSilentWavSet2=NULL; }
			//pFinalSilentWavSet12->WriteWavFile("testoutput-random.wav");
			//pFinalSilentWavSet->Play(&outputParameters);
		}

		if(0)
		{
			cout << endl;
			cout << endl;
			cout << "**********************************************************" << endl;
			cout << "creating track 3, with an alternance of silence and speech" << endl; //speech from bean brother chat
			cout << "**********************************************************" << endl;
			cout << endl;
			WavSet* pFinalSilentWavSet3 = new WavSet;
			pFinalSilentWavSet3->CreateSilence(0.001f); //1ms initial length so we can concatenate later
			bool speechflag=0;
			while(pFinalSilentWavSet3->GetWavSetLength()<trackduration_s)
			{
				int random_integer;
				int lowest=10, highest=30;
				int range=(highest-lowest)+1;
				random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));
				float random_float = (float)random_integer;

				float segmentduration_s = random_float*barduration_s;

				if(speechflag==0) speechflag=1;
				  else speechflag=0;

				WavSet* pSilentWavSet3 = new WavSet;
				if(speechflag==0)
				{
					pSilentWavSet3->CreateSilence(segmentduration_s);
					cout << "silence duration in sec: " << segmentduration_s << endl;
				}
				else
				{
					pSilentWavSet3->Copy(pBeanBrotherChat, segmentduration_s, 0.0f);
					cout << "speech duration in sec: " << segmentduration_s << endl;
				}
				//concatenate
				pFinalSilentWavSet3->Concatenate(pSilentWavSet3);
				if(pSilentWavSet3) { delete pSilentWavSet3; pSilentWavSet3 = NULL; }
			}

			cout << endl;
			cout << "track 3 duration (in sec): " << pFinalSilentWavSet3->GetWavSetLength() << endl;

			WavSet* pFinalSilentWavSet123 = new WavSet;
			pFinalSilentWavSet123->Mix(0.8, pFinalSilentWavSet12, 0.5, pFinalSilentWavSet3);
			if(pFinalSilentWavSet12) { delete pFinalSilentWavSet12; pFinalSilentWavSet12=NULL; }
			if(pFinalSilentWavSet3) { delete pFinalSilentWavSet3; pFinalSilentWavSet3=NULL; }
			pFinalSilentWavSet123->WriteWavFile("testoutput-random.wav");
			//pFinalSilentWavSet->Play(&outputParameters);

			if(pFinalSilentWavSet123) { delete pFinalSilentWavSet123; pFinalSilentWavSet123=NULL; }
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//for each wavset, loop wavset for loopduration_s seconds (i.e. 16 bars) into a segment called pSilentWavSetX
	//then concatenate these pSilentWavSetX and save output wav to file
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if(0)
	{
		float extralength_s = 0.001f; //1 ms extra
		//////////////////
		//loop a few times
		//////////////////
		float loopduration_s = 4 * pFBM4Bars->GetWavSetLength() + extralength_s; //extralength_s sec to ensure loopduration_s is larger than sample
		WavSet* pSilentWavSet = new WavSet;
		pSilentWavSet->CreateSilence(loopduration_s); 
		pSilentWavSet->LoopSample(pFBM4Bars, loopduration_s, -1.0, 0.0); //from second 0, loop sample during loopduration_s seconds
		//pSilentWavSet->Play(&outputParameters);
		//////////////////
		//loop a few times
		//////////////////
		loopduration_s = 8 * pFBMHH2Bars->GetWavSetLength() + extralength_s; //extralength_s sec to ensure loopduration_s is larger than sample
		WavSet* pSilentWavSet1 = new WavSet;
		pSilentWavSet1->CreateSilence(loopduration_s); 
		pSilentWavSet1->LoopSample(pFBMHH2Bars, loopduration_s, -1.0, 0.0); //from second 0, loop sample during loopduration_s seconds
		//pSilentWavSet1->Play(&outputParameters);
		//////////////////
		//loop a few times
		//////////////////
		loopduration_s = 16 * pFBMSnare1Bar->GetWavSetLength() + extralength_s; //extralength_s sec to ensure loopduration_s is larger than sample
		WavSet* pSilentWavSet2 = new WavSet;
		pSilentWavSet2->CreateSilence(loopduration_s); 
		pSilentWavSet2->LoopSample(pFBMSnare1Bar, loopduration_s, -1.0, 0.0); //from second 0, loop sample during loopduration_s seconds
		//pSilentWavSet2->Play(&outputParameters);
		//////////////////
		//loop a few times
		//////////////////
		loopduration_s = 8 * pFBMLow2Bars->GetWavSetLength() + extralength_s; //extralength_s sec to ensure loopduration_s is larger than sample
		WavSet* pSilentWavSet3 = new WavSet;
		pSilentWavSet3->CreateSilence(loopduration_s); 
		pSilentWavSet3->LoopSample(pFBMLow2Bars, loopduration_s, -1.0, 0.0); //from second 0, loop sample during loopduration_s seconds
		//pSilentWavSet3->Play(&outputParameters);
		//////////////////
		//loop a few times
		//////////////////
		loopduration_s = 16 * pFBMOff1Bar->GetWavSetLength() + extralength_s; //extralength_s sec to ensure loopduration_s is larger than sample
		WavSet* pSilentWavSet4 = new WavSet;
		pSilentWavSet4->CreateSilence(loopduration_s); 
		pSilentWavSet4->LoopSample(pFBMOff1Bar, loopduration_s, -1.0, 0.0); //from second 0, loop sample during loopduration_s seconds
		//pSilentWavSet4->Play(&outputParameters);
		//////////////////
		//loop a few times
		//////////////////
		loopduration_s = 16 * 4 * pFBMLow1Beat->GetWavSetLength() + extralength_s; //extralength_s sec to ensure loopduration_s is larger than sample
		WavSet* pSilentWavSet5 = new WavSet;
		pSilentWavSet5->CreateSilence(loopduration_s); 
		pSilentWavSet5->LoopSample(pFBMLow1Beat, loopduration_s, -1.0, 0.0); //from second 0, loop sample during loopduration_s seconds
		//pSilentWavSet5->Play(&outputParameters);


		///////////////////////////////////////////////////////
		//concatenate all beat segments, to make the beat track
		///////////////////////////////////////////////////////
		pSilentWavSet->Concatenate(pSilentWavSet1);
		pSilentWavSet->Concatenate(pSilentWavSet2);
		pSilentWavSet->Concatenate(pSilentWavSet3);
		pSilentWavSet->Concatenate(pSilentWavSet4);
		pSilentWavSet->Concatenate(pSilentWavSet5);
		//pSilentWavSet->Play(&outputParameters);
		pSilentWavSet->WriteWavFile("testoutput.wav");

		if(pSilentWavSet) { delete pSilentWavSet; pSilentWavSet=NULL; }
		if(pSilentWavSet1) { delete pSilentWavSet1; pSilentWavSet1=NULL; }
		if(pSilentWavSet2) { delete pSilentWavSet2; pSilentWavSet2=NULL; }
		if(pSilentWavSet3) { delete pSilentWavSet3; pSilentWavSet3=NULL; }
		if(pSilentWavSet4) { delete pSilentWavSet4; pSilentWavSet4=NULL; }
		if(pSilentWavSet5) { delete pSilentWavSet5; pSilentWavSet5=NULL; }
	}


	//if(pFBM3Bars) delete pFBM3Bars;
	if(pFBM4Bars) { delete pFBM4Bars; pFBM4Bars=NULL; }
	if(pFBM3ClavesBars) { delete pFBM3ClavesBars; pFBM3ClavesBars=NULL; }
	if(pFBMHH2Bars) { delete pFBMHH2Bars; pFBMHH2Bars=NULL; }
	if(pFBMLow1Beat) { delete pFBMLow1Beat; pFBMLow1Beat=NULL; }
	if(pFBMLow2Bars) { delete pFBMLow2Bars; pFBMLow2Bars=NULL; }
	if(pFBMOff1Bar) { delete pFBMOff1Bar; pFBMOff1Bar=NULL; }
	if(pFBMSnare1Bar) { delete pFBMSnare1Bar; pFBMSnare1Bar=NULL; }

	if(pPlayaWide) { delete pPlayaWide; pPlayaWide=NULL; }
	if(pBeanBrotherChat) { delete pBeanBrotherChat; pBeanBrotherChat=NULL; }
	
	/*
	/////////////////////
	//terminate portaudio
	/////////////////////
	Pa_Terminate();
	//if(pInstrumentSet) delete pInstrumentSet;
	printf("Exiting!\n"); fflush(stdout);
	*/
	Terminate();
	return 0;
}

int Terminate()
{
	/////////////////////
	//terminate portaudio
	/////////////////////
	Pa_Terminate();

	//delete all memory allocations
	if(pInstrumentSet!=NULL) delete pInstrumentSet;
	if(pInstrumentSet2!=NULL) delete pInstrumentSet2;

	printf("Exiting!\n"); fflush(stdout);
	return 0;
}

//Called by the operating system in a separate thread to handle an app-terminating event. 
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT ||
        dwCtrlType == CTRL_BREAK_EVENT ||
        dwCtrlType == CTRL_CLOSE_EVENT)
    {
        // CTRL_C_EVENT - Ctrl+C was pressed 
        // CTRL_BREAK_EVENT - Ctrl+Break was pressed 
        // CTRL_CLOSE_EVENT - Console window was closed 
		Terminate();
        // Tell the main thread to exit the app 
        ::SetEvent(g_hTerminateEvent);
        return TRUE;
    }

    //Not an event handled by this function.
    //The only events that should be able to
	//reach this line of code are events that
    //should only be sent to services. 
    return FALSE;
}

