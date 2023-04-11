#ifdef BUILD_APPLE

#include "mediaTypeWOZ.h"
#include "../../include/debug.h"
#include <string.h>

#define WOZ1 '1'
#define WOZ2 '2'

mediatype_t MediaTypeWOZ::mount(FILE *f, uint32_t disksize)
{
    _media_fileh = f;
    diskiiemulation = true;
    // check WOZ header
    if (wozX_check_header())
        return MEDIATYPE_UNKNOWN;

    // work through INFO chunk
    if (wozX_read_info())
        return MEDIATYPE_UNKNOWN;

    if (wozX_read_tmap())
        return MEDIATYPE_UNKNOWN;
        
    // read TRKS table
    switch (woz_version)
    {
    case WOZ1:
        if (woz1_read_tracks())
            return MEDIATYPE_UNKNOWN;
        break;
    case WOZ2:
        if (woz2_read_tracks())
            return MEDIATYPE_UNKNOWN;
        break;
    default:
        Debug_printf("\nWOZ version %c not supported", woz_version);
        return MEDIATYPE_UNKNOWN;
    }

    return MEDIATYPE_WOZ;
}

void MediaTypeWOZ::unmount()
{
    MediaType::unmount();
    for (int i = 0; i < MAX_TRACKS; i++)
    {
        if (trk_ptrs[i] != nullptr)
            free(trk_ptrs[i]);
    }
}

bool MediaTypeWOZ::wozX_check_header()
{
    char hdr[12];
    fread(&hdr, sizeof(char), 12, _media_fileh);
    if (hdr[0] == 'W' && hdr[1] == 'O' && hdr[2] == 'Z')
    {
        woz_version = hdr[3];
        Debug_printf("\nWOZ-%c file confirmed!",woz_version);
    }
    else
    {
        Debug_printf("\nNot a WOZ file!");
        return true;
    }
    // check for file integrity
    if (hdr[4] == 0xFF && hdr[5] == 0x0A && hdr[6] == 0x0D && hdr[7] == 0x0A)
        Debug_printf("\n8-bit binary file verified");
    else
        return true;

    // could check CRC if one wanted
    
    return false;  
}

// TODO: move to another place
disk_diameter_t MediaTypeWOZ::read_woz_diameter(FILE *f)
{
    disk_diameter_t ret = disk_diameter_t::WOZ_UNKNOWN;;
    fseek(f, 21, SEEK_SET);
    uint8_t t = fgetc(f);
    if (t == 1)
        ret = disk_diameter_t::WOZ_525;
    else if (t == 2)
        ret = disk_diameter_t::WOZ_35;

#ifdef DEBUG
    if (ret == disk_diameter_t::WOZ_UNKNOWN)
        Debug_printf("\rUNKNOWN disk diameter in WOZ file");
#endif

    fseek(f, 0, SEEK_SET);

    return ret;
}

bool MediaTypeWOZ::wozX_read_info()
{
    /*
     Byte	Offset	Type	Vers	Name	Usage
     12		        uint32		    ‘INFO’ Chunk ID	0x4F464E49
     16		        uint32		    Chunk Size	Size is always 60.
     20	    +0	    uint8	  1	    INFO Version	Version number of the INFO chunk. Current version is 3.
     21	    +1	    uint8	  1	    Disk Type   1 = 5.25, 2 = 3.5
    */
    if (fseek(_media_fileh, 12, SEEK_SET))
    {
        Debug_printf("\nError seeking INFO chunk");
        return true;
    }
    uint32_t _chunk_id, _chunk_size;
    // byte 12
    fread(&_chunk_id, sizeof(_chunk_id), 1, _media_fileh);
    Debug_printf("\nINFO Chunk ID: %08x", _chunk_id);
    // byte 16
    fread(&_chunk_size, sizeof(_chunk_size), 1, _media_fileh);
    Debug_printf("\nINFO Chunk size: %d", _chunk_size);

    uint8_t _infoversion, _disktype;
    // byte 20
    fread(&_infoversion, sizeof(_infoversion), 1, _media_fileh);
    // byte 21
    fread(&_disktype, sizeof(_disktype), 1, _media_fileh);
    if (_disktype == 1)
        disk_type = disk_diameter_t::WOZ_525;
    else if (_disktype == 2)
        disk_type = disk_diameter_t::WOZ_35;
    else
    {    
        disk_type = disk_diameter_t::WOZ_UNKNOWN;
        Debug_printf("\rdisk type unknown: %d", disk_type);
        return true;
    }
    Debug_printf("\nNow at byte %d", ftell(_media_fileh));
    // could read a whole bunch of other stuff  ...

    switch (woz_version)
    {
    case WOZ1:
        num_blocks = WOZ1_NUM_BLKS; //WOZ1_TRACK_LEN / 512;
        optimal_bit_timing = WOZ1_BIT_TIME; // 4 x 8 x 125 ns = 4 us
        number_of_sides = 1;
        if (disk_type == disk_diameter_t::WOZ_35)
            number_of_sides = 2;
        break;
    case WOZ2:
        // jump to read number of sides (offset 37 but jumping from offset 2)
        fseek(_media_fileh, 35, SEEK_CUR);
        number_of_sides = fgetc(_media_fileh);
        Debug_printf("\nNumber of sides: %d", number_of_sides);
        // jump to offset 39 to get bit timing (from offset 37)
        fgetc(_media_fileh);
        optimal_bit_timing = fgetc(_media_fileh);
        Debug_printf("\nWOZ2 Optimal Bit Timing = 125 ns X %d = %d ns",optimal_bit_timing, (int)optimal_bit_timing * 125);
        {
            // and jump to offset 44 to get the track size
            fseek(_media_fileh, 4, SEEK_CUR);
            uint16_t largest_track;
            fread(&largest_track, sizeof(uint16_t), 1, _media_fileh);
            num_blocks = largest_track;
        }
        break;
    default:
        Debug_printf("\nWOZ version %c not supported", woz_version);
        return true;
    }

    return false;
}

