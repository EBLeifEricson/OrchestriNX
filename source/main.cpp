#include <cstring>
#include <cstdio>
#include <ctime>
#include <dirent.h>
#include <string>
#include <sstream>
#include <switch.h>

extern "C"
{
  #include "draw.h"
}

#include "audio.h"

#define DEFAULTSAMPLERATE 22050
#define BYTESPERSAMPLE 4

#define INSTRUMENTCOUNT 10
#define SONGCOUNT 38

#define MAXNOTES 8
#define MAXSONGSPERINSTRUMENT 24

#define WAVEBUFCOUNT (MAXNOTES+4)
#define SONGWAVEBUF (WAVEBUFCOUNT-1)

#define INST_OCARINA 0
#define INST_PIPES 1
#define INST_DRUMS 2
#define INST_GUITAR 3
#define INST_MALON 4
#define INST_HARP 5
#define INST_FROGS 6
#define INST_MUSBOX 7
#define INST_FLUTE 8
#define INST_WAKER 9

using namespace std;

// One wavebuf for each of the current instrument's notes, one for each sound effect, one for the current full song playing
dspWaveBuf waveBuf[WAVEBUFCOUNT];
bool wavebufList[WAVEBUFCOUNT];

// Noteset
enum {
    NOTESET_OOT,
    NOTESET_ST,
    NOTESET_WW
};

typedef struct {
    u8 notes;
    string notestr;
    u32 keys[MAXNOTES];
} Noteset;

// Instrument
typedef struct {
    string name;
    u8 nset;
    u8 songs;
    u16 songlist[MAXSONGSPERINSTRUMENT];
} Instrument;

// Song
typedef struct {
    string name;
    string sequence;
} Song;

// Source
typedef struct {
    u32 nsamples;
    u32 size;
    char* data;
    bool loop;
    int wbuf;
    int channel;
} Source;

Source* notes[MAXNOTES];
Bitmap* nicons[MAXNOTES];

// Array of notesets
Noteset notesets[3] = {
    { 5, "lxyar",  { KEY_L, KEY_X, KEY_Y, KEY_A, KEY_R } },
    { 6, "lxyarb", { KEY_L, KEY_X, KEY_Y, KEY_A, KEY_R, KEY_B } },
    { 5, "ruldn",  { KEY_A, KEY_X, KEY_Y, KEY_B, 0 } }
};

// Array of songs
Song songs[SONGCOUNT] = {
    { "NULL",                   "------"   }, //  0 - OOT/MM
    { "Zelda's Lullaby",        "xayxay"   }, //  1
    { "Saria's Song",           "ryxryx"   }, //  2
    { "Epona's Song",           "axyaxy"   }, //  3
    { "Sun's Song",             "yrayra"   }, //  4
    { "Song of Time",           "ylrylr"   }, //  5
    { "Song of Storms",         "lralra"   }, //  6
    { "Minuet of Forest",       "laxyxy"   }, //  7
    { "Bolero of Fire",         "rlrlyryr" }, //  8
    { "Serenade of Water",      "lryyx"    }, //  9
    { "Nocturne of Shadow",     "xyylxyr"  }, // 10
    { "Requiem of Spirit",      "lrlyrl"   }, // 11
    { "Prelude of Light",       "ayayxa"   }, // 12
    { "Song of Healing (MM)",   "xyrxyr"   }, // 13
    { "Inverted Song of Time",  "rlyrly"   }, // 14
    { "Song of Double Time",    "yyllrr"   }, // 15
    { "Oath to Order",          "yrlrya"   }, // 16
    { "Song of Soaring",        "rxarxa"   }, // 17
    { "Sonata of Awakening",    "axaxlyl"  }, // 18
    { "Goron's Lullaby",        "lyxlyxyl" }, // 19
    { "New Wave Bossa Nova",    "xaxyrxy"  }, // 20
    { "Elegy of Emptiness",     "yxyryax"  }, // 21
    { "Song of Frogs",          "lrxya"    }, // 22
    { "NULL",                   "------"   }, // 23 - ST
    { "Song of Awakening",      "ya"       }, // 24
    { "Song of Healing (ST)",   "lxl"      }, // 25
    { "Song of Discovery",      "aray"     }, // 26
    { "Song of Light",          "brayx"    }, // 27
    { "Song of Birds",          "brb"      }, // 28
    { "NULL",                   "------"   }, // 29 - CUSTOM/HIDDEN
    { "Chai Kingdom",           "yxaxy"    }, // 30
    { "NULL",                   "------"   }, // 31 - WW
    { "Wind's Requiem",         "ulr"      }, // 32
    { "Ballad of Gales",        "drlu"     }, // 33
    { "Command Melody",         "lnrn"     }, // 34
    { "Earth God's Lyric",      "ddarln"   }, // 35
    { "Wind God's Aria",        "uudrlr"   }, // 36
    { "Song of Passing",        "rld"      }  // 37
};

