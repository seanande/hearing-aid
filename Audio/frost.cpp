#include "arm_math.h"
#include "AudioStream.h"
#include "Arduino.h"
#include "frost.h"

void mat_t_to_arm_matrix(mat src, arm_matrix_instance_f32 *dest) {
    dest->numRows = src->m;
    int numCols = src->n;
    dest->numCols = numCols;
    for (int i=0; i < src->m; i++) {
        for (int j=0; j < src->n; j++) {
            dest->pData[i*numCols+j] = (float32_t) src->v[i][j];
        }
    }
}

void create_input_matrix(mat input_matrix, audio_block_t* blocks[CHANNELS]) {
  /*
  (A, B) means the Bth sample of channel A


  Shown here for CHANNELS = 3, TAPS = 3, BLOCK_SIZE = 10

  ==== Blocks =========================================================
  (0, 0) (0, 1) (0, 2) (0, 3) (0, 4) (0, 5) (0, 6) (0, 7) (0, 8) (0, 9)
  (1, 0) (1, 1) (1, 2) (1, 3) (1, 4) (1, 5) (1, 6) (1, 7) (1, 8) (1, 9)
  (2, 0) (2, 1) (2, 2) (2, 3) (2, 4) (2, 5) (2, 6) (2, 7) (2, 8) (2, 9)
  (3, 0) (3, 1) (3, 2) (3, 3) (3, 4) (3, 5) (3, 6) (3, 7) (3, 8) (3, 9)

  ==== Desired input_matrix ===========================================
  (0, 0) (1, 0) (2, 0)  ??     ??     ??     ??     ??     ??
  (0, 1) (1, 1) (2, 1) (0, 0) (1, 0) (2, 0)  ??     ??     ??
  (0, 2) (1, 2) (2, 2) (0, 1) (1, 1) (2, 1) (0, 0) (1, 0) (2, 0) < matrix actually starts here
  (0, 3) (1, 3) (2, 3) (0, 2) (1, 2) (2, 2) (0, 3) (1, 3) (2, 3)
  (0, 4) (1, 4) (2, 4) (0, 3) (1, 3) (2, 3) (0, 4) (1, 4) (2, 4)
  (0, 5) (1, 5) (2, 5) (0, 4) (1, 4) (2, 4) (0, 5) (1, 5) (2, 5)
  (0, 6) (1, 6) (2, 6) (0, 5) (1, 5) (2, 5) (0, 6) (1, 6) (2, 6)
  (0, 7) (1, 7) (2, 7) (0, 6) (1, 6) (2, 6) (0, 7) (1, 7) (2, 7)
  (0, 8) (1, 8) (2, 8) (0, 7) (1, 7) (2, 7) (0, 8) (1, 8) (2, 8)
  (0, 9) (1, 9) (2, 9) (0, 8) (1, 8) (2, 8) (0, 9) (1, 9) (2, 9)
  ===================================================================

  This matrix is size (BLOCK_SIZE - TAPS) rows * (CHANNELS * TAPS) columns
  TODO: fill in ?? with samples from the previous block

  We expect BLOCK_SIZE - TAPS >> CHANNELS * TAPS.
  */
    for (int chan=0; chan < CHANNELS; chan++) {
        audio_block_t *b = blocks[chan];
        for (int sample=TAPS-1; sample < BLOCK_SIZE; sample++) {
            double value = (double) b->data[sample];
            for (int tap=0; tap < TAPS; tap++) {
                // Serial.printf("Chan %d, Sample %d, Tap %d\n", chan, sample, tap);
                if (sample + tap < input_matrix->m) {
                    // don't try to write the values that "fall off" the bottom

                    // Serial.printf("Sample+tap: %d, chan+tap*CHANNELS: %d\n", sample+tap, chan+tap*CHANNELS);
                    // Serial.printf("Old value: %f\n", input_matrix->v[sample+tap][chan+tap*CHANNELS]);
                    input_matrix->v[sample+tap][chan+tap*CHANNELS] = value;
                }
            }
        }
    }
}

void populate_output_block(arm_matrix_instance_f32 *output_matrix, audio_block_t *block) {
    for (int i=0; i < BLOCK_SIZE-TAPS; i++) {
        block->data[i] = output_matrix->pData[i];
    }
}

void AudioFrostBeamformer::calculate_weights(arm_matrix_instance_f32 *R, arm_matrix_instance_f32 *weights) {
    arm_matrix_instance_f32 R_inverse;
    arm_mat_inverse_f32(R, &R_inverse);
    arm_mat_mult_f32(&R_inverse, &steering_matrix, weights);
}

void AudioFrostBeamformer::update(void) {
    audio_block_t *blocks[CHANNELS];
    mat R, Q;

    blocks[0] = receiveReadOnly(0);
    if (!blocks[0]) {
        return;
    }
    for (int i = 1; i < CHANNELS; i++) {
        blocks[i] = receiveReadOnly(i);
    }

    // input_matrix is a mat_t
    // size (BLOCK_SIZE-TAPS) rows x (TAPS * CHANNELS) cols
    create_input_matrix(input_matrix, blocks);

    // Calculate the QR decomposition
    // R is a mat_t
    // size SHOULD BE (TAPS * CHANNELS) square, but I haven't checked
    // matrix_show(input_matrix);

    householder(input_matrix, &R, &Q);

    // // Apply diagonal loading
    // for (int i = 0; i < TAPS*CHANNELS; i++) {
    //     R->v[i][i] = R->v[i][i] + DIAGONAL_LOAD_CONST;
    // }

    // // R_cmsis is an arm_matrix_instance_f32
    // // size is equal to R (TAPS * CHANNELS square)
    // mat_t_to_arm_matrix(R, &R_cmsis);

    // // weights is an arm_matrix_instance_f32
    // // size is (TAPS * CHANNELS) rows x 1 column
    // calculate_weights(&R_cmsis, &weights_cmsis);

    // // input_matrix_cmsis is an arm_matrix_instance_f32
    // // size (BLOCK_SIZE-TAPS) rows x (TAPS * CHANNELS) cols
    // mat_t_to_arm_matrix(input_matrix, &input_matrix_cmsis);

    // // output_matrix_cmsis is an arm_matrix_instance_f32
    // // size (BLOCK_SIZE-TAPS) rows x 1 col
    // arm_mat_mult_f32(&input_matrix_cmsis, &weights_cmsis, &output_matrix_cmsis);

    audio_block_t *block_new = allocate();
    populate_output_block(&output_matrix_cmsis, block_new);

    transmit(block_new);
    release(block_new);

    for (int i = 0; i < CHANNELS; i++) {
        release(blocks[i]);
    }
}

