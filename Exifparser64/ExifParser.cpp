#include "ExifParser.h"



struct GimbalPos
{
    double roll{ 0.0 };
    double yaw{ 0.0 };
    double pitch{ 0.0 };

    GimbalPos(std::string& xmp)
    {
        roll = get_rotation(xmp, "drone-dji:GimbalRoll");
        pitch = get_rotation(xmp, "drone-dji:GimbalPitch");
        yaw = get_rotation(xmp, "drone-dji:GimbalYaw");
    }

private:
    double get_float(std::string str)
    {
        auto pos_begin = str.find_first_of("\"");
        auto pos_end = str.find_first_of("\n");
        pos_end -= pos_begin + 2;
        if (pos_begin == std::string::npos || pos_end == std::string::npos)
        {
            //ERR
            return -999.0;
        }
        str = str.substr(pos_begin + 1, pos_end);
        return std::stod(str);
    }
    double get_rotation(std::string str, const std::string fstr)
    {
        auto pos = str.find(fstr);
        if (pos == std::string::npos)
        {
            //ERR
            return -999.0;
        }
        return get_float(str.substr(pos));
    }
};

static void error_(const char* err)
{
    fprintf(stderr, "%s", err);
    exit(0);
}

/**
* Funkcija lokalizuoti xmp duomenis baitu masyve ir juos perdaryti i string
* tolimesniam apdorojimui
* @param buffer baitu masyvas su xmp segmentu jpg faile
* @return GimbalPos klase su isextractintais roll,pitch,yaw
*/
static GimbalPos parse_xmp(uint8_t* buffer)
{
    int counter = 0;
    while (*buffer++ != '\n');

    while (buffer[counter] != '<' || buffer[counter + 1] != '/' || buffer[counter + 2] != 'x')
    {
        ++counter;
    }

    buffer[counter] = '\0';
    std::string xmp(reinterpret_cast<char*>(buffer));
    GimbalPos gp(xmp);
    return gp;
}

/**
* Funkcija nuskaityti EXIF segmentus
* @param f Ptr i atidaryta faila
* @return JpegSegment struktura su apdoroto segmento masyvu
*/
static JpegSegment* read_segment(FILE* f)
{
    uint32_t segment_info = 0;
    uint16_t segment_size = 0;
    uint8_t* buff = NULL;
    JpegSegment* return_segment = new JpegSegment;
    if(!return_segment) 
        error_("heap alloc failed");
    uint8_t first_block = (uint8_t)(ftell(f) == 0);
    fread(&segment_info, sizeof(uint32_t), 1, f);
    if ((segment_info & 0xFFFF) == JPEG_SOI || !first_block)
    {
        if (first_block)
        {
            segment_info = (segment_info >> 0x10);
            fread(&segment_size, sizeof(uint16_t), 1, f);
            if (segment_info == JPEG_APP0)
            {
                segment_size = (segment_size >> 8 & 0xFF) | (segment_size << 8 & 0xFF00);
                buff = (uint8_t*)malloc(segment_size * sizeof(uint8_t));
                if (!buff)
                {
                    error_("heap alloc failed");
                }
                else
                {
                    fread(buff, sizeof(uint8_t), segment_size - 2, f);
                    free(buff);
                }
                buff = NULL;
                fread(&segment_info, sizeof(uint32_t), 1, f);
                segment_size = segment_info >> 0x10;
                segment_info &= 0xFFFF;
            }
        }
        else
        {
            segment_size = segment_info >> 0x10;
            segment_info &= 0xFFFF;
        }
        segment_size = (segment_size >> 8 & 0xFF) | (segment_size << 8 & 0xFF00);

        
        if (segment_info == JPEG_APP1)
        {
            return_segment->s_size = segment_size;
            buff = (uint8_t*)malloc(segment_size * sizeof(uint8_t));
            if (!buff)
            {
                error_("heap alloc failed");
            }
            else
            {
                fread(buff, sizeof(uint8_t), segment_size - 2, f);
            }
        }
        else
        {
            fprintf(stderr, "Uknown segment %x", segment_info);
            return NULL;
        }
    }
    else
    {
        error_("Non jpeg marker");
        return NULL;
    }
    return_segment->data = buff;
    return return_segment;
}

