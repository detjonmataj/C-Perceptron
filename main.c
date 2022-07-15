#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#define WIDTH 50
#define HEIGHT 50
#define PPM_MAX_COLOR 255
#define PPM_MIN_COLOR 0
#define NUMBER_OF_SAMPLES 15
#define SAMPLES_FOLDER_NAME "samples"
#define RECTANGLE_SAMPLE_NAME "rectangle"
#define CIRCLE_SAMPLE_NAME "circle"
#define BORDER_OFFSET_L_T 10
#define BORDER_OFFSET_R_B 10

#define PPM_SCALE_FACTOR 25
#define scale(x) ((x)*PPM_SCALE_FACTOR)
#define unscale(x) ((x) / PPM_SCALE_FACTOR)
#define min(a, b) ((a) < (b) ? (a) : (b))

typedef float Layer[HEIGHT][WIDTH];

// Save the layer to a ppm file
void save_layer_ppm(Layer layer, const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if (fp == NULL)
	{
		///* %m will print the error message from strerror() supplied by errno
		///* Such as "No such file or directory"
		///* %m is only supported by GCC (GNU Compiler Collection)
		///? fprintf(stderr, "Error opening file %s: %m\n", filename);

		///* Print the error message from strerror() supplied by errno
		fprintf(stderr, "Error opening file %s: %s\n", filename, strerror(errno));
		exit(1);
	}

	///* Header of the ppm file
	fprintf(fp, "P6\n");
	fprintf(fp, "%d %d\n", scale(WIDTH), scale(HEIGHT));
	fprintf(fp, "255\n");

	///* Write the layer to the file
	for (int y = 0; y < scale(HEIGHT); ++y)
	{
		for (int x = 0; x < scale(WIDTH); ++x)
		{
			float scaler = layer[unscale(y)][unscale(x)];
			///* Creating a blue color pixel from the scaler
			unsigned char pixel[3] = {0, 0, (unsigned char)scale(scaler * PPM_MAX_COLOR)};

			///* Write the pixel to the file
			fwrite(pixel, sizeof pixel, 1, fp);
		}
	}

	///* Close the file
	fclose(fp);
}

///* Save the layer to a binary file
void save_layer_bin(Layer layer, const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if (fp == NULL)
	{
		///* %m will print the error message from strerror() supplied by errno
		///* Such as "No such file or directory"
		///* %m is only supported by GCC (GNU Compiler Collection)
		///? fprintf(stderr, "Error opening file %s: %m\n", filename);

		///* Print the error message from strerror() supplied by errno
		fprintf(stderr, "Error opening file %s: %s\n", filename, strerror(errno));
		exit(1);
	}

	///* Write the layer to the file

	fwrite(layer, sizeof(Layer), 1, fp);

	///* Close the file
	fclose(fp);
}

///* Load the layer from a binary file
void load_layer_bin(Layer layer, const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		///* %m will print the error message from strerror() supplied by errno
		///* Such as "No such file or directory"
		///* %m is only supported by GCC (GNU Compiler Collection)
		///? fprintf(stderr, "Error opening file %s: %m\n", filename);

		///* Print the error message from strerror() supplied by errno
		fprintf(stderr, "Error opening file %s: %s\n", filename, strerror(errno));
		exit(1);
	}

	///* Read the layer from the file
	fread(layer, sizeof(Layer), 1, fp);

	///* Close the file
	fclose(fp);
}

///* Clamp a value between a min and max value
float clamp(int value, int min, int max)
{
	if (value < min)
		return min;

	if (value > max)
		return max;

	return value;
}

///* Fill random rectangular area with a value
/// @param layer, Layer to fill
/// @param x, X coordinate of the top-left corner
/// @param y, Y coordinate of the top-left corner
/// @param width, Width of the area to fill
/// @param height, Height of the area to fill
/// @param value, Value to fill the area with
void fill_rectangle(Layer layer, int x, int y, int width, int height, float value)
{
	assert(width > 0);
	assert(height > 0);

	///* The area to fill is limited to the layer's dimensions
	int x0 = clamp(x, 0, WIDTH - 1);
	int y0 = clamp(y, 0, HEIGHT - 1);
	int x1 = clamp(x0 + width - 1, 0, WIDTH - 1);
	int y1 = clamp(y0 + height - 1, 0, HEIGHT - 1);

	///* Fill the area with the given value
	for (int i = y0; i <= y1; ++i)
	{
		for (int j = x0; j <= x1; ++j)
		{
			layer[i][j] = value;
		}
	}
}

// fill circle
void fill_circle(Layer layer, int center_x, int center_y, int radius, float value)
{
	assert(radius > 0);
	assert(center_x >= 0);
	assert(center_x < WIDTH);
	assert(center_y >= 0);
	assert(center_y < HEIGHT);

	///* The area to fill is limited to the layer's dimensions
	int x0 = clamp(center_x - radius, 0, WIDTH - 1);
	int y0 = clamp(center_y - radius, 0, HEIGHT - 1);
	int x1 = clamp(center_x + radius, 0, WIDTH - 1);
	int y1 = clamp(center_y + radius, 0, HEIGHT - 1);

	///* Fill the area with the given value
	for (int i = y0; i <= y1; ++i)
	{
		for (int j = x0; j <= x1; ++j)
		{
			///* Calculate the distance from the center of the circle to the pixel (x, y)
			///* and compare it to the radius of the circle to determine if the pixel is
			///* inside the circle or not
			///* (if the distance is less than the radius, the pixel is inside the circle)
			float distance = sqrt(pow(i - center_y, 2) + pow(j - center_x, 2));
			if (distance <= radius)
			{
				layer[i][j] = value;
			}
		}
	}
}

