#ifdef BUILD_MAC

#include "mediaTypeDCD.h"

#include <cstring>
#include "utils.h"
#include "../../include/debug.h"

bool MediaTypeDCD::read(uint32_t blockNum, uint8_t* buffer)
{
    size_t readsize = 512;//_media_sector_size;
if ((blockNum == 0) || (blockNum != last_block_num + 1)) // example optimization, only do seek if not reading next block -tschak
  {
     if (fseek(_media_fileh, (blockNum * readsize) + offset, SEEK_SET))
    {
        reset_seek_opto();
        return true;
    }
  }

//   if (high_score_enabled && blockNum >= _high_score_block_lb && blockNum <= _high_score_block_ub)
//     last_block_num = INVALID_SECTOR_VALUE; // try to invalidate cache if game re-reads hs table
//   else
    last_block_num = blockNum;

  readsize = fread((unsigned char *)buffer, 1, readsize, _media_fileh); // Reading block from SD Card
  return (readsize != _media_sector_size);
}

bool MediaTypeDCD::write(uint32_t blockNum, uint8_t* buffer)
{
    size_t writesize = _media_sector_size;

    // if (high_score_enabled && blockNum >= _high_score_block_lb && blockNum <= _high_score_block_ub)
    // {
    //     Debug_printf("high score: Swapping file handles\r\n");
    //     oldFileh = _media_fileh;
    //     hsFileh = _media_host->file_open(_disk_filename, _disk_filename, strlen(_disk_filename) +1, "r+");
    //     _media_fileh = hsFileh;
    // }

    if (blockNum != last_block_num + 1) // example optimization, only do seek if not writing next block -tschak
    {
         if (fseek(_media_fileh, (blockNum * writesize) + offset, SEEK_SET))
        {
            reset_seek_opto();
            return true;
        }
    }
    last_block_num = blockNum;
    writesize = fwrite((unsigned char *)buffer, 1, writesize, _media_fileh);
    if (writesize != _media_sector_size)
    {
       reset_seek_opto();
       return true;
    }

    // if (high_score_enabled && blockNum >= _high_score_block_lb && blockNum <= _high_score_block_ub)
    // {
    //     Debug_printf("high score: Reverting file handles.\r\n");
    //     if (hsFileh != nullptr)
    //         fclose(hsFileh);

    //     _media_fileh = oldFileh;
    //     last_block_num = INVALID_SECTOR_VALUE; // Invalidate cache
    // }

    return false;
}

bool MediaTypeDCD::format(uint16_t *responsesize)
{
    return false;
}

mediatype_t MediaTypeDCD::mount(FILE *f, uint32_t disksize)
{
    // diskiiemulation = false;
    // char hdr[64];
    // fread(&hdr,sizeof(char),64,f);
    // if (hdr[0] == '2' && hdr[1] == 'I' && hdr[2] == 'M' && hdr[3] == 'G')
    // {
    //     // check for 'high score enabled'
    //     // expected format at offset 0x48 of 2mg hdr: H,I,<blk_lb_lo>,<blk_lb_hi>,<blk_ub_lo>,<blk_ub_hi>
    //     // where blk is block range containing high score table, lb = lower bound, ub = upper bound
    //     // if upper bound is undefined (0x0000), then lower bound defines range
    //     if (hdr[48] == 'H' && hdr[49] == 'I')
    //     {
    //         // read range of blocks from the 2mg header that qualify to be written (little endian)
    //         _high_score_block_lb = UINT16_FROM_HILOBYTES(hdr[51], hdr[50]);
    //         _high_score_block_ub = UINT16_FROM_HILOBYTES(hdr[53], hdr[52]);
    //         if (_high_score_block_lb > 0)
    //         {
    //             if (_high_score_block_ub == 0)
    //                 _high_score_block_ub = _high_score_block_lb;

    //             high_score_enabled = true;
    //             Debug_printf("\r\nhigh score: block range: %04x..%04x\r\n", _high_score_block_lb, _high_score_block_ub);
    //         }
    //     }
    //     offset = 64;
    // }
    offset = 0;
    _media_fileh = f;
    disksize -= offset;
    _media_sector_size = 512;
    num_blocks = disksize / _media_sector_size;
    return MEDIATYPE_DCD;
}


// static bool create(FILE *f, uint32_t numBlock)
// {
//     return false;
// }

#endif // BUILD_APPLE
