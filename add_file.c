#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOOT_SECTOR_OFFSET 0

typedef struct {
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short total_sectors_short; // if zero, later field is used
    unsigned char media_descriptor;
    unsigned short fat_size_sectors;
    unsigned short sectors_per_track;
    unsigned short number_of_heads;
    unsigned int hidden_sectors;
    unsigned int total_sectors_long;
    
    unsigned char drive_number;
    unsigned char current_head;
    unsigned char boot_signature;
    unsigned int volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat16BootSector;

typedef struct {
    unsigned char filename[8];
    unsigned char ext[3];
    unsigned char attributes;
    unsigned char reserved[10];
    unsigned short modify_time;
    unsigned short modify_date;
    unsigned short starting_cluster;
    unsigned int file_size;
} __attribute((packed)) Fat16Entry;

char *name = "limerick";
char *ext = "txt";
char *short_poem = "The Owl and the Pussy-cat went to sea \
In a beautiful pea green boat, \
They took some honey, and plenty of money, \
Wrapped up in a five pound note.";

char *poem =
  "Shall I compare thee to a summer's day? \
Thou art more lovely and more temperate: \
Rough winds do shake the darling buds of May, \
And summer's lease hath all too short a date: \
Sometime too hot the eye of heaven shines, \
And often is his gold complexion dimm'd; \
And every fair from fair sometime declines, \
By chance, or nature's changing course, untrimm'd: \
But thy eternal summer shall not fade, \
Nor lose possession of that fair thou ow'st; \
Nor shall Death brag thou wander'st in his shade, \
When in eternal lines to time thou grow'st: \
So long as men can breathe, or eyes can see, \
So long lives this, and this gives life to thee.";


void print_file_info(Fat16Entry *entry) {
    switch(entry->filename[0]) {
    case 0x00:
        return; // unused entry
    case 0xE5:
        printf("Deleted file: [?%.7s.%.3s]\n", entry->filename+1, entry->ext);
        return;
    case 0x05:
        printf("File starting with 0xE5: [%c%.7s.%.3s]\n", 0xE5, entry->filename+1, entry->ext);
        break;
    case 0x2E:
        printf("Directory: [%.8s.%.3s]\n", entry->filename, entry->ext);
        break;
    default:
        printf("File: [%.8s.%.3s]\n", entry->filename, entry->ext);
    }
    
    printf("  Modified: %04d-%02d-%02d %02d:%02d.%02d    Start: [%04X]    Size: %d\n", 
        1980 + (entry->modify_date >> 9), (entry->modify_date >> 5) & 0xF, entry->modify_date & 0x1F,
        (entry->modify_time >> 11), (entry->modify_time >> 5) & 0x3F, entry->modify_time & 0x1F,
        entry->starting_cluster, entry->file_size);
}

int main() {
    FILE * in = fopen("example.img", "r+b");
    int i, n;
    char fat_ptr[2];
    Fat16BootSector bs;
    Fat16Entry *entry;

    printf("boot sector struct size %lu\n", sizeof(Fat16BootSector));
    
    fseek(in, BOOT_SECTOR_OFFSET, SEEK_SET);
    fread(&bs, sizeof(Fat16BootSector), 1, in);
    
    printf("Now at 0x%lX, sector size %d, FAT size %d sectors, %d FATs\n\n", 
           ftell(in), bs.sector_size, bs.fat_size_sectors, bs.number_of_fats);
           


    // add a new entry as 1st file in root directory
    // first create FAT root dir entry
    printf("creating root dir entry\n");
    entry = (Fat16Entry*)calloc(1, sizeof(Fat16Entry));
    // (1) TODO fill in the data fields for this entry

    // Now we write this entry into the root directory (first slot)
    // move to place in root dir
    fseek(in, (bs.reserved_sectors + bs.fat_size_sectors * bs.number_of_fats) *
          bs.sector_size, SEEK_SET);
    fwrite(entry, sizeof(Fat16Entry), 1, in);
    printf("written root dir entry\n");


    
    // then allocate file into data cluster 2
    // so write poem string here...
    printf("writing to data cluster\n");
    fseek(in, (1/*rootdir*/ + bs.reserved_sectors + bs.fat_size_sectors * bs.number_of_fats) *bs.sector_size, SEEK_SET);
    fwrite(short_poem, sizeof(char), strlen(short_poem), in);
    printf("written data\n");
    
    // then update FAT entries for data cluster 2
    n = 2;
    printf("updating FAT\n");
    fseek(in, sizeof(bs), SEEK_SET); // seek past end of boot sector
    fseek(in, n*2*sizeof(char), SEEK_CUR);  // seek to correct FAT entry
    // (2) TODO - what values do we write into the 
    // FAT for this single-sector file?
    ////fat_ptr[0] = ???
    ////fat_ptr[1] = ???
    fwrite(fat_ptr, sizeof(char), 2, in);
    printf("updated FAT\n");

    fclose(in);
    return 0;
}
