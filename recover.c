/*Wes Collatz
CS 46 Fall 2015 
CSI - 10/6/2015*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

unsigned char * read_card(char fname[], int *size);
void save_jpeg(unsigned char data[], int size, char fname[]);
void recover(unsigned char data[], int size);

int main()
{

    int card_length;
    unsigned char *card = read_card("card.raw", &card_length);
    
    recover(card, card_length);
}

unsigned char * read_card(char fname[], int *size)
{

    struct stat st;
    if (stat(fname, &st) == -1)
    {
        fprintf(stderr, "Can't get info about %s\n", fname);
        exit(1);
    }
    int len = st.st_size;
    unsigned char *raw = (unsigned char *)malloc(len * sizeof(int));
    
    FILE *fp = fopen(fname, "rb");
    if (!fp)
    {
        fprintf(stderr, "Can't open %s for reading\n", fname);
        exit(1);
    }
    
    char buf[512];
    int r = 0;
    while (fread(buf, 1, 512, fp))
    {
        for (int i = 0; i < 512; i++)
        {
            raw[r] = buf[i];
            r++;
        }
    }
    fclose(fp);
    
    
    *size = len;                                
    return raw;                                 
}


void recover(unsigned char data[], int size) {
    
    int masterQty = 0;                          
   
    // first loop through to count jpegs
    for (int byt = 0; byt < size; byt = byt + 512)  {
        
        if (data[byt] == 0xff && data[byt+1] == 0xd8 && data[byt+2] == 0xff && 
           (data[byt+3] == 0xe1 || data[byt+3] == 0xe1)) {   

            masterQty++;                              
        }
    }
                                     
    int *begBytes = (int *)malloc(masterQty * sizeof(int));
    for (int i = 0; i < masterQty; i++) {
        begBytes[i] = 0;
    }
    
    // second loop to get the beginning of each jpeg
    int count = 0;    
    
    for (int byt = 0; byt < size-512; byt = byt + 512)  {
                
        if (data[byt] == 0xff && data[byt+1] == 0xd8 && data[byt+2] == 0xff && 
        (data[byt+3] == 0xe0 || data[byt+3] == 0xe1)) {   
                                                     
            begBytes[count] = byt;                     
            count++; 
        }
    } 
          
    int *endBytes = (int *)malloc(masterQty * sizeof(int));
    
    for (int i = 0; i < masterQty; i++) {
        endBytes[i] = 0;
    } 
    // loop to get the end of each jpeg (except the final one)
    for (int i = 0; i < masterQty - 1; i++ ){
        
        int end = 0;    
        
        for (int j = begBytes[i + 1]-512; end != 1; j++) {
       
            if (data[j] == 0xff && data[j+1] == 0xd9) {
           
                endBytes[i] = j;
                end = 1;
            }  
        }
    }
    
    // this loop will get the end of the final image    
    int end = 0; 
                                
    for (int i = size; end != 1; i--) {
              
        if (data[i] == 0xff && data[i + 1] == 0xd9) {
       
            endBytes[masterQty - 1] = i;
            end = 1;
        }             
    } 
    
    // loop to load files up with the data     
    for (int i = 0; i < masterQty; i++) {   
    
        int length = (endBytes[i] - begBytes[i]); 
     
        unsigned char *imgData = (unsigned char *)malloc(length * sizeof(char));
        char *filename = (char *)malloc((7 * sizeof(char) + 1));

        for (int dmp = begBytes[i]; dmp <= endBytes[i]; dmp++) {        
            imgData[dmp - begBytes[i]] = data[dmp];
        }
         
        if (i > 8) {
            sprintf( filename, "0%d%s", i + 1, ".jpg");
        }
        else {
            sprintf( filename, "00%d%s", i + 1, ".jpg");
        }
        save_jpeg(imgData, length, filename);
    }
}

void save_jpeg(unsigned char data[], int size, char fname[])
{
    FILE *fp = fopen(fname, "wb");
    if (!fp)
    {
        fprintf(stderr, "Can't write to %s\n", fname);
        exit(1);
    }
    
    fwrite(data, 1, size, fp);
    fclose(fp);   
}
