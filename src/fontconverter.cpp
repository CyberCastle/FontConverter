/*
TrueType to Adafruit_GFX font converter.  Derived from Peter Jakobs'
Adafruit_ftGFX fork & makefont tool, and Paul Kourany's Adafruit_mfGFX.

NOT AN ARDUINO SKETCH.  This is a command-line tool for preprocessing
fonts to be used with the Adafruit_GFX Arduino library.

For UNIX-like systems.  Outputs to stdout; redirect to header file, e.g.:
  ./fontconvert ~/Library/Fonts/FreeSans.ttf 18 > FreeSans18pt7b.h

REQUIRES FREETYPE LIBRARY.  www.freetype.org

Currently this only extracts the printable 7-bit ASCII chars of a font.
Will eventually extend with some int'l chars a la ftGFX, not there yet.
Keep 7-bit fonts around as an option in that case, more compact.

See notes at end for glyph nomenclature & other tidbits.
*/

#include <cctype>
#include <ft2build.h>
#include <iostream>
#include FT_MODULE_H
#include FT_GLYPH_H
#include FT_TRUETYPE_DRIVER_H
#include "../include/gfxfont.h" // Adafruit_GFX font structures
#include "fontconverter.h"
#include "utils.h"

#define DPI 141 // Approximate res. of Adafruit 2.8" TFT

template <std::ostream *stream>
void FontConverter<stream>::fillFontInfo(std::string fontName, int fontSize, int bits, std::string fontStyle) {
    // Space and punctuation chars in name replaced w/ underscores, for C++ output.
    char c;
    if (this->outType == 0) {
        for (int i = 0; (c = fontName[i]); i++) {
            if (isspace(c) || ispunct(c))
                fontName[i] = '_';
        }
    } else {
        // Add font style to JSON output
        utils::replaceAll(this->templateToFill, "$fontStyle$", fontStyle);
    }

    utils::replaceAll(this->templateToFill, "$fontName$", fontName);
    utils::replaceAll(this->templateToFill, "$fontSize$", utils::intToStr(fontSize));
    utils::replaceAll(this->templateToFill, "$bits$", utils::intToStr(bits));
}

template <std::ostream *stream>
void FontConverter<stream>::fillBitmapInfo() {
    utils::replaceAll(this->templateToFill, "$bitmapData$", this->bitmapBuilder.str());
    this->bitmapBuilder.str(std::string());
}

template <std::ostream *stream>
void FontConverter<stream>::fillGlyphInfo() {
    utils::replaceAll(this->templateToFill, "$glyphData$", this->bitmapBuilder.str());
    this->bitmapBuilder.str(std::string());
}

template <std::ostream *stream>
void FontConverter<stream>::fillFontSpec(int firstChar, int lastChar, int yAdvance) {
    utils::replaceAll(this->templateToFill, "$firstChar$", utils::intToHex(firstChar));
    utils::replaceAll(this->templateToFill, "$lastChar$", utils::intToHex(lastChar));
    utils::replaceAll(this->templateToFill, "$yAdvance$", utils::intToStr(yAdvance));
}

// Accumulate bits for output, with periodic hexadecimal byte write
template <std::ostream *stream>
void FontConverter<stream>::enbit(uint8_t value) {
    if (value)
        sum |= bit;       // Set bit if needed
    if (!(bit >>= 1)) {   // Advance to next bit, end of byte reached?
        if (!firstCall) { // Format output table nicely
            if (++row >= 12) {
                // Last entry on line?
                this->bitmapBuilder << ",\n  "; // Newline format output
                row = 0;                        // Reset row counter
            } else {                            // Not end of line
                this->bitmapBuilder << ", ";    // Simple comma delim
            }
        }

        // Write byte value
        if (this->outType == 0) {
            // C++ output
            this->bitmapBuilder << utils::intToHex(unsigned(sum));
        } else {
            // JSON output
            this->bitmapBuilder << std::setw(3) << std::setfill(' ') << utils::intToStr(unsigned(sum));
        }
        sum = 0;       // Clear for next byte
        bit = 0x80;    // Reset bit counter
        firstCall = 0; // Formatting flag
    }
}

