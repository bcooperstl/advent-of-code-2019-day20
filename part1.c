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
    
    // looking for the letters next to the dots for the portal names. can skip first and last column and rows because all names are two chars
        
    for (int row=1; row<file_row-1; row++)
    {
        for (int col=1; col<file_col-1; col++)
        {
            if (buffer[row][col]>='A' && buffer[row][col]<='Z')
            {
                char portal_name[3];
                int should_add=0;
                int dest_row = 0;
                int dest_col = 0;
                // portal dot below. this character is the second character of the name, and the first character is above this one
                if (buffer[row+1][col]==MAP_PASSAGE)
                {
                    portal_name[0]=buffer[row-1][col];
                    portal_name[1]=buffer[row][col];
                    portal_name[2]='\0';
                    // need to set this name on the below cell.
                    //   row offset is -2+1=-1 - need -2 for the same row and then +1 to go down one row
                    //   col offset is -2 to keep the same column.
                    should_add=1;
                    dest_row=row-1;
                    dest_col=col-2;
                }
                // portal dot above. this character is the first character of the name, and then second character is below this one
                else if (buffer[row-1][col]==MAP_PASSAGE)
                {
                    portal_name[0]=buffer[row][col];
                    portal_name[1]=buffer[row+1][col];
                    portal_name[2]='\0';
                    // need to set this name on the above cell.
                    //   row offset is -2-1=-3 - need -2 for the same row and then -1 to go up one row
                    //   col offset is -2 to keep the same column.
                    should_add=1;
                    dest_row=row-3;
                    dest_col=col-2;
                }
                // portal dot right. this character is the second character of the name, and the first character is left of this one
                else if (buffer[row][col+1]==MAP_PASSAGE)
                {
                    portal_name[0]=buffer[row][col-1];
                    portal_name[1]=buffer[row][col];
                    portal_name[2]='\0';
                    // need to set this name on the below cell.
                    //   row offset is -2 to keep the same row.
                    //   col offset is -2+1=-1 - need -2 for the same column and then +1 to go right one row.
                    should_add=1;
                    dest_row=row-2;
                    dest_col=col-1;
                }
                // portal dot left. this character is the first character of the name, and then second character is right of this one
                else if (buffer[row][col-1]==MAP_PASSAGE)
                {
                    portal_name[0]=buffer[row][col];
                    portal_name[1]=buffer[row][col+1];
                    portal_name[2]='\0';
                    // need to set this name on the above cell.
                    //   row offset is -2 to keep the same row.
                    //   col offset is -2-1=-3 - need -2 for the same column and then -1 to go left one row
                    should_add=1;
                    dest_row=row-2;
                    dest_col=col-3;
                }
                
                if (should_add)
                {
                    printf("setting row=%d col=%d to be portal %s\n", dest_row, dest_col, portal_name);
                    u.myMap.layout[dest_row][dest_col].is_portal=1;
                    strncpy(u.myMap.layout[dest_row][dest_col].portal, portal_name, PORTAL_LENGTH+1);
                }
            }
        }
    }
    
    display_map(&u.myMap, WITH_PORTALS);
    
    return 0;
}