// Array of instruments
Instrument instruments[INSTRUMENTCOUNT] = {
    { "Ocarina", NOTESET_OOT, 22,
        {  1,  2,  3,  4,  5,  6,
           7,  8,  9, 10, 11, 12,
          13, 14, 15, 16, 17, 18,
          19, 20, 21, 30          }
    },
    { "Pipes", NOTESET_OOT, 2,
        { 18, 21 }
    },
    { "Drums", NOTESET_OOT, 1,
        { 19 }
    },
    { "Guitar", NOTESET_OOT, 1,
        { 20 }
    },
    { "Malon", NOTESET_OOT, 1,
        {  3 }
    },
    { "Harp", NOTESET_OOT, 6,
        {  7,  8,  9, 10, 11, 12 }
    },
    { "Frogs", NOTESET_OOT, 1,
        { 22 }
    },
    { "Music Box", NOTESET_OOT, 1,
        {  6 }
    },
    { "Spirit Flute", NOTESET_ST, 5,
        { 24, 25, 26, 27, 28 }
    },
    { "Wind Waker", NOTESET_WW, 6,
        { 32, 33, 34, 35, 36, 37 }
    }
};

// Converts string to uppercase characters
string upperStr(string in) {
    for (u32 i = 0; i < in.size(); ++i) {
        in[i] = toupper(in[i]);
    }
    return in;
}

// Converts boolean value to string
string boolToStr(bool in) {
    if (in == true) return "True";
    else return "False";
}

// Converts integer value to string
string intToStr(int num) {
    stringstream ss;
    ss << num;
    return ss.str();
}

// Download song from URL
void downloadSong(u16 songid) {
    // Stubbed for now
	return;
}

// Returns the first unused wavBuf available for use
int getOpenWavbuf() {

    for (u32 i = 0; i <= WAVEBUFCOUNT; i++) {
        if (!wavebufList[i]) {
            wavebufList[i] = true;
            return i;
        }
    }

    return -1;

}

// Initializes an audio source and returns any errors that may occur
int sourceInit(Source *self, const char *filename, int channel, int wbuf = -1) {
    FILE *file = fopen(filename, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        self->size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Set wavebuf
        if (wbuf==-1) self->wbuf = getOpenWavbuf();
        else self->wbuf = wbuf;
        self->loop = false;
        self->nsamples = (self->size / 2);

        // Read data
        //if (linearSpaceFree() < self->size) return -1;
        //self->data = (char*)linearAlloc(self->size);

        fread(self->data, self->size, 1, file);

        fclose(file);

        waveBuf[self->wbuf].data_vaddr = self->data;
        waveBuf[self->wbuf].nsamples = self->nsamples;
        waveBuf[self->wbuf].looping = self->loop;

        self->channel = channel;
    }
    else return -2;
    return 0;
}

// Plays an initialized audio source on its assigned DSP channel
int sourcePlay(Source *self) { // Source:play()

    if (self==NULL || self->wbuf == -1) return -1;

    //DSP_FlushDataCache((u32*)self->data, self->size);

    //ndspChnWaveBufAdd(self->channel, &waveBuf[self->wbuf]);

    return self->wbuf;
}

// Frees an audio source and its wavebuf
void sourceFree(Source *self) {
    if (self==NULL) return;
    wavebufList[self->wbuf] = false;
    //if (waveBuf[self->wbuf].status == NDSP_WBUF_PLAYING || waveBuf[self->wbuf].status==NDSP_WBUF_QUEUED) ndspChnWaveBufClear(self->channel);
    //linearFree(self->data);
    delete self;
}