///* Transfer function
float forward(Layer inputs, Layer weights)
{

	float output = 0.0f;

	for (int y = 0; y < HEIGHT; ++y)
	{
		for (int x = 0; x < HEIGHT; ++x)
		{
			output += inputs[x][y] * weights[x][y];
		}
	}

	return output;
}

///* Generate random value between range min and max
int random_int_range(int min, int max)
{
	assert(min < max);
	return min + rand() % (max - min + 1);
}

void random_rectangle_layer(Layer layer)
{

	int x = random_int_range(0, WIDTH - 2);			  ///* X coordinate of the top-left corner
	int y = random_int_range(0, HEIGHT - 2);		  ///* Y coordinate of the top-left corner
	int width = random_int_range(1, WIDTH - 1 - x);	  ///* Width of the rectangle
	int height = random_int_range(1, HEIGHT - 1 - y); ///* Height of the rectangle

	fill_rectangle(layer, x, y, width, height, 1.0f);
}

void random_circle_layer(Layer layer)
{

	int center_x = random_int_range(BORDER_OFFSET_L_T, WIDTH - BORDER_OFFSET_R_B);	///* X coordinate of the center of the circle
	int center_y = random_int_range(BORDER_OFFSET_L_T, HEIGHT - BORDER_OFFSET_R_B); ///* Y coordinate of the center of the circle

	///* Radius of the circle
	///* Making sure that the circle is between the layer's dimensions

	///? int min_allowed_radius = (min(BORDER_OFFSET_L_T, BORDER_OFFSET_R_B) / 2) - 1;
	int min_allowed_radius = 1;
	int max_allowed_radius = min(WIDTH, HEIGHT) / 2;

	///* Minimum between center_x and center_y
	max_allowed_radius = min(min(center_x, center_y), max_allowed_radius);

	///* Minimum between difference between center_x and center_y and the layer's dimensions
	max_allowed_radius = min(min(WIDTH - center_x, HEIGHT - center_y), max_allowed_radius);

	///* Generate random radius between min and max radius
	int radius = (min_allowed_radius != max_allowed_radius) ? random_int_range(min_allowed_radius, max_allowed_radius) : max_allowed_radius;

	fill_circle(layer, center_x, center_y, radius, 1.0f);
}

///* Initialized as 0 if not initialized explicitly. (ISO C Standart)
static Layer inputs;
///* Initially Frank Rosenblatt set all the weights to 0
static Layer weights;

int main(void)
{

	///* Sample file name buffer
	char filename[256];

	///* Create a folder for the sample
	struct stat st = {0};

	if (stat(SAMPLES_FOLDER_NAME, &st) == -1)
	{
		mkdir(SAMPLES_FOLDER_NAME);
	}

	///* Changle the current working directory to the sample folder
	chdir(SAMPLES_FOLDER_NAME);

	if (stat("bin", &st) == -1)
	{
		mkdir("bin");
		mkdir("bin/rectangles");
		mkdir("bin/circles");
	}
	if (stat("ppm", &st) == -1)
	{
		mkdir("ppm");
		mkdir("ppm/rectangles");
		mkdir("ppm/circles");
	}

	chdir("..");

#if defined(PREFIX)
#undef PREFIX
#endif // PREFIX
#define PREFIX CIRCLE_SAMPLE_NAME

	for (int i = 0; i < NUMBER_OF_SAMPLES; ++i)
	{
		printf("[Info] generating " PREFIX " %d\n", i);

		///* Fill entire layer with 0
		memset(inputs, 0x0, sizeof(Layer));

		///* Generate random circle
		random_circle_layer(inputs);

		///* Save the layer to a binary and PPM file
		snprintf(filename, sizeof filename, SAMPLES_FOLDER_NAME "/bin/" PREFIX "s/" PREFIX "-sample_%03d.bin", i);
		save_layer_bin(inputs, filename);
		snprintf(filename, sizeof filename, SAMPLES_FOLDER_NAME "/ppm/" PREFIX "s/" PREFIX "-sample_%03d.ppm", i);
		save_layer_ppm(inputs, filename);
	}

#if defined(PREFIX)
#undef PREFIX
#endif // PREFIX
#define PREFIX RECTANGLE_SAMPLE_NAME

	for (int i = 0; i < NUMBER_OF_SAMPLES; ++i)
	{
		printf("[Info] generating " PREFIX " %d\n", i);

		///* Fill entire layer with 0
		memset(inputs, 0x0, sizeof(Layer));

		///* Generate random rectangle
		random_rectangle_layer(inputs);

		///* Save the layer to a binary and PPM file
		snprintf(filename, sizeof filename, SAMPLES_FOLDER_NAME "/bin/" PREFIX "s/" PREFIX "-sample_%03d.bin", i);
		save_layer_bin(inputs, filename);
		snprintf(filename, sizeof filename, SAMPLES_FOLDER_NAME "/ppm/" PREFIX "s/" PREFIX "-sample_%03d.ppm", i);
		save_layer_ppm(inputs, filename);
	}

	return 0;
}
