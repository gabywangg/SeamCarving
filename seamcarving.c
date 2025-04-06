#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "c_img.h"
#include "seamcarving.h"

void calc_energy(struct rgb_img *im, struct rgb_img **grad)
//calculates the energy gradient of an image, wrapping around the sides (treating it like a torus!)
//and puts the gradiant in a new image called grad
{
  int H = im->height;
  int W = im->width;

  create_img(grad, H, W); //allocates memory
 
  for (int j = 0; j < H; j++){ //for every pixel
    for (int i = 0; i < W; i++){
        int sum = 0;
        for (int c = 0; c < 3; c++){ //for each color
            int dx = get_pixel(im, j, (i-1+W)%W, c) - get_pixel(im, j, (i+1+W)%W, c); //calculate L/R difference
            //printf("pixel: %d, %d, color: %d, left ind: %d, right ind: %d, dx:%d\n", i, j, c, (i-1+W)%W, (i+1+W)%W, dx);
            int dy = get_pixel(im, (j-1+H)%H, i, c) - get_pixel(im, (j+1+H)%H, i, c); //calculate U/D difference
           // printf("pixel: %d, %d, color: %d, up ind: %d, down ind: %d, dy:%d\n", i, j, c, (j-1+H)%H, (j+1+H)%H, dy);
            sum += dx*dx + dy*dy;
        }
        int pixel_e = sqrt(sum);
        int grad_e = (uint8_t)(pixel_e/10);
        set_pixel(*grad, j, i, grad_e, grad_e, grad_e);
    }
  }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr)
{ // allocates and computes the dynamic array *best_arr.
    int H = grad->height;
    int W = grad->width;

    *best_arr = (double *)malloc(sizeof(double) * H * W); // allocating space in the memory for the best array to be stored

    if (*best_arr == NULL)
    { // making sure you don't proceed if malloc fails
        fprintf(stderr, "Memory allocation failed\n");
    }

    for (int j = 0; j < H; j++)
    {
        for (int i = 0; i < W; i++)
        {
            double my_energy = get_pixel(grad, j, i, 0); // pixel's own energy (not sure why in this order but... leave it for now)

            if (j == 0){                                       // base case--top border
                (*best_arr)[j * W + i] = my_energy; // setting the costs of the top row as themselves in the best array
            }
            else {
                //(*best_arr)[j*W+i] = 0;
                double min;
                int l_idx = (j - 1) * W + i - 1;
                int u_idx = (j - 1) * W + i;
                double left = (i > 0) ? (*best_arr)[l_idx] : INFINITY; //the up for all the non-top pixels
                double up = (*best_arr)[u_idx]; 

                if (left < up){
                    min = left;
                }
                else{
                    min = up;
                }

                int r_idx = (j - 1) * W + i + 1;
                double right = (i < W - 1) ? (*best_arr)[r_idx] : INFINITY; // everything not right border

                if (right < min){
                    min = right;
                }

                (*best_arr)[j * W + i] = my_energy + min;            
            }
        }
    }
}

void recover_path(double *best, int height, int width, int **path)
{
    // start from the bottom of the best and see which one is the smallest
    // once you find it, work your way back up through the mins to see which path it  took, saving the indexes in an array called path
    int H = height;
    int W = width;
    double min = INFINITY;
    int idx;

    (*path) = malloc(sizeof(int)*H);

    //this is when j = H-1
    for (int i = 0; i < W; i++){ // check through the bottom row to find the min
        if (best[(H - 1) * W + i] < min) {
            min = best[(H - 1) * W + i]; //identify the lowest path cost pixel on bottom row
            idx = i; //and save its index as "idx"
        }
    }
    (*path)[H-1] = idx; //and put that mf index in the bottom of the path array

    for (int j = H - 2; j >= 0; j--){ // at every row we're going up
        double min = best[j * W + idx];
        int min_idx = idx;

        // check left
        if (idx > 0 && best[j * W + idx - 1] < min) {
            min = best[j * W + idx - 1];
            min_idx = idx - 1;
        }

        // check right
        if (idx < W - 1 && best[j * W + idx + 1] < min) {
            min = best[j * W + idx + 1];
            min_idx = idx + 1;
        }

        idx = min_idx;
        (*path)[j] = idx;
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path)
{
    int H = src->height;
    int W = src->width;
    create_img(dest, H, W - 1);
    for (int j = 0; j < H; j++){
        int src_col_offset = 0;
        for (int i = 0; i < W - 1; i++)
        {
            if (i == path[j])
            {
                src_col_offset = 1;
            }
            set_pixel(*dest, j, i, get_pixel(src, j, src_col_offset + i, 0), get_pixel(src, j, src_col_offset + i, 1), get_pixel(src, j, src_col_offset + i, 2));
        }
    }
}
