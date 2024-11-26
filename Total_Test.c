#include "c_img.c"
#include "c_img.h"
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



void recover_path_a(double *best, int height, int width, int **path){
    *path = (int *)malloc(height * sizeof(int)); 
    int index = -1;
    for (int i = 0; i < height; i++){
        int min_num = min(best[i*width], best[i*width + 1]);
        for (int j = 0; j< width; j++){
            if (min_num > best[i * width + j]){
                index = j;
                min_num = best[i * width + j];
            }
        }
        *(*path + i) = index;
    }
}

void recover_path_good(double *best, int height, int width, int **path){
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
    printf("%d\n", index);
    for (int i = height - 1; i >= 0; i--){
        //printf("%f\n", best[i*width + index]);
        if (index == width-1){
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

void remove_seam_chris(struct rgb_img *src, struct rgb_img **dest, int *path)
{
    int height = src->height;
    int width = src->width;
    *dest = (struct rgb_img *)malloc(sizeof(struct rgb_img));
    (*dest)->height = height;
    (*dest)->width = width - 1;
    (*dest)->raster = (uint8_t *)malloc(3 * height * (width - 1));
    int count = 0;
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            for (int s = 0; s < 3; s++)
            {
                if (w == path[h])
                {
                    count++;
                }
                else
                {
                    (*dest)->raster[h * width * 3 + w * 3 + s - count * 3] = src->raster[h * width * 3 + w * 3 + s];
                }
            }
        }
    }
}

int main(){
    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    //read_in_img(&im, "6x5.bin");
    read_in_img(&im, "HJoceanSmall.bin");
    //read_in_img(&im, "jack.bin");
    //read_in_img(&im, "test_im1.bin");
    
    for(int i = 0; i < 260; i++){
        printf("i = %d\n", i);
        calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);
        

        if (i == 149){
                char filename[200];
                sprintf(filename, "Chris%d.bin", i);
                write_img(cur_im, filename);
            }

        for (int i = 1; i < grad->height * grad->width + 1; i++){
            //printf("%d\n", 11);
            //printf("%f ", best[i-1]);
            if (i % (grad->width) == 0){
                //printf("\n");
            }
        }
        //printf("\n");

        if (i < 10){
            for (int i =0; i < grad->height; i++){
                //printf("%d ", path[i]);
            }
        }

        //printf("\n");

        
        


        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
    }
    destroy_image(im);
}