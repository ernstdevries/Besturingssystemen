// -----------------------------------------------------
/** @file main.cc
 * This is the program that you have to finish.
 */
// -----------------------------------------------------

// Our own includes
#include "ansi.h"		// for: ansi color code strings
#include "asserts.h"	// for: notreached() e.a.
#include "unix_error.h"	// for: the unix_error() exception class
#include "cstr.h"		// for: the cstr() wrapper and it's output operator

// C/C++/STL specific
#include <iostream>	    // for: std::cout, std::endl
#include <ctime>		// for: ctime()
#include <cstdlib>		// for: EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>		// for: close() etc
#include <fcntl.h>		// for: O_WRONLY etc
#include <cstdio>       // for: printf()


// Our own classes
#include "Device.h"		// the "device driver"
#include "Block.h"		// the "data blocks"

using namespace std;
// ================================================================


void   printSuperblock(Device& device)
{
    string line = "---------------------------------------";
    // - - - - - - - - - - -
    // read SUPERBLOCK data
    // Also see: e7filsys.h
    // - - - - - - - - - - -
    // Fetch the block containing the super-block
    Block*  sp = device.getBlock(SUPERB);
    // We are going to use the u.fs view on it's contents
    // You can print the data with the printf function from C
    // which gives you precise control over the layout ...
    cout << "Dump of superblock on volume: " << cstr(sp->u.fs.s_fname,6) << "." << cstr(sp->u.fs.s_fpack,6)<< endl;
    cout << line << endl;
    cout << "userdata area starts in block: " << sp->u.fs.s_isize << endl;
    cout << "number of blocks on this volume is: " << sp->u.fs.s_fsize << endl;
    cout << "number of freeblocks in free[] is: " << sp->u.fs.s_nfree << " (max " << NICFREE << ")"<< endl;
    for (int i = 0; i < sp->u.fs.s_nfree; ++i) {
        cout << sp->u.fs.s_free[i] << " ";
    }
    cout << endl << "number of free inodes in inode[] is: " << sp->u.fs.s_ninode <<  " (max " << NICINOD << ")"<< endl;
    for (int i = 0; i < sp->u.fs.s_ninode; ++i) {
        cout << sp->u.fs.s_inode[i] << " ";
    }
    cout << endl << "freelist lock flag is: " << sp->u.fs.s_flock << endl;
    cout << "i-list lock flag is: " << sp->u.fs.s_ilock << endl;
    cout << "superblock-modified is: " << sp->u.fs.s_fmod<< endl;
    cout << "read-only flag is: " <<sp->u.fs.s_ronly << endl;
    cout << "last update time was: " << ctime(&sp->u.fs.s_time);
    cout << "total number of free blocks is: " << sp->u.fs.s_tfree<< endl;
    cout << "total number of free inodes is: " << sp->u.fs.s_tinode<< endl;
    cout << "Interleave factors are: m=" << sp->u.fs.s_m << " and n=" << sp->u.fs.s_n<< endl;
    cout << "File system name: " << cstr(sp->u.fs.s_fname,6)<< endl;
    cout << "File system pack: " <<cstr(sp->u.fs.s_fpack,6)<< endl;
    /*
        printf("printf     fs.s_isize=%d fs.s_fsize=%d\n",
                sp->u.fs.s_isize, sp->u.fs.s_fsize);
        // See Also: man 3 printf
        // ... or use the iostream operators from C++
        std::cout << "std::cout  fs.s_isize=" << sp->u.fs.s_isize
                  << " fs.s_fsize=" << sp->u.fs.s_fsize << std::endl;
        // The superblock-last-update timestamp can be
        // converted to human readable format with ctime()
        // (which always adds a newline character!) ...
        std::cout << "fs.s_time=" << ctime(&sp->u.fs.s_time);   // see: man 3 ctime
        // ... or use printf and limit the output to 24 chars
        printf("fs.s_time=%.24s\n", ctime(&sp->u.fs.s_time));   // see: man 3 ctime
        // Note: The time shown may differ from the given dump.txt file
        //       because of daylight savings time and different timezones.
        //       Internally all times are expressed in GMT.
        // Note: When on your system a time_t is 64 bit, you can not use
        //       ctime(&...) directly. Instead you have to use a little
        //       work-around. First assign the 32 bit time_x timestamp
        //       to a 64 bit time_t temporary:
        //          time_t  x = sp->u.fs.s_time;
        //       then use ctime on that temporary
        //          ... ctime(&x) ...
        // Because on the actual filesystem "strings" are not
        // guarantied to be null-terminated, simply printing
        // them you may risk printing your entire memory until
        // we somewhere run into a 0-byte.
        // To handle this problem a special C-string wrapper class
        //      cstr(const char[] text, unsigned maxlen)
        // with a suitable output operator is provided ...
        std::cout << "fs.s_fname=" << cstr(sp->u.fs.s_fname,6)
                  << ", fs.s_fpack=" << cstr(sp->u.fs.s_fpack,6) << std::endl;
        // .. or use printf again and tell it to stop after N characters.
        printf("fs.s_fname=%.6s, fs.s_fpack=%.6s\n",
                sp->u.fs.s_fname, sp->u.fs.s_fpack);
                */

    sp->release();  // We no longer need the super block

}
void    printRootinode(Device& device)
{
    // - - - - - - - - - - - - -
    // read INODE's from disk
    // Also see: e7ino.h
    // - - - - - - - - - - - - -
    // Fetch the block containing the root inode
    Block*  ip = device.getBlock( itod(ROOTINO) );
    // Going to use the u.dino[] view on the block.
    // Use a pointer variable as a shortcut to the wanted dinode in this block
    dinode* dp = &ip->u.dino[ itoo(ROOTINO) ];
    // Print the inode type + protection flags
    // If (di_mode == 0) then this inode is not used
    printf("inode %d, mode=%#o (expect 040777)\n", ROOTINO, dp->di_mode);
    // or
    std::cout << "inode " << ROOTINO << ", mode="
              // switching between octal/decimal notation
              << std::oct << dp->di_mode << std::dec
              << " (expect 040777)" << std::endl;
    // Verify this is indeed a directory inode using a "bit-mask test"
    if((dp->di_mode & X_IFMT) == X_IFDIR) {
        printf(AC_GREEN "Good: it is a directory\n"     AA_RESET);
    } else {
        printf(AC_RED   "Oops: it is not a directory\n" AA_RESET);
    }
    // Convert the 13, 24-bits, blocknumbers in the inode
    // to normal 32-bit daddr_x values (only valid for DIR or REG type!)
    daddr_x  diskaddrs[NADDR];              // provide some room to store those 13 blocknumbers
    Block::l3tol(diskaddrs, dp->di_addr);   // the data convertion routine
    std::cout << "diskaddr: ";
    for(auto addr : diskaddrs) {            // A C++ "range-base-for-loop"
        printf(" %d", addr);
    }
    std::cout << std::endl;
    ip->release();  // We no longer need this inode block
}

