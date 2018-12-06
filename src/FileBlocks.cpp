#include "pch.h"
#include "FileBlocks.h"

#include <algorithm>
#include <fstream>
#include <vector>

void IndexRecord::print(std::wostream &os) const
{
    os << type << ' ' << length << ' ' << fileNo << ' ' << offset << ' ' << lastBlockNo << ' '
        << name << '\0' << std::endl;
}

void IndexRecord::read(std::wistream &is)
{
    is >> type >> length >> fileNo >> offset >> lastBlockNo;
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

void DataBlock::read(std::istream &is)
{
	is.read(reinterpret_cast<char*>(&no), sizeof(no));
	is.read(reinterpret_cast<char*>(&origLength), sizeof(origLength));
	is.read(reinterpret_cast<char*>(&length), sizeof(length));
	data.resize(length);
	is.read(reinterpret_cast<char*>(&data[0]), length);
}

void DataBlock::write(std::ostream &os)
{
	os.write(reinterpret_cast<char*>(&no), sizeof(no));
	os.write(reinterpret_cast<char*>(&origLength), sizeof(origLength));
	os.write(reinterpret_cast<char*>(&length), sizeof(length));
	os.write(reinterpret_cast<char*>(&data[0]), length);
}

void IndexBlock::print(std::wostream &os) const
{
    os << no << ' ' << records.size() << '\n';
    for (const auto &r : records)
        os << r;
}

void IndexBlock::read(std::wistream &is)
{
    size_t recCount;
    is >> no >> recCount;
    records.resize(recCount);
    for (size_t i = 0; i < recCount; i++)
        is >> records[i];

    is >> std::ws;
}

std::wostream& operator<<(std::wostream &os, const IndexBlock &block)
{
    block.print(os);
    return os;
}

std::wistream& operator>>(std::wistream &is, IndexBlock &block)
{
    block.read(is);
    return is;
}
