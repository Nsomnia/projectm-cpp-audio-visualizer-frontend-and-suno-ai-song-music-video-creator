#ifndef _PCM_H
#define _PCM_H

#include "dlldefs.h"
#define FFT_LENGTH 1024

class DLLEXPORT PCM {
public:
    void addPCM16Data(const short* pcm_data, short samples);
};
#endif  