// Method for convert TrueType font to Adafruit_GFX font structures
template <std::ostream *stream>
int FontConverter<stream>::convert(char *fileName, int fontSize, int firstChar, int lastChar) {
    int i, j, err, bitmapOffset = 0, x, y, byte;
    FT_Library library;
    FT_Face face;
    FT_Glyph glyph;
    FT_Bitmap *bitmap;
    FT_BitmapGlyphRec *g;
    GFXglyph *table;
    uint8_t bit;

    if (lastChar < firstChar) {
        i = firstChar;
        firstChar = lastChar;
        lastChar = i;
    }

    // Allocate space glyph table
    if ((!(table = static_cast<GFXglyph *>(malloc((lastChar - firstChar + 1) *
                                                  sizeof(GFXglyph)))))) {
        (*stream) << "Malloc error" << std::endl;
        return 1;
    }

    // Init FreeType lib, load font
    if ((err = FT_Init_FreeType(&library))) {
        (*stream) << "FreeType init error: " << err << std::endl;
        return err;
    }

    // Use TrueType engine version 35, without subpixel rendering.
    // This improves clarity of fonts since this library does not
    // support rendering multiple levels of gray in a glyph.
    // See https://github.com/adafruit/Adafruit-GFX-Library/issues/103
    FT_UInt interpreter_version = TT_INTERPRETER_VERSION_35;
    FT_Property_Set(library, "truetype",
                    "interpreter-version",
                    &interpreter_version);

    if ((err = FT_New_Face(library, fileName, 0, &face))) {
        (*stream) << "Font load error: " << err << std::endl;
        FT_Done_FreeType(library);
        return err;
    }

    // Add font name, size and 7/8 bit.
    this->fillFontInfo(face->family_name, fontSize, (lastChar > 127) ? 8 : 7, face->style_name);

    // << 6 because '26dot6' fixed-point format
    FT_Set_Char_Size(face, fontSize << 6, 0, DPI, 0);

    // Currently all symbols from 'first' to 'last' are processed.
    // Fonts may contain WAY more glyphs than that, but this code
    // will need to handle encoding stuff to deal with extracting
    // the right symbols, and that's not done yet.
    // Process glyphs and output huge bitmap data array
    for (i = firstChar, j = 0; i <= lastChar; i++, j++) {
        // MONO renderer provides clean image with perfect crop
        // (no wasted pixels) via bitmap struct.
        if ((err = FT_Load_Char(face, i, FT_LOAD_TARGET_MONO))) {
            (*stream) << "Error " << err << " loading char '" << static_cast<char>(i) << "'" << std::endl;
            continue;
        }

        if ((err = FT_Render_Glyph(face->glyph,
                                   FT_RENDER_MODE_MONO))) {
            (*stream) << "Error " << err << " rendering char '" << static_cast<char>(i) << "'" << std::endl;
            continue;
        }

        if ((err = FT_Get_Glyph(face->glyph, &glyph))) {
            (*stream) << "Error " << err << " getting glyph '" << static_cast<char>(i) << "'" << std::endl;
            continue;
        }

        bitmap = &face->glyph->bitmap;
        g = reinterpret_cast<FT_BitmapGlyphRec *>(glyph);

        // Minimal font and per-glyph information is stored to
        // reduce flash space requirements.  Glyph bitmaps are
        // fully bit-packed; no per-scanline pad, though end of
        // each character may be padded to next byte boundary
        // when needed.  16-bit offset means 64K max for bitmaps,
        // code currently doesn't check for overflow.  (Doesn't
        // check that size & offsets are within bounds either for
        // that matter...please convert fonts responsibly.)
        table[j].bitmapOffset = bitmapOffset;
        table[j].width = bitmap->width;
        table[j].height = bitmap->rows;
        table[j].xAdvance = face->glyph->advance.x >> 6;
        table[j].xOffset = g->left;
        table[j].yOffset = 1 - g->top;

        for (y = 0; y < bitmap->rows; y++) {
            for (x = 0; x < bitmap->width; x++) {
                byte = x / 8;
                bit = 0x80 >> (x & 7);
                enbit(bitmap->buffer[y * bitmap->pitch + byte] & bit);
            }
        }

        // Pad end of char bitmap to next byte boundary if needed
        int n = (bitmap->width * bitmap->rows) & 7;
        if (n) {       // Pixel count not an even multiple of 8?
            n = 8 - n; // # bits to next multiple
            while (n--)
                enbit(0);
        }
        bitmapOffset += (bitmap->width * bitmap->rows + 7) / 8;

        FT_Done_Glyph(glyph);
    }

    // Fill template with bitmap array
    this->fillBitmapInfo();

    // Output glyph attributes table (one per character)
    for (i = firstChar, j = 0; i <= lastChar; i++, j++) {
        if (this->outType == 0) {
            // Braces for C++ output
            this->bitmapBuilder << "  { ";
        } else {
            // Square brackets for JSON output
            this->bitmapBuilder << "  [ ";
        }
        this->bitmapBuilder << std::setw(5) << std::setfill(' ') << unsigned(table[j].bitmapOffset);
        this->bitmapBuilder << ", " << std::setw(3) << std::setfill(' ') << unsigned(table[j].width);
        this->bitmapBuilder << ", " << std::setw(3) << std::setfill(' ') << unsigned(table[j].height);
        this->bitmapBuilder << ", " << std::setw(3) << std::setfill(' ') << unsigned(table[j].xAdvance);
        this->bitmapBuilder << ", " << std::setw(4) << std::setfill(' ') << static_cast<int>(table[j].xOffset);
        this->bitmapBuilder << ", " << std::setw(4) << std::setfill(' ') << static_cast<int>(table[j].yOffset);

        // Close braces
        if (i < lastChar) {
            if (this->outType == 0) {
                // Braces for C++ output
                this->bitmapBuilder << " }, ";
            } else {
                // Square brackets for JSON output
                this->bitmapBuilder << " ], ";
            }
        } else {
            if (this->outType == 0) {
                // Braces for C++ output
                this->bitmapBuilder << " }  ";
            } else {
                // Square brackets for JSON output
                this->bitmapBuilder << " ]  ";
            }
        }

        // Print character associated to glyph attributes (only for C++ output)
        if (this->outType == 0) {
            if ((i >= ' ') && (i <= '~')) {
                this->bitmapBuilder << "  // " << utils::intToHex(i) << " '" << static_cast<char>(i) << "'" << std::endl;
            } else {
                this->bitmapBuilder << "  // " << utils::intToHex(i) << std::endl;
            }
        } else {
            // Omit print character in JSON output
            this->bitmapBuilder << std::endl;
        }
    }

    // Fill template with glyph attributes table
    this->fillGlyphInfo();

    // Output font structure
    if (face->size->metrics.height == 0) {
        // No face height info, assume fixed width and get from a glyph.
        this->fillFontSpec(firstChar, lastChar, table[0].height);
    } else {
        this->fillFontSpec(firstChar, lastChar, face->size->metrics.height >> 6);
    }

    // Free resources
    FT_Done_FreeType(library);
    return 0;
}

