#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"

int main (int argc, char * argv[])
{
    FILE * infile=NULL;
    char buffer[MAX_MAP_SIZE+5][MAX_MAP_SIZE+5];
    
    memset(&buffer, '\0', sizeof(buffer));
    
    universe u;
    init_universe(&u);
    
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        exit(1);
    }
    
    fprintf(stderr, "Opening file %s\n", argv[1]);
    
    infile=fopen(argv[1], "r");
    if (infile == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        exit(1);
    }
    
    int file_row=0;
    int file_col=0;
    
    while (fgets(buffer[file_row], sizeof(buffer[file_row]), infile))
    {
        char * eol = strchr(buffer[file_row], '\r');
        if (eol)
        {
            *eol='\0';
        }
        eol = strchr(buffer[file_row], '\n');
        if (eol)
        {
            *eol='\0';
        }
        
        file_col=strlen(buffer[file_row]);
        file_row++;
    }
    
    fclose(infile);
    
    printf("done parsing infile\n");
    
    // file_row contains the number of rows in the file. we need to load from 2 to file_row-3
    //   if there were 10 rows, we load 2,3,4,5,6,7. skipping 0 and 1 for top portal names and 8 and 9 for bottom portal names
    // file_col contains the number of columns in the file. we need to load from 2 to file_col-3
    //   if there were 10 cols, we load 2,3,4,5,6,7. skipping 0 and 1 for left portal names and 8 and 9 for right portal names.
    
    init_map(&u.myMap, file_row-4, file_col-4);
    
    for (int row=2; row<=file_row-3; row++)
    {
        for (int col=2; col<file_col-2; col++)
        {
            char ch=buffer[row][col];
            if (!((ch==MAP_WALL)||(ch==MAP_PASSAGE)))
                ch=MAP_BLANK;
            u.myMap.layout[row-2][col-2].display_ch=ch;
        }
    }
    
    segmentize(&u);
    
    
    
    return 0;
}
