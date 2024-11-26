#include "c_img.c"
#include "c_img.h"
#include "seamcarving.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>



uint8_t find_energy(struct rgb_img *im, int row, int col){
    int height = im->height;
    int width = im->width;
    int x_sum = 0;
    int y_sum = 0;
    int x_next;
    int x_before;
    int y_next;
    int y_before;
    //printf("%d %d\n", col, width);
    //printf("%d\n", im->raster[3]);
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
        if (i == 0){
            //printf("%d\n", x_next);
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
    //printf("%d\n", result);
    return result;
}

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int height = im->height;
    int width = im->width;
    //printf("%d\n", width);
    uint8_t energy;
    *grad = (struct rgb_img *)malloc(sizeof(struct rgb_img));
    (*grad)->height = height;
    (*grad)->width = width;
    (*grad)->raster = (uint8_t *)malloc(height * width * 3);
    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            energy = find_energy(im, i, j);
            //printf("%d %d\n", i, j);

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
            
            //*best_arr[i*j + j] = grad->raster[i*width*3 + j*3];
            if (j == 0){
                //printf("%f ", *((*best_arr) + i*j + j));
                *((*best_arr) + i*width + j) += min(*((*best_arr) + (i-1)*width + j), *((*best_arr) + (i-1)*width + j + 1));
                //*best_arr[i*j + j] += min(grad->raster[(i-1) * width * 3 + 3 * j], grad->raster[(i-1) * width * 3 + 3 * j + 1]);
                //printf("%f\n", *((*best_arr) + (i-1)*width + j + 1));
                //printf("%f\n", *((*best_arr) + (i-1)*width + j));
            }else if (j == width - 1){
                *((*best_arr) + i*width + j) += min(*((*best_arr) + (i-1)*width + j), *((*best_arr) + (i-1)*width + j - 1));
                //*best_arr[i*j + j] += min(grad->raster[(i-1) * width * 3 + 3 * j], grad->raster[(i-1) * width * 3 + 3 * j - 1]);
            }else{
                *((*best_arr) + i*width + j) += min(min(*((*best_arr) + (i-1)*width + j), *((*best_arr) + (i-1)*width + j - 1)), *((*best_arr) + (i-1)*width + j + 1));
                //*best_arr[i*j + j] += min(min(grad->raster[(i-1) * width * 3 + 3 * j], grad->raster[(i-1) * width * 3 + 3 * j - 1]), grad->raster[(i-1) * width * 3 + 3 * j + 1]);
            }
        }
    }
}

int find_index_with_less_sum(double *best, int height, int width){
    int ind;
    int ans;
    double summative;
    double min_num = best[(height - 1) * width] * height * height;
    for (int k = 0; k < width; k++){
        ind = k;
        //*(*path + height) = index;
        //printf("%d\n", index);
        summative = 0;
        summative += best[(height - 1) * width + k];
        printf("%f\n", summative);
        for (int i = height - 1; i >= 0; i--){
            //printf("%f\n", best[i*width + index]);
            if (ind == width){
                if (best[i*width + ind - 1] < best[i*width + ind]){
                    ind = ind - 1;
                    printf("%s\n", "A");
                }
            }else if (ind == 0){
                if (best[i*width + ind + 1] < best[i*width + ind]){
                    ind = ind + 1;
                    printf("%s\n", "B");
                }
            }else{
                double before = best[i*width + ind - 1];
                double middle = best[i*width + ind];
                double after = best[i*width + ind + 1];
                if ((before < middle) & (before <= after)){
                    ind = ind - 1;
                } else if((after <= before) & (middle > after)){
                    ind = ind + 1;
                }
            summative += best[i*width + ind];
            printf("%f\n", summative);
            }
        }
        
        if (summative < min_num){
            min_num = summative;
            ans = k;
            printf("%d\n", ind);
        }
    }
    
    return ans;
    
}

void recover_path_c(double *best, int height, int width, int **path){
    *path = (int *)malloc(height * sizeof(int)); 
    int index = find_index_with_less_sum(best, height, width);
    
    //*(*path + height) = index;
    printf("%d %s\n", index, "A");
    for (int i = height - 2; i >= 0; i--){
        //printf("%f\n", best[i*width + index]);
        if (index == width){
            if (best[i*width + index - 1] < best[i*width + index]){
                index = index - 1;
                //printf("%s\n", "A");
            }
        }else if (index == 0){
            if (best[i*width + index + 1] < best[i*width + index]){
                index = index + 1;
                //printf("%s\n", "A");
            }
        }else{
            double before = best[i*width + index - 1];
            double middle = best[i*width + index];
            double after = best[i*width + index + 1];
            if ((before < middle) & (before <= after)){
                index = index - 1;
            } else if((after <= before) & (middle > after)){
                index = index + 1;
            }
        //printf("%d\n", index);
        }
        *(*path + i) = index;
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
    //*(*path + height) = index;
    //printf("%d\n", index);
    for (int i = height - 1; i >= 0; i--){
        //printf("%f\n", best[i*width + index]);
        double before = best[i*width + index - 1];
        double middle = best[i*width + index];
        double after = best[i*width + index + 1];
        if (index == width-1){
            if (before < middle){
                index = index - 1;
                //printf("%s\n", "A");
            }
        }else if (index == 0){
            if (after < middle){
                index = index + 1;
                //printf("%s\n", "A");
            }
        }else{
            if ((before < middle) & (before <= after)){
                index = index - 1;
            }else if((after < before) & (after < middle)){
                index = index + 1;
            }
        //printf("%d\n", index);
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

void print_raster(struct rgb_img *im) {
    size_t height = im->height;
    size_t width = im->width;
    uint8_t *raster = im->raster;

    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            size_t index = (y * width + x) * 3; // Calculate the index of the pixel in the raster array

            uint8_t r = raster[index];   // Red component
            uint8_t g = raster[index + 1]; // Green component
            uint8_t b = raster[index + 2]; // Blue component

            printf("(%d, %d, %d) ", r, g, b); // Print RGB values of the pixel
        }
        printf("\n");
    }
}


int main(){

    struct rgb_img *im;
    struct rgb_img *grad;
    struct rgb_img *cur_im;
    double *best;
    int *path;

    //read_in_img(&im, "3x4.bin");
    read_in_img(&im, "6x5.bin");
    //read_in_img(&im, "HJoceanSmall.bin");
    //read_in_img(&im, "lon.bin");
    calc_energy(im,  &grad);
    print_grad(grad);
    dynamic_seam(grad, &best);
    recover_path(best, grad->height, grad->width, &path);
    remove_seam(im, &cur_im, path);
    printf("\n");

    for (int i = 1; i < grad->height * grad->width + 1; i++){
        //printf("%d\n", 11);
        printf("%f ", best[i-1]);
        if (i % (grad->width) == 0){
            printf("\n");
        }
    }
    printf("\n");

    

    for (int i = 0; i < grad->width; i++){
        printf("%d ", path[i]);
    }
    printf("\n");
    printf("\n");
    
    //write_img(im, "abc.txt");
    //print_grad(im);
    //print_raster(cur_im);

    destroy_image(im);

    //printf("%d\n", 7 % 2);

}