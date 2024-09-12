#ifndef __SNDBRIDGE_SND_STREAM2_H
#define __SNDBRIDGE_SND_STREAM2_H

/**
 * @brief Yamaha AICA 4-Bit ADPCM sample format.
 * This sample format is described as "looping" ADPCM in the specs
 * but with ADPCM sample streaming support.
 * starting KallistiOS v2.1.0 this macro value can be found as "AICA_SM_ADPCM_LS (Long stream ADPCM 4-bit)"
 */
#define SND_STREAM_SM_AICA_ADPCM   3

/** \brief  The maximum buffer size (in bytes) for a ADPCM stream.
 * This value was rounded from 0x10000 to 0x8000. The AICA "loopend" register
 * only can hold 65535 (only 65534 usable for adpcm) samples and the ADPCM format can go
 * up to 131070 samples, to avoid this, the buffer size is limited to 32767 bytes.
*/
#define SND_STREAM2_BUFFER_ADPCM_MAX 0x8000
//#define SND_STREAM2_BUFFER_ADPCM_MAX 0x7fff
//#define SND_STREAM2_BUFFER_ADPCM_MAX 0x7ffc

#endif
