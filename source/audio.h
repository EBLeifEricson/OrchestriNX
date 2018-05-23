#ifndef AUDIO_H
#define AUDIO_H

/*
A lot of this stuff was copied from libctru just for reference and will likely NOT work as is.
*/

/// ADPCM data.
typedef struct {
	u16 index;    ///< Current predictor index
	s16 history0; ///< Last outputted PCM16 sample.
	s16 history1; ///< Second to last outputted PCM16 sample.
} dspAdpcmData;

/// Wave buffer struct.
typedef struct dspbufstruct dspWaveBuf;
struct dspbufstruct {
	union
	{
		s8*         data_pcm8;  ///< Pointer to PCM8 sample data.
		s16*        data_pcm16; ///< Pointer to PCM16 sample data.
		u8*         data_adpcm; ///< Pointer to DSPADPCM sample data.
		const void* data_vaddr; ///< Data virtual address.
	};
	u32 nsamples;              ///< Total number of samples (PCM8=bytes, PCM16=halfwords, DSPADPCM=nibbles without frame headers)
	dspAdpcmData* adpcm_data; ///< ADPCM data.

	u32  offset;  ///< Buffer offset. Only used for capture.
	bool looping; ///< Whether to loop the buffer.
	u8   status;  ///< Queuing/playback status.

	u16 sequence_id;   ///< Sequence ID. Assigned automatically by ndspChnWaveBufAdd.
	dspWaveBuf* next; ///< Next buffer to play. Used internally, do not modify.
};

#endif