bool MediaTypeWOZ::wozX_read_tmap()
{ // read TMAP, no change for 3.5" disk
    if (fseek(_media_fileh, 80, SEEK_SET))
    {
        Debug_printf("\nError seeking TMAP chunk");
        return true;
    }

    uint32_t chunk_id, chunk_size;
    fread(&chunk_id, sizeof(chunk_id), 1, _media_fileh);
    Debug_printf("\nTMAP Chunk ID: %08x", chunk_id);
    fread(&chunk_size, sizeof(chunk_size), 1, _media_fileh);
    Debug_printf("\nTMAP Chunk size: %d", chunk_size);
    Debug_printf("\nNow at byte %d", ftell(_media_fileh));

    fread(&tmap, sizeof(tmap[0]), MAX_TRACKS, _media_fileh);
#ifdef DEBUG
    Debug_printf("\nTrack, Index");
    for (int i = 0; i < MAX_TRACKS; i++)
        Debug_printf("\n%d/4, %d", i, tmap[i]);
#endif

    return false;
}

bool MediaTypeWOZ::woz1_read_tracks()
{    // depend upon little endian-ness, seems like no change for 3.5"
    fseek(_media_fileh, 256, SEEK_SET);

    // woz1 track data organized as:
    // Offset	Size	    Name	        Usage
    // +0	    6646 bytes  Bitstream	    The bitstream data padded out to 6646 bytes
    // +6646	uint16	    Bytes Used	    The actual byte count for the bitstream.
    // +6648	uint16	    Bit Count	    The number of bits in the bitstream.
    // +6650	uint16	    Splice Point	Index of first bit after track splice (write hint). If no splice information is provided, then will be 0xFFFF.
    // +6652	uint8	    Splice Nibble	Nibble value to use for splice (write hint).
    // +6653	uint8	    Splice Bit Count	Bit count of splice nibble (write hint).
    // +6654	uint16		Reserved for future use.

    Debug_printf("\nStart Block, Block Count, Bit Count");
    
    uint8_t *temp_ptr = (uint8_t *)heap_caps_malloc(WOZ1_NUM_BLKS * 512, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    uint16_t bytes_used;
    uint16_t bit_count;

    for (int i = 0; i < MAX_TRACKS; i++)
    {
        memset(temp_ptr, 0, WOZ1_NUM_BLKS * 512);
        fread(temp_ptr, 1, WOZ1_TRACK_LEN, _media_fileh);
        fread(&bytes_used, sizeof(bytes_used), 1, _media_fileh);
        fread(&bit_count, sizeof(bit_count), 1, _media_fileh);
        trks[i].block_count = bytes_used / 512;
        if (bytes_used % 512)
            trks[i].block_count++;
        trks[i].bit_count = bit_count;
        if (bit_count > 0)
        {
            size_t s = trks[i].block_count * 512;
            trk_ptrs[i] = (uint8_t *)heap_caps_malloc(s, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
            if (trk_ptrs[i] != nullptr)
            {
                Debug_printf("\nStoring %d bytes of track %d into location %lu", bytes_used, i, trk_ptrs[i]);
                memset(trk_ptrs[i],0,s);
                memcpy(trk_ptrs[i], temp_ptr, bytes_used);
            }
            else
            {
                Debug_printf("\nNo RAM allocated!");
                free(temp_ptr);
                return true;
            }
        }
        else
        {
            trk_ptrs[i] = nullptr;
            Debug_printf("\nTrack %d is blank!",i);
        }
        fread(temp_ptr, 1, 6, _media_fileh); // read through rest of bytes in track
    }
    free(temp_ptr);
    return false;
}

bool MediaTypeWOZ::woz2_read_tracks()
{    // depend upon little endian-ness, looks like no change for 3.5" disk
    fseek(_media_fileh, 256, SEEK_SET);
    fread(&trks, sizeof(TRK_t), MAX_TRACKS, _media_fileh);
#ifdef DEBUG
    Debug_printf("\nStart Block, Block Count, Bit Count");
    for (int i=0; i<MAX_TRACKS; i++)
        Debug_printf("\n%d, %d, %lu", trks[i].start_block, trks[i].block_count, trks[i].bit_count);
#endif
    // read WOZ tracks into RAM
    for (int i=0; i<MAX_TRACKS; i++)
    {
        size_t s = trks[i].block_count * 512;
        if (s != 0)
        {
            trk_ptrs[i] = (uint8_t *)heap_caps_malloc(s, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
            if (trk_ptrs[i] != nullptr)
            {
                Debug_printf("\nReading %d bytes of track %d into location %lu", s, i, trk_ptrs[i]);
                fseek(_media_fileh, trks[i].start_block * 512, SEEK_SET);
                fread(trk_ptrs[i], 1, s, _media_fileh);
                Debug_printf("\n%d, %d, %lu", trks[i].start_block, trks[i].block_count, trks[i].bit_count);
            }
            else
            {
                Debug_printf("\nNo RAM allocated!");
                return true;
            }
        }
    }
    return false;
}

#endif // BUILD_APPLE