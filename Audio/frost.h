/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Pete (El Supremo)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef frost_h_
#define frost_h_

#include "Arduino.h"
#include "AudioStream.h"
#include "arm_math.h"
#include "utility/qr_decomp.h"

#define CHANNELS 4

#define BLOCK_SIZE 128   /* teensyduino samples per block     */
                         /* equivalent to AUDIO_BLOCK_SAMPLES */

#define TAPS 0     /* number of input taps */

#define DIAGONAL_LOAD_CONST 0.7



inline void init_arm_matrix(arm_matrix_instance_f32 *matrix, int m, int n) {
    matrix->numRows = m;
    matrix->numCols = n;
    matrix->pData = (float32_t *) calloc(sizeof(float32_t), n*m);
    return;
}

class AudioFrostBeamformer : public AudioStream
{
public:
	AudioFrostBeamformer(void): AudioStream(CHANNELS, inputQueueArray) {
        input_matrix = qr_matrix_new(BLOCK_SIZE-TAPS, TAPS*CHANNELS);

        // init_arm_matrix(&input_matrix_cmsis, BLOCK_SIZE-TAPS, TAPS*CHANNELS);
        // init_arm_matrix(&output_matrix_cmsis, BLOCK_SIZE-TAPS, 1);
        // init_arm_matrix(&Q_cmsis, TAPS*CHANNELS, TAPS*CHANNELS);
        // init_arm_matrix(&weights_cmsis, TAPS*CHANNELS, 1);
        // init_arm_matrix(&steering_matrix, TAPS*CHANNELS, 1);

        int middle = TAPS*CHANNELS / 2;
        int low = middle - CHANNELS/2;
        int high = middle + CHANNELS/2;
        for (int i=0; i < TAPS*CHANNELS; i++) {
            if (low <= i && i < high) {
                steering[i] = 1;
            } else {
                steering[i] = 0;
            }
        }
        steering_matrix.pData = steering;
	}
	virtual void update(void);
private:

	audio_block_t *inputQueueArray[CHANNELS];

    // Size of TAPS * CHANNELS, middle CHANNELS should be 1
    // This should also be arm_matrix_instance_f32
    float steering[TAPS*CHANNELS];

    // static arm_matrix_instance_f32 input_matrix, output_matrix;
    arm_matrix_instance_f32 input_matrix_cmsis, Q_cmsis, R_cmsis, output_matrix_cmsis, steering_matrix, weights_cmsis;
    mat input_matrix, Q, R;


    void calculate_weights(arm_matrix_instance_f32 *R, arm_matrix_instance_f32 *weights);
};

#endif