template <std::ostream *stream>
const char *FontConverter<stream>::getCode() {
    size_t size = this->templateToFill.size();
    char *result = new char[size + 1];
    this->templateToFill.copy(result, size + 1);
    result[size] = '\0';
    this->templateToFill.clear();
    return result;
}

template class FontConverter<&std::cout>;
template class FontConverter<&std::cerr>;

/* -------------------------------------------------------------------------

Character metrics are slightly different from classic GFX & ftGFX.
In classic GFX: cursor position is the upper-left pixel of each 5x7
character; lower extent of most glyphs (except those w/descenders)
is +6 pixels in Y direction.
W/new GFX fonts: cursor position is on baseline, where baseline is
'inclusive' (containing the bottom-most row of pixels in most symbols,
except those with descenders; ftGFX is one pixel lower).

Cursor Y will be moved automatically when switching between classic
and new fonts.  If you switch fonts, any print() calls will continue
along the same baseline.

                    ...........#####.. -- yOffset
                    ..........######..
                    ..........######..
                    .........#######..
                    ........#########.
   * = Cursor pos.  ........#########.
                    .......##########.
                    ......#####..####.
                    ......#####..####.
       *.#..        .....#####...####.
       .#.#.        ....##############
       #...#        ...###############
       #...#        ...###############
       #####        ..#####......#####
       #...#        .#####.......#####
====== #...# ====== #*###.........#### ======= Baseline
                    || xOffset

glyph->xOffset and yOffset are pixel offsets, in GFX coordinate space
(+Y is down), from the cursor position to the top-left pixel of the
glyph bitmap.  i.e. yOffset is typically negative, xOffset is typically
zero but a few glyphs will have other values (even negative xOffsets
sometimes, totally normal).  glyph->xAdvance is the distance to move
the cursor on the X axis after drawing the corresponding symbol.

There's also some changes with regard to 'background' color and new GFX
fonts (classic fonts unchanged).  See Adafruit_GFX.cpp for explanation.
*/
