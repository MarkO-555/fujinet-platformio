#ifndef PRINTER_H
#define PRINTER_H
#include <Arduino.h>
#include <string.h>
#include <FS.h>

#include "sio.h"

#define EOL 155
#define BACKSLASH 92
#define LEFTPAREN 40
#define RIGHTPAREN 41
#define UPARROW 0xAD
#define DOWNARROW 0xAF
#define LEFTARROW 0xAC
#define RIGHTARROW 0xAE
#define BUFN 40

#define PLAIN 0
#define UNDERSCORE 0x0100
#define SYMBOL 0x0200
#define BOLD 0x0400
#define EMPHASIS 0x0800

enum printer_t
{
    A820,
    A822,
    A825,
    A1020,
    A1025,
    A1027,
    EMX80
};

enum paper_t
{
    RAW,
    TRIM,
    ASCII,
    PDF
};

class sioPrinter : public sioDevice
{
private:
    byte buffer[40];
    void sio_write();
    void sio_status() override;
    void sio_process() override;

    paper_t paperType = PDF;
    int pageWidth = 612;
    int pageHeight = 792;
    int leftMargin = 18;
    int bottomMargin = 6;
    int maxLines = 66;
    int maxCols = 80;
    float maxWidth = 12.0 * 80.0;
    float lineHeight = 12.0;
    float charWidth = 7.2;
    int fontNumber = 1;
    int fontSize = 12;
    const char *fontName = "Courier";
    float pdf_X = 0; // across the page - columns in pts
    bool BOLflag = true;
    float pdf_Y = 0; // down the page - lines in pts
    bool TOPflag = true;
    int voffset;
    int pageObjects[256];
    int pdf_pageCounter = 0;
    size_t objLocations[256]; // reference table storage
    int pdf_objCtr = 0;       // count the objects
    bool eolFlag = false;
    bool intlFlag = false;
    bool uscoreFlag = false;
    bool escMode = false;

    void pdf_header();
    void pdf_xref();
    void pdf_new_page();
    void pdf_end_page();
    void pdf_begin_text(int font, int fsize, int vpos);
    void pdf_set_font();
    void pdf_add_line(std::u16string L);
    void pdf_add(std::string output);
    size_t idx_stream_length; // file location of stream length indictor
    size_t idx_stream_start;  // file location of start of stream
    size_t idx_stream_stop;   // file location of end of stream

    void processBuffer(byte *B, int n);
    //std::u16string buffer_to_string(byte *S);
    //std::u16string output;
    std::string output;
    //int j;

    File *_file;

public:
    void initPrinter(File *f, paper_t ty);
    void initPrinter(File *f);
    void pageEject();
};

#endif // guard