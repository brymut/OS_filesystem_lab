#!/usr/bin/python

# gen_fat_fs.py
# Jeremy Singer
# 26 Feb 2017

## simple python script to create an example, empty FAT
## filesystem image, from scratch

image = 'example.img'  # or use an argument from command-line?

# create (or overwrite) the file and start writing bytes  (wb) into it
f = open(image, 'wb')


###################
# BOOT SECTOR, 512B
###################
# first three bytes of boot sector are 'magic value'
f.write( bytearray([0xeb, 0x3c, 0x90]) )

# next 8 bytes are manufacturer name, in ASCII
f.write( 'GLASGOW '.encode('ascii') )

# next 2 bytes are bytes per block - 512 is standard
# this is in little endian format - so 0x200 is  0x00, 0x02
# A block == A sector in FAT terminology
f.write( bytearray([0x00, 0x02]) )

# next byte, number of blocks per allocation unit - we say 1 for simplicity
# An allocation unit == A cluster in FAT terminology
f.write( bytearray([0x01]) )

# next two bytes, number of reserved blocks - we
# say 1 for boot sector only
f.write( bytearray([0x01, 0x00]) )

# next byte, number of File Allocation tables - can have multiple
# tables for redundancy - we'll stick with 1 for now
f.write( bytearray([0x01]) )

# next two bytes, number of root directory entries - including blanks
# let's say 16 files in root for now - which means the root directory
# will occupy a single sector
f.write( bytearray([0x10, 0x00]) )

# next two bytes, number of blocks in the entire disk - we want a 4 meg disk,
# so need 8192 0.5K blocks == 2^13 == 0x00 0x20
f.write( bytearray([0x00, 0x20]) )

# single byte media descriptor - magic value 0xf8??
f.write( bytearray([0xf8]) )

# next two bytes, number of blocks for FAT 
# FAT needs two bytes per block, we have 8192 blocks on disk
# 512 bytes per block - i.e. can store FAT metadata for 256 blocks in
# a single block, so need 8192/256 blocks == 2^13/2^8 == 2^5 == 32
f.write( bytearray([0x20, 0x00]) )

# next 8 bytes are legacy values, can all be 0
f.write( bytearray([0,0,0,0,0,0,0,0]) )

# next 4 bytes are total number of blocks in entire disk - if it overflows
# previous 2 byte entry ... otherwise leave at 0
f.write( bytearray([0x00, 0x00, 0x00, 0x00]) )

# next 2 bytes are legacy values
f.write( bytearray([0x80,0]) )

# magic value 29 - for FAT16 extended signature
f.write( bytearray([0x29]) )

# next 4 bytes are volume serial number (unique id)
f.write( bytearray([0x41,0x42,0x43,0x44]) )

# next 11 bytes are volume label (name) - pad with trailing spaces
f.write( "TEST_DISK  ".encode('ascii'))

# next 8 bytes are file system identifier - pad with trailing spaces
f.write( "FAT16   ".encode('ascii'))

# pad with '\0'
for i in range(0,0x1c0):
    f.write( bytearray([0]) )

# end of boot sector magic marker
f.write(  bytearray([0x55, 0xaa]) )

############
# (1) Now write the file allocation tablea
#############
# each entry needs 2 bytes for FAT16
# We need 8192 entries (== 32 blocks of 512B)

# (a) first two entries are magic value 0xf8 0xff 0xff 0xff

# (b) all subsequent FAT entries should be 0x00 0x00

############
# (2) next sector is root directory
###########
# There are 16 files in the root directory
# Each file entry occupies 32 bytes - we just no entries for now - all zeros.
# Root directory takes 16*32 bytes == 512B == 1 sector


#############
# (3) next sector is data region

# create 8192 blank sectors, each containing 512 bytes of zero values


###
# finally close file
f.close()


# Now, in bash, you should be able to mount this file
# (a) either
#    sudo mount -t vfat -o loop example.img /mnt
# or (b) write the file out to an unmounted USB disk
#    dd if=example.img of=/dev/USBDISK bs=512





