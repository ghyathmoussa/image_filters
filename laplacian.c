#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// pgm image struct
typedef struct pgm{
    char pgm_type[3];
    int width;
    int height;
    int max_value;
    int** data;
}PGM;


// ignore comments in pgm image
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

// read pgm image and load data to struct
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
    fscanf(pgm, "%d ", &(p->max_value));
    ignore_comments(pgm);

    // Allocation memory to store data of image

    printf("width,height: %d %d\n",p->width,p->height);
    printf("%s",p->pgm_type);
    p->data = malloc(p->height * sizeof(int*));
    for(i=0;i<p->height;i++)
        p->data[i]= malloc(p->width * sizeof(int));
    
    // if pgm version is P5
    if(strcmp(p->pgm_type,"P5") == 0){
        char* buffer;
        int buffer_size = p->width * p->height;
        buffer = (char*) malloc((buffer_size+1) * sizeof(char));

        if(buffer == NULL){
            printf("can not read memory for buffer in version P5\n");
            exit(0);
        }

        fread(buffer,sizeof(char),p->width * p->height,pgm);
        for(i=0;i<p->width * p->height;i++)
            p->data[i/(p->width)][i%p->width] = buffer[i];
        free(buffer);
    }else if(strcmp(p->pgm_type,"P2") == 0){
        // Storing the pixel info in
        // the struct
        for (i = 0;i < p->height; i++) {
            
            for(j=0;j<p->width;j++)
                fscanf(pgm,"%d ",&(p->data[i][j]));
        }
    }else{
        printf("Unsupported pgm version\n");
        exit(0);
    }

    fclose(pgm);
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
        printf("PGM File Format:Binary\n");
 
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
    int i,j, max_val = 0;
    FILE *fp;
    printf("Writing....\n");
    fp = fopen(file_name,"wb");
    if (fp == NULL){
        printf("Can not open file to write\n");
        exit(-1);
    }

    // find max value

    for(i=0;i<p->height;i++){
        for(j=0;j<p->width;j++){
            if(p->data[i][j] > max_val)
                max_val = p->data[i][j];
        }
     }

    fprintf(fp,"%s\n",p->pgm_type);
    fprintf(fp,"#created by Ghyath Moussa\n");
    fprintf(fp,"%d %d\n",p->width,p->height);
    fprintf(fp,"%d\n",max_val);
    // write image
    for(i=0;i<p->height;i++){
        for(j=0;j<p->width;j++){
            fprintf(fp,"%d ",p->data[i][j]);
        }
        fprintf(fp,"\n");
    }
    fclose(fp);
}

// convolution operation
int conv(PGM* p,int kernal[3][3],int row_no,int col_no){
    int i,j,sum=0;
    for(i=0;i<3;i++)
        for(j=0;j<3;j++)
            sum += p->data[i+row_no][j+col_no] * kernal[i][j];

    return sum;
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

// start laplacian filter
// you can change the filter as you want 
// or load it dynamicly from user by take it from terminal :)
void laplacian(PGM* p,char* file_out){
    int i,j;
    int kernal[3][3] = {
        {0,-1,0},
        {-1,4,-1},
        {0,-1,0}
    };
    PGM *output = malloc(sizeof(PGM));
    // Copy PGM image 
    strcpy(output->pgm_type,p->pgm_type);
    output->width = p->width;
    output->height = p->height;

    output->data = malloc((output->height) * sizeof(int*));

    for(i=0;i<output->height;i++)
        output->data[i] = malloc((output->width) * sizeof(int));
    
    for(i=0;i<p->height;i++){
        for(j=0;j<p->width;j++)
            output->data[i][j] = p->data[i][j];
        
        
    }
    /* End Copy */

    for(i=1;i<output->height-2;i++){
        for(j=1;j<output->width-2;j++){
            output->data[i][j] = conv(output,kernal,i,j);
        }
    }

    // // normalize data
    normalize(output->width,output->height,output->data);
    write_image(file_out,output);

    free(p->data);
    free(output->data);
}

int main(){
    PGM* p = malloc(sizeof(PGM));;
    char file_in[30];
    char file_out[30];
    printf("name or path of the input file: ");scanf("%s",file_in);
    printf("name or path of the output file: ");scanf("%s",file_out);
    p = read_pgm(p,file_in);
    printImageDetails(p);
    printf("Reading from file %s\n",file_in);
    laplacian(p,file_out);
    printf("Writing to file %s\n",file_out);
    return 0;
}