#define ZLIB_WINAPI   // actually actually needed (for linkage)
#include "zlib.h"     // declare the external fns -- uses zconf.h, too
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned char U8;

int getMaxCompressedLen( int nLenSrc ) {
    int n16kBlocks = (nLenSrc+16383) / 16384; // round up any fraction of a block
    return ( nLenSrc + 6 + (n16kBlocks*5) );
}

int compressData( const U8* abSrc, int nLenSrc, U8* abDst, int nLenDst ) {
    z_stream zInfo ={0};
    zInfo.total_in=  zInfo.avail_in=  nLenSrc;
    zInfo.total_out= zInfo.avail_out= nLenDst;
    zInfo.next_in= (U8*)abSrc;
    zInfo.next_out= abDst;

    int nErr, nRet= -1;
    nErr= deflateInit( &zInfo, Z_DEFAULT_COMPRESSION ); // zlib function
    if ( nErr == Z_OK ) {
        nErr= deflate( &zInfo, Z_FINISH );              // zlib function
        if ( nErr == Z_STREAM_END ) 
            nRet= zInfo.total_out;
    }
    deflateEnd( &zInfo );    // zlib function
    return( nRet );
}

int main() {
    U8 pbSrc[]="hello hello hello hello there";

    int nLenOrig= strlen( (char*)pbSrc )+1; // include terminating NULL
    int nLenDst= getMaxCompressedLen( nLenOrig );
    U8 *pbDst = malloc(nLenDst);
    if (!pbDst) return 2;

    int nLenPacked= compressData( pbSrc, nLenOrig, pbDst, nLenDst );
    if ( nLenPacked == -1 ) return(1);  

    free(pbDst);
    return 0;
}