void    printFreeList(Device& device, daddr_x blockNumber)
{
    Block*  ip = device.getBlock( blockNumber );
    cout << endl <<  "Freeblock " << blockNumber << ": 50 (max "<< NICFREE <<")"<< endl;
    for (int i = 0; i < 50; ++i) {
        cout << ip->u.fb.df_free[i] << " ";
    }
    if (ip->u.fb.df_free[0] != 0) {
        printFreeList(device, ip->u.fb.df_free[0]);
    }
    ip->release();
}

void printFreeLists(Device& device)
{
    string line = "---------------------------------------";
    Block*  sp = device.getBlock(SUPERB);
    cout << "Rest of free list continues in block " << sp->u.fs.s_free[0];
    cout << endl << line;
    printFreeList(device, sp->u.fs.s_free[0]);
    cout << endl << line << endl;
    cout << "Examining "<< ((sp->u.fs.s_isize -2 ) * INOPB) << " winodes" << endl;
    cout<< line << endl;
    sp->release();

}

void printInode(int inodeNumber, dinode* dp){

     if (inodeNumber == 1 || dp->di_nlink != 0) {
                cout << inodeNumber << endl;
        }
}
void printInodes(Device& device)
{
    Block*  sp = device.getBlock(SUPERB);
    for (int i = 0; i < sp->u.fs.s_isize -2 ; ++i) {
        Block*  ip = device.getBlock( itod(ROOTINO) + i);
        for (int y = 1; y < INOPB+1; ++y) {
            int iNum = (i*8) + y;
            dinode* dp = &ip->u.dino[ itoo(y)];
            printInode(iNum, dp);


        }
    }

}



//
// TODO: write all the functions etc you need for this assignment
void	dump( const char* floppie )
{
    string line = "---------------------------------------";
    std::cout << "Opening device '" << floppie << "'" << std::endl;
    std::cout << line << std::endl;
    Device device(floppie);
    printSuperblock(device);
    cout << line << endl;
    printFreeLists(device);
    cout << endl << line << endl;
    printInodes(device);
    printRootinode(device);

}

// ================================================================

// Main is just the TUI
int  main(int argc, const char* argv[])
{

    try {
// Note: To get your output into file 'log.txt', change the 0 below into a 1
#if	1
        std::cerr << "Sending output to log.txt" << std::endl;
        close(1);
        if( open("log.txt", O_WRONLY|O_TRUNC|O_CREAT, 0666) < 0) {
            throw unix_error("log.txt");
        }
#endif
        // Tell cout to add a 0 or 0x prefix when printing in non-decimal notation
        std::cout << std::showbase;

        // Pass the given parameter or use the default filename ?
        dump((argc > 1) ? argv[1] : "floppie.img");
        return EXIT_SUCCESS;
    } catch(const unix_error& e) {
        std::cerr << AC_RED "SYSTEM: " << e.what() << AA_RESET << std::endl;
        return EXIT_FAILURE;
    } catch(const std::exception& e) {
        std::cerr << AC_RED "OOPS: " << e.what() << AA_RESET << std::endl;
        return EXIT_FAILURE;
    } catch(...) {
        std::cerr << AC_RED "OOPS: something went wrong" AA_RESET << std::endl;
        return EXIT_FAILURE;
    }
}


// vim:aw:ai:ts=4:

