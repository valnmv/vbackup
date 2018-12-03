#include "pch.h"
#include "FileBlocks.h"

void IndexRecord::print(std::wostream &os) const
{
    os << type << ' ' << length << ' ' << fileNo << ' ' << offset << ' '
        << blockCount << ' ' << name << '\0' << std::endl;
}

void IndexRecord::read(std::wistream &is)
{
    is >> type >> length >> fileNo >> offset >> blockCount;
    is.ignore();
    std::vector<wchar_t> buf;
    wchar_t ch;
    do
    {
        is.get(ch);
        buf.push_back(ch);
    } while (ch);

    name.assign(&buf[0]);
}

std::wostream& operator<<(std::wostream &os, const IndexRecord &rec)
{
    rec.print(os);
    return os;
}

std::wistream& operator>>(std::wistream &is, IndexRecord &rec)
{
    rec.read(is);
    return is;
}