//
// Created by Detjon on 16.07.2022.
//

#ifndef CONFIGS_H
#define CONFIGS_H

#define WIDTH 50
#define HEIGHT 50

#define BORDER_OFFSET_L_T 0 // left and top
#define BORDER_OFFSET_R_B 0 // right and bottom

#define PPM_MIN_COLOR 0
#define PPM_MAX_COLOR 255

#define PPM_RANGE 10
#define PPM_SCALE_FACTOR 25

#define NUMBER_OF_SAMPLES 100
#define FORWARD_TRAIN_PASSES 100

#define BIAS 10.0

#define CHECK_SEED_VALUE 666
#define TRAIN_SEED_VALUE 69

#define SAVE_TO_SAMPLES_FOLDER 1

#define SAMPLES_FOLDER_NAME "samples"
#define RECTANGLE_SAMPLE_NAME "rectangle"
#define WEIGHT_SAMPLE_NAME "weight"
#define CIRCLE_SAMPLE_NAME "circle"

#endif // CONFIGS_H