/**
* Funkcija nuskaityti ifd lenteles irasus
* Detalesnis ifd lenteles strukturos aprasymas apacioje
* @param buffer Ptr i ifd lenteles pradzia
* @param B_ORDER baitu isdestymas, jei True -> Intel (little-endian); false Motorolla (big-endian)
* @return  Apdorotas IFD duomenu masyvas
*/
std::vector<IFD> get_ifd_table(uint8_t* buffer, bool B_ORDER)
{
    std::vector<IFD> ifd_table;
    uint16_t tag_count = *(reinterpret_cast<uint16_t*>(buffer));
    buffer += 2;
    if (!B_ORDER) tag_count = (tag_count >> 8 & 0xFF) | (tag_count << 8 & 0xFF00);

    while (tag_count--)
    {
        uint16_t tag_id = *(reinterpret_cast<uint16_t*>(buffer));
        if (!B_ORDER) tag_id = (tag_id >> 8 & 0xFF) | (tag_id << 8 & 0xFF00);
        buffer += 2;
        uint16_t val_type = *(reinterpret_cast<uint16_t*>(buffer));
        if (!B_ORDER) val_type = (val_type >> 8 & 0xFF) | (val_type << 8 & 0xFF00);
        buffer += 2;
        uint32_t num_of_val = *(reinterpret_cast<uint32_t*>(buffer));
        if (!B_ORDER)
        {
            num_of_val = (num_of_val >> 24 & 0xFF) | (num_of_val >> 8 & 0xFF00) |
                (num_of_val << 8 & 0xFF0000) | (num_of_val << 24 & 0xFF000000);
        }
        buffer += 4;
        uint32_t offset = *(reinterpret_cast<uint32_t*>(buffer));
        if (!B_ORDER)
        {
            offset = (offset >> 24 & 0xFF) | (offset >> 8 & 0xFF00) |
                (offset << 8 & 0xFF0000) | (offset << 24 & 0xFF000000);
        }
        buffer += 4;
        ifd_table.emplace_back(IFD(tag_id, val_type, num_of_val, offset));
    }
    return ifd_table;
}

// EXIF formato struktura
    //45 78 69 66 0 0 EXIF Header
    // 49 49 II ARBA 4D 4D MM byte order / init offset
    // 00 2A arba 2A 00  EXIF TIFF marker
    // 08 00 00 00 baitai offset pirmo IFD 

// 0F 00 IFD tagu skaicius
    // ===========================
    // 01 0F is the ID for the first tag in the first IFD
    // 00 02 is the type of the value(2 means it's an ASCII string)
    // 00 00 00 16 is the number of components
    // 00 00 01 B2
    //=============================
    // 00 00 00 00 optional pointer to next IFD (zero if no more)
    // DATA SEGMENT
// NEXT IFD

static GPS_DATA parse_exif(uint8_t* buffer, size_t size)
{
    int tag_count = 0;
    uint8_t* offset = NULL;
    uint8_t exif_header[] = { 0x45,0x78,0x69,0x66,0x00,0x00 };
    
    for (int i = 0; i < sizeof(exif_header); ++i)
    {
        if (*buffer++ != exif_header[i])
        {
            error_("BAD EXIF\n");
            return GPS_DATA();
        }
    }

    offset = buffer; // offset count start here
    bool INTEL_BYTE_ORDER = true;
    // Intel or Motorola
    if (*(reinterpret_cast<uint16_t*>(buffer)) == EXIF_II)
    {
        INTEL_BYTE_ORDER = true;
        //std::cout << "II\n";
    }
    else if (*(reinterpret_cast<uint16_t*>(buffer)) == EXIF_MM)
    {
        //std::cout << "MM\n";
        INTEL_BYTE_ORDER = false;
    }
    else
    {
        error_("BYTE ORDER UNKNOWN\n");
        return GPS_DATA();
    }

    buffer += 8; // skip TIFF markers
    auto ifd0 = get_ifd_table(buffer, INTEL_BYTE_ORDER);
    bool found_gps = false;
    // ifd0 nuskaitymas, kad gauti ptr i atskira GPS info ifd
    for (auto entry : ifd0)
    {
        if (entry.tag_id == GPS_IFD)
        {
            buffer = offset + entry.offset;
            found_gps = true;
            break;
        }
    }
    if (found_gps)
    {
        auto gps_ifd = get_ifd_table(buffer, INTEL_BYTE_ORDER);

        auto find_gps = [&](auto a, auto ifd)
        {
            for (auto i : ifd)
                if (i.tag_id == static_cast<uint16_t>(a))
                    return i;
            return IFD();
        };
        
        // Surasti lat/long/alt is gps info ifd
        auto ltt = find_gps(GPS_TAG::latitude, gps_ifd);
        auto lng = find_gps(GPS_TAG::longtitude, gps_ifd);
        auto alt = find_gps(GPS_TAG::altitude, gps_ifd);

        // Pagalbine funkcija apversti baitus
        auto shuffle_bytes = [](auto a)
        {
            a = (a >> 24 & 0xFF) | (a >> 8 & 0xFF00) |
                (a << 8 & 0xFF0000) | (a << 24 & 0xFF000000);
            return a;
        };

        // Funkcija isgauti laipsnius is RATIONAL duomenu tipo
        // RATIONAL struktura pvz:
        /*
         0x00 0x00 0x00 0x6A / 0x00 0x00 0x00 0x01 = 106 / 1 = 6
         0x00 0x00 0x12 0xFE / 0x00 0x00 0x00 0x64 = 4862 / 100 = 48.62
         0x00 0x00 0x00 0x00 / 0x00 0x00 0x00 0x01 = 0 / 1 = 0
         */
         // Laipsniu skaiciavimo formule: dd = d + m/60 + s/3600
        auto get_decimal = [&](uint32_t* b, bool byte_order)
        {
            if (!byte_order)
            {
                return (static_cast<double>(shuffle_bytes(*b)) / shuffle_bytes(*(b + 1)) ) +
                    (static_cast<double>(shuffle_bytes(*(b + 2))) / shuffle_bytes(*(b + 3))) / 60.0 +
                    (static_cast<double>(shuffle_bytes(*(b + 4))) / shuffle_bytes(*(b + 5))) / 3600.0;
            }
            return (static_cast<double>(*b) / *(b + 1)) +
                (static_cast<double>(*(b + 2)) / *(b + 3)) / 60.0 +
                (static_cast<double>(*(b + 4)) / *(b + 5)) / 3600.0;
        };
        auto get_decimal_alt = [&](uint32_t* b, bool byte_order)
        {
            if (!byte_order)
            {
                return static_cast<double>(shuffle_bytes(*b)) / shuffle_bytes(*(b + 1));
            }
            return static_cast<double>(*b) / *(b + 1);
        };

        auto latituded = get_decimal(reinterpret_cast<uint32_t*>(offset + ltt.offset),INTEL_BYTE_ORDER);
        auto longtituded = get_decimal(reinterpret_cast<uint32_t*>(offset + lng.offset), INTEL_BYTE_ORDER);
        auto altituded = get_decimal_alt(reinterpret_cast<uint32_t*>(offset + alt.offset), INTEL_BYTE_ORDER);

        // Vertes gali buti neigiamos priklausomai nuo krypties
        if ((find_gps(GPS_TAG::latituderef, gps_ifd).offset & 0xFF) == 'S')
            latituded = -latituded;
        if ((find_gps(GPS_TAG::longtituderef, gps_ifd).offset & 0xFF) == 'W')
            longtituded = -longtituded;
        if ((find_gps(GPS_TAG::altituderef, gps_ifd).offset & 0xFF) == 1)
            altituded = -altituded;

        GPS_DATA returns;
        returns.altitude = altituded;
        returns.latitude = latituded;
        returns.longtitude = longtituded;
        return returns;
    }
    return GPS_DATA();
}


EXIFPARSER CAMERA_DATA get_file_data(const wchar_t *path)
{
    JpegSegment* exif_buffer = NULL;
    JpegSegment* xmp_buffer = NULL;
    FILE* f = _wfopen(path, L"rb");
    if (!f)
    {
        error_("No file or memory");
        exit(0);
    }

    exif_buffer = read_segment(f);
    xmp_buffer = read_segment(f);
    fclose(f);
    if (!xmp_buffer || !exif_buffer) return {};

    auto gps_info = parse_exif(exif_buffer->data, exif_buffer->s_size);
    auto gimbal_info = parse_xmp(xmp_buffer->data);
    delete xmp_buffer;
    delete exif_buffer;

    CAMERA_DATA cd;
    cd.gps[GPS_EXPORT_INDEX::altitude] = gps_info.altitude;
    cd.gps[GPS_EXPORT_INDEX::latitude] = gps_info.latitude;
    cd.gps[GPS_EXPORT_INDEX::longtitude] = gps_info.longtitude;
    cd.pitch = gimbal_info.pitch;
    cd.yaw = gimbal_info.yaw;
    cd.roll = gimbal_info.roll;

    return cd;
}