// Initializes an instrument and returns any errors that may occur
int instrumentInit(u8 id) {

    for (u32 i = 0; i < notesets[instruments[id].nset].notes; i++) {
        // Free previous instrument's resources
        if (nicons[i] != NULL) delete(nicons[i]);
        sourceFree(notes[i]);

        // Load new instrument's resources
        string ipath = "romfs:/notes/"+intToStr(instruments[id].nset)+"_"+intToStr(i)+".bin";
        string spath = "romfs:/sound/notes/"+instruments[id].name+"/"+intToStr(i)+".pcm";

        nicons[i] = openFileBitmap(ipath.c_str(), 48, 48);
        notes[i] = new Source;

        int result = sourceInit(notes[i], spath.c_str(), 0);
        if (result!=0) return -1;
    }

    return 0;
}

// Frees an instrument and its resources
void instrumentFree(u8 id) {
    for (u32 i = 0; i < notesets[instruments[id].nset].notes; i++) {
        if (nicons[i] != NULL) delete(nicons[i]);
        sourceFree(notes[i]);
    }
}

int main(int argc, char **argv)
{
	// Seed rand
    srand(time(NULL));
	
	// Init graphics
    gfxInitDefault();
	
	consoleInit(NULL);
	
	printf("OrchestriNX by LeifEricson\n");
	
	// TODO: Load font
	
	// TODO: Load textures and instrument icons
	
	// Temp background from the 3DS version, a new HD one will have to be made
	printf("Opening test bitmap...\n");
	Bitmap* background_temp = openFileBitmap("/switch/orchestrinx/res/oldbg_400_240.bin", 400, 240);
	if (!background_temp) {
		printf("Error opening bitmap\n");
		fflush(stdout);
		svcSleepThread(5000000000);
		exit(1);
	}
	else {
		printf("Bitmap opened. First 100 RGB values:\n\n");
	}
	for (int i = 0; i < 300; i+=3) {
		printf("%d%d%d ", background_temp->buf[i], background_temp->buf[i+1], background_temp->buf[i+2]);
	}
	
	printf("\nCreating directory structure...\n");
	
	// Create data path
    mkdir("/switch", 0777);
    mkdir("/switch/orchestrinx", 0777);
    mkdir("/switch/orchestrinx/res", 0777);
    mkdir("/switch/orchestrinx/res/songs", 0777);
	
	// Number of songs found on the SD card
    int nsongs = 0;
	
	// Check if all songs are present
    for (int i = 0; i < SONGCOUNT; i++) {
        FILE* f = fopen(("/switch/orchestrinx/res/songs/"+songs[i].name+".pcm").c_str(), "rb");
        if (f!=NULL || songs[i].name=="NULL") nsongs++;
        fclose(f);
    }
	
	printf("Song count = %d\n", nsongs);
	
	// TODO: Initialize audio
	
	// TODO: Load SFX
	
	// Last 20 played notes
    string playingsong = "";
	
	// Index of current selected instrument
    u8 currentinstrument = 0;

    // Load Ocarina by default
    instrumentInit(0);

    // Index of song played
    int songtrigger = -1;

    // Index of button being pressed
    u32 pressed = 0xFF;
	
	// Whether free play is on or not
	bool freePlay = (nsongs<SONGCOUNT);
	
	bool errorflag = false;
    Result errorcode = 0;
	
	// Position of inventory graphic
	int invYPos = -240;
	
	bool invOpen = false;
	
	int wakertimer = 4;
	bool wakertimerdir = false;
	u32 rhythm = 3;
	int separation = 0;
	
	printf("Entering main loop...\n");

    // Main loop
    while(appletMainLoop())
    {
        //Scan all the inputs. This should be done once for each frame
        hidScanInput();


        //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 keys = hidKeysDown(CONTROLLER_P1_AUTO);
        u32 released = hidKeysUp(CONTROLLER_P1_AUTO);
		u32 held = hidKeysHeld(CONTROLLER_P1_AUTO);
		
		u32 keyset[MAXNOTES];

        memcpy(keyset, notesets[instruments[currentinstrument].nset].keys, MAXNOTES * sizeof(u32));
		
		// Exit on minus
		if (keys & KEY_MINUS) {
			break;
		}
		
		//drawStart();
		//drawBitmap(400, 240, background_temp);

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }

    gfxExit();
    return 0;
}

