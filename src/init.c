/* libxtract feature extraction library
 *  
 * Copyright (C) 2006 Jamie Bullock
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
 * USA.
 */

/* init.c: defines functions that extract a feature as a single value from an input vector */

#include "xtract/libxtract.h"
#include <math.h>
#include <stdlib.h>

int xtract_init_mfcc(int N, float nyquist, int style, float freq_max, float freq_min, int freq_bands, float **fft_tables){

    int n,i, *fft_peak, M; 
    float norm, mel_freq_max, mel_freq_min, norm_fact, height, inc, val, 
        freq_bw_mel, *mel_peak, *height_norm, *lin_peak;


    mel_peak = height_norm = lin_peak = NULL;
    fft_peak = NULL;
    norm = 1; 

    mel_freq_max = 1127 * log(1 + freq_max / 700);
    mel_freq_min = 1127 * log(1 + freq_min / 700);
    freq_bw_mel = (mel_freq_max - mel_freq_min) / freq_bands;

    mel_peak = (float *)malloc((freq_bands + 2) * sizeof(float)); 
    /* +2 for zeros at start and end */
    lin_peak = (float *)malloc((freq_bands + 2) * sizeof(float));
    fft_peak = (int *)malloc((freq_bands + 2) * sizeof(int));
    height_norm = (float *)malloc(freq_bands * sizeof(float));

    if(mel_peak == NULL || height_norm == NULL || 
                    lin_peak == NULL || fft_peak == NULL)
                    return MALLOC_FAILED;
        
    M = N >> 1;

    mel_peak[0] = mel_freq_min;
    lin_peak[0] = 700 * (exp(mel_peak[0] / 1127) - 1);
    fft_peak[0] = lin_peak[0] / nyquist * M;


    for (n = 1; n <= freq_bands; n++){	
    /*roll out peak locations - mel, linear and linear on fft window scale */
        mel_peak[n] = mel_peak[n - 1] + freq_bw_mel;
        lin_peak[n] = 700 * (exp(mel_peak[n] / 1127) -1);
        fft_peak[n] = lin_peak[n] / nyquist * M;
    }

    for (n = 0; n < freq_bands; n++){
        /*roll out normalised gain of each peak*/
        if (style == EQUAL_GAIN){
            height = 1;	
            norm_fact = norm;
        }
        else{
            height = 2 / (lin_peak[n + 2] - lin_peak[n]);
            norm_fact = norm / (2 / (lin_peak[2] - lin_peak[0]));
        }
        height_norm[n] = height * norm_fact;
    }

    i = 0;
   
    for(n = 0; n < freq_bands; n++){
        if(n > 0)
            /*calculate the rise increment*/
            inc = height_norm[n] / (fft_peak[n] - fft_peak[n - 1]);
        else
            inc = height_norm[n] / fft_peak[n];
        val = 0;	
        for(; i <= fft_peak[n]; i++){ 
            /*fill in the 'rise' */
            fft_tables[n][i] = val;
            val += inc;
        }
        inc = height_norm[n] / (fft_peak[n + 1] - fft_peak[n]);
        /*calculate the fall increment */
        val = 0;
        for(i = fft_peak[n + 1]; i > fft_peak[n]; i--){ 
            /*reverse fill the 'fall' */
            fft_tables[n][i] = val;
            val += inc;
        }
    }

    
    free(mel_peak);
    free(lin_peak);
    free(height_norm);
    free(fft_peak);

    return SUCCESS;

}

int xtract_init_bark(int N, float nyquist, int *band_limits){

    float  bark_freq_max, bark_freq_min, freq_bw_bark, temp, edges[] = {0, 100, 200, 300, 400, 510, 630, 770, 920, 1080, 1270, 1480, 1720, 2000, 2320, 2700, 3150, 3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500, 20500, 27000}; /* Takes us up to sr = 54kHz (CCRMA: JOS)*/

    int M, bands = BARK_BANDS;
    
    M = N >> 1;
    
    while(bands--)
        band_limits[bands] = edges[bands] / nyquist * M;
        /*FIX shohuld use rounding, but couldn't get it to work */
}
