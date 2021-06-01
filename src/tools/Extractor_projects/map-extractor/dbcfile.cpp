/**
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2021 MaNGOS <https://getmangos.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * World of Warcraft, and all World of Warcraft or Warcraft art, images,
 * and lore are copyrighted by Blizzard Entertainment, Inc.
 */

#include "dbcfile.h"
#undef min
#undef max
#include <mpq.h>

#include <cstdio>

using namespace std;

DBCFile::DBCFile(HANDLE handle, const std::string& filename):
    fileHandle(handle),filename(filename)

{

}

bool DBCFile::open()
{
    const static int BYTES = 4;
    unsigned char header[BYTES];
    unsigned int rcount, fcount, rsize, ssize;

    if (!SFileReadFile(fileHandle, header, BYTES, NULL, NULL))              // Magic header
    {
        SFileCloseFile(fileHandle);
        cout << "Could not read header in DBCFile " << filename << " err=" <<  GetLastError()<< endl;
        return false;
    }

    if (header[0] != 'W' || header[1] != 'D' || header[2] != 'B' || header[3] != 'C')
    {
        SFileCloseFile(fileHandle);
        cout << "The header in DBCFile " << filename << " did not match. err= " << GetLastError() << endl;
        return false;
    }

    if (!SFileReadFile(fileHandle, &rcount, BYTES, NULL, NULL))                 // Number of records
    {
        SFileCloseFile(fileHandle);
        cout << "Could not read number of records from DBCFile " << filename << ". err=" << GetLastError() << endl;
        return false;
    }

    if (!SFileReadFile(fileHandle, &fcount, BYTES, NULL, NULL))                 // Number of fields
    {
        SFileCloseFile(fileHandle);
        cout << "Could not read number of fields from DBCFile " << filename << ". err=" << GetLastError() << endl;
        return false;
    }

    if (!SFileReadFile(fileHandle, &rsize, BYTES, NULL, NULL))                 // Size of a record
    {
        SFileCloseFile(fileHandle);
        cout << "Could not read record size from DBCFile " << filename << ". err=" << GetLastError() << endl;
        return false;
    }

    if (!SFileReadFile(fileHandle, &ssize, BYTES, NULL, NULL))                 // String size
    {
        SFileCloseFile(fileHandle);
        cout << "Could not read string block size from DBCFile " << ". err=" << GetLastError() << endl;
        return false;
    }

    if (fcount * BYTES != rsize)
    {
        SFileCloseFile(fileHandle);
        cout << "Field count and record size in DBCFile " << filename << " do not match." << endl;
        return false;
    }

    recordSize = rsize;
    recordCount = rcount;
    fieldCount = fcount;
    stringSize = ssize;

    data = new unsigned char[recordSize * recordCount + stringSize];
    stringTable = data + recordSize * recordCount;

    size_t data_size = recordSize * recordCount + stringSize;

    if (!SFileReadFile(fileHandle, data, data_size, NULL, NULL))
    {
        SFileCloseFile(fileHandle);
        cout << "DBCFile " << filename << " did not contain expected amount of data for records." << endl;
        return false;
    }
    SFileCloseFile(fileHandle);

    return true;
}
DBCFile::~DBCFile()
{
    delete [] data;
}

DBCFile::Record DBCFile::getRecord(size_t id)
{
    assert(data);
    return Record(*this, data + id * recordSize);
}

size_t DBCFile::getMaxId()
{
    assert(data);

    size_t maxId = 0;
    for (size_t i = 0; i < getRecordCount(); ++i)
    {
        if (maxId < getRecord(i).getUInt(0))
        {
            maxId = getRecord(i).getUInt(0);
        }
    }
    return maxId;
}

DBCFile::Iterator DBCFile::begin()
{
    assert(data);
    return Iterator(*this, data);
}

DBCFile::Iterator DBCFile::end()
{
    assert(data);
    return Iterator(*this, stringTable);
}
