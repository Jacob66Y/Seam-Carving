#include "seamcarving.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t find_energy(struct rgb_img *im, int row, int col){
    int height = im->height;
    int width = im->width;
    int x_sum = 0;
    int y_sum = 0;
    int x_next;
    int x_before;
    int y_next;
    int y_before;
    for (int i = 0; i < 3; i++){
        if (col == 0){
            x_before = im->raster[((row+1) * width * 3) - 3 + i];
        }else{
            x_before = im->raster[(row * width * 3) + (col-1) * 3 + i];
        }
        if (col == width-1){
            x_next  = im->raster[(row * width * 3) + i];   
        }else{
            x_next = im->raster[(row * width * 3) + (col+1) * 3 + i];        
        }
        x_sum += ((x_next - x_before)*(x_next - x_before));
        if (row == 0){
            y_before = im->raster[((height-1) * width * 3) + col * 3 + i];
        }else{
            y_before = im->raster[((row-1) * width * 3) + col * 3 + i];
        }
        if(row == height-1){
            y_next = im->raster[col * 3 + i];
        }else{
            y_next = im->raster[((row+1) * width * 3) + col * 3 + i];
        }
        y_sum += ((y_next - y_before)*(y_next - y_before));
    }
    uint8_t result;
    result = sqrt(x_sum + y_sum) / 10;
    return result;
}

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int height = im->height;
    int width = im->width;
    uint8_t energy;
    *grad = (struct rgb_img *)malloc(sizeof(struct rgb_img));
    (*grad)->height = height;
    (*grad)->width = width;
    (*grad)->raster = (uint8_t *)malloc(height * width * 3);
    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            energy = find_energy(im, i, j);
            for (int k  =0; k < 3; k++){
                (*grad)->raster[(i*width*3) + 3 * j + k] = energy;
            }
        }
    }
}

double min(double a, double b){
    if (a < b){
        return a;
    }else{
        return b;
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){
    int height = grad->height;
    int width = grad->width;
    *best_arr = (double*)malloc(height * width * sizeof(double));
    for (int i = 0; i < width; i++){
        *((*best_arr) + i) = grad->raster[i*3];
    }
    for (int i = 1; i < height; i++){
        for (int j = 0; j < width; j++){        
            *((*best_arr) + i*width + j) = grad->raster[i*width*3 + j*3];      
            if (j == 0){      
                *((*best_arr) + i*width + j) += min(*((*best_arr) + (i-1)*width + j), *((*best_arr) + (i-1)*width + j + 1));
            }else if (j == width - 1){
                *((*best_arr) + i*width + j) += min(*((*best_arr) + (i-1)*width + j), *((*best_arr) + (i-1)*width + j - 1));
            }else{
                *((*best_arr) + i*width + j) += min(min(*((*best_arr) + (i-1)*width + j), *((*best_arr) + (i-1)*width + j - 1)), *((*best_arr) + (i-1)*width + j + 1));
            }
        }
    }
}

void recover_path(double *best, int height, int width, int **path){
    *path = (int *)malloc(height * sizeof(int)); 
    int index = 0;
    int min_num = best[(height - 1) * width];
    for (int j = 0; j< width; j++){
        if (min_num > best[(height - 1) * width + j]){
            index = j;
            min_num = best[(height - 1) * width + j];
        }
    }
    for (int i = height - 1; i >= 0; i--){
        double before = best[i*width + index - 1];
        double middle = best[i*width + index];
        double after = best[i*width + index + 1];
        if (index == width-1){
            if (before < middle){
                index = index - 1;
            }
        }else if (index == 0){
            if (after < middle){
                index = index + 1;
            }
        }else{
            if ((before < middle) & (before <= after)){
                index = index - 1;
            }else if((after < before) & (after < middle)){
                index = index + 1;
            }
        }
        *(*path + i) = index;
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    int height = src->height;
    int width = src->width;
    *dest = (struct rgb_img *)malloc(sizeof(struct rgb_img));
    (*dest)->height = height;
    (*dest)->width = width - 1;
    (*dest)->raster = (uint8_t *)malloc(height * (width - 1) * 3);
    int count = 0;
    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            if (j == path[i]){
                count ++;
            }else{
                for (int k =0; k< 3; k++){
                    (*dest)->raster[i * width * 3 + j * 3 - count * 3 + k] = src->raster[i * width * 3 + j * 3 + k];
                }
            }
        }
    }
}