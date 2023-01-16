#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define PI 3.14

// struct of pgm image
typedef struct pgm{
    char pgm_type[3];
    int width;
    int height;
    int max_value;
    int** data;
}PGM;

void ignore_comments(FILE *fp){
    int ch;
    char line[100];
 
    // Ignore blank lines
    while ((ch = fgetc(fp)) != EOF && isspace(ch));
 
    // Recursively ignore comments
    // in a PGM image commented lines
    // start with a '#'
    if (ch == '#') {
        fgets(line, sizeof(line), fp);
        ignore_comments(fp);
    }
    else
        fseek(fp, -1, SEEK_CUR);
}

// read image and load data to struct
PGM* read_pgm(PGM *p,char* file_path){
    int i,j;
    // open image file as binary 'rb'
    FILE *pgm = fopen(file_path,"rb");

    if(pgm == NULL){
        printf("File does not exist!");
        exit(0);
    }
    ignore_comments(pgm);
    fscanf(pgm,"%s",p->pgm_type);

    ignore_comments(pgm);
    // Read the image dimensions
    fscanf(pgm, "%d %d",&(p->width),&(p->height));

    ignore_comments(pgm);
 
    // Read maximum gray value
    fscanf(pgm, "%d", &(p->max_value));
    ignore_comments(pgm);

    // Allocation memory to store data of image
    printf("height and width: %d %d\n",p->height,p->width);
    p->data = malloc(p->height * sizeof(int*));
    for (i = 0;i < p->height; i++)
        p->data[i]= malloc(p->width * sizeof(int));

    // if pgm version is P5
    if(strcmp(p->pgm_type,"P5") == 0){
        char buffer;
        for(i=0;i<p->height;i++){
            for(j=0;j<p->width;j++){
                buffer = getc(pgm);
                p->data[i][j] = (int)buffer;
                printf("%d ",p->data[i][j]);
            }
        }
        
    }else{
        // Storing the pixel info in
        // the struct
        for (i = 0;i < p->height; i++) {
            for(j=0;j<p->width;j++)
                fscanf(pgm,"%d ",&(p->data[i][j]));
        }
    }
    fclose(pgm);
    // for(i=0;i<p->height;i++){
    //     for(j=0;j<p->width;j++)
    //         printf("%d ",p->data[i][j]);
    //     printf("\n");
    // }
    return p;
}

// Function to print the file details
void printImageDetails(PGM* p){ 
    int i,j;
    printf("PGM File type  : %s\n",p->pgm_type);
 
    // Print type of PGM file, in ascii
    // and binary format
    if (!strcmp(p->pgm_type, "P2"))
        printf("PGM File Format: ASCII\n");
    else if (!strcmp(p->pgm_type,"P5"))
        printf("PGM File Format: Binary\n");
 
    printf("Width of img   : %d px\n",p->width);
    printf("Height of img  : %d px\n",p->height);
    printf("Max Gray value : %d\n",p->max_value);
    // for(i=0;i<p->height;i++){
    //     for(j=0;j<p->width;j++)
    //         printf("%d ",p->data[i][j]);
    //     printf("\n");
    // }
}

// write to pgm file
void write_image(char* file_name,PGM *p){
    int i,j;
    FILE *fp;
    printf("Writing....\n");
    fp = fopen(file_name,"wb");
    if (fp == NULL){
        printf("Can not open file to write\n");
        exit(-1);
    }

    fprintf(fp,"%s\n",p->pgm_type);
    fprintf(fp,"#created by Ghyath Moussa\n");
    fprintf(fp,"%d %d\n",p->width,p->height);
    fprintf(fp,"%d\n",p->max_value);
    // write image
    for(i=0;i<p->height;i++)
        for(j=0;j<p->width;j++)
            fprintf(fp,"%d ",p->data[i][j]);

    fclose(fp);
}

// normalize data
    
void normalize(int width,int height,int** matrix){
    int i,j, min = 1000000,max=0;
    double rat;

    for(i=0;i<height;i++){
        for(j=0;j<width;j++){
            if(matrix[i][j] < min)
                min = matrix[i][j];
            else if(matrix[i][j] > max)
                max = matrix[i][j];
        }
    }

    for(i=0;i<height;i++){
        for(j=0;j<width;j++){
            rat = (double) (matrix[i][j] - min) / (max-min);
            matrix[i][j] = rat * 255;
        }
    }
} 

// Filter Operations
// create gaussian kernel filter
double** filter_image(int sigma,int arr_dim){
    sigma = (double)sigma;
    int i,j;
    double **mat;
    double sum = 0.0;
    double r,s = 2*sigma*sigma;
     // init the gaussian matrix
    mat = malloc(arr_dim * sizeof(double*));
    for(i=0;i<arr_dim;i++)
        mat[i] = malloc(arr_dim * sizeof(double));

    for(i = -(arr_dim/2);i<=arr_dim/2;i++){
        for(j = -(arr_dim/2);j<=arr_dim/2;j++){
            r = sqrt(i*i+j*j);
            mat[i+arr_dim/2][j+arr_dim/2] = (exp(-(r*r)/s))/(PI * s);
            sum += mat[i+(arr_dim/2)][j+(arr_dim/2)];
        }
    }

    // normalizing the Kernel
    for (int i = 0; i < arr_dim; ++i)
        for (int j = 0; j < arr_dim; ++j)
            mat[i][j] /= sum;

    for(i=0;i<arr_dim;i++){
        for(j=0;j<arr_dim;j++)
            printf("%lf ",mat[i][j]);
        printf("\n");
    }

    return mat;
}

// start filtering
void gaussian_blur(PGM* p,double **kernal,int k_dim,char *output_file){
    PGM *output = malloc(sizeof(PGM));
    int i,j,x,y,max_val = 0;
    double sum = 0.0;
    /* Copy data to output PGM*/
    strcpy(output->pgm_type,p->pgm_type);
    output->width = p->width;
    output->height = p->height;
    output->max_value = p->max_value;

    output->data = malloc((output->height) * sizeof(int*));

    for(i=0;i<p->height;i++){
        output->data[i] = malloc((output->width) * sizeof(int));
        for(j=0;j<p->width;j++){
            output->data[i][j] = p->data[i][j];
        }
    }
    /* End Copy */
    printf("Starting convolution....\n");
    // convolution
    for(i=1;i<output->height-1;i++){
        for(j=1;j<output->width-1;j++){
            sum = 0.0;
            for(x=0;x<k_dim;x++){
                for(y=0;y<k_dim;y++){
                    if(i+x < output->height && j+y < output->width)
                        sum += (double)(output->data[i+x][y+j]) * (kernal[x][y]);
                }
            }

            // printf("sum: %d ",sum);
            if(i+(k_dim/2) < output->height && j+(k_dim/2) < output->width)
                output->data[i+(k_dim/2)][j+(k_dim/2)] = sum;
            
            if( output->data[i][j]>max_val)
                max_val = output->data[i][j];
        }
    }
    output->max_value = max_val;
    // normalize(output->width,output->height,output->data);
    write_image(output_file,output);

    free(p->data);
    free(output->data);
}

int main(){

    int mask_dim,sigma,i;
    PGM* p = malloc(sizeof(PGM));
    char output_filter_file[30];
    // char* file_in = "./images/lena.pgm";
    char* file_in = "./images/coins.ascii.pgm";
    p = read_pgm(p,file_in);
    // printImageDetails(p);

    /* Init the mask kernal matrix */
    double**kernal = malloc(3*sizeof(double*));
    for(i=0;i<3;i++)
        kernal[i] = malloc(3*sizeof(double));
    printf("Set the kernal dimension: ");scanf("%d",&mask_dim);
    printf("set the sigma value: ");scanf("%d",&sigma);
    printf("set th output file name: ");scanf("%s",output_filter_file);
    kernal = filter_image(sigma,mask_dim);
    gaussian_blur(p,kernal,mask_dim,output_filter_file);
    return 0;
}