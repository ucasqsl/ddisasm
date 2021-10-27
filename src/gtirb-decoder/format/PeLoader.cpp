//===- PeLoader.cpp ---------------------------------------------*- C++ -*-===//
//
//  Copyright (C) 2020 GrammaTech, Inc.
//
//  This code is licensed under the GNU Affero General Public License
//  as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version. See the
//  LICENSE.txt file in the project root for license terms or visit
//  https://www.gnu.org/licenses/agpl.txt.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU Affero General Public License for more details.
//
//  This project is sponsored by the Office of Naval Research, One Liberty
//  Center, 875 N. Randolph Street, Arlington, VA 22203 under contract #
//  N68335-17-C-0700.  The content of the information does not necessarily
//  reflect the position or policy of the Government and no official
//  endorsement should be inferred.
//
//===----------------------------------------------------------------------===//
#include "PeLoader.h"

#include "../../AuxDataSchema.h"

void PeSymbolLoader(const gtirb::Module &Module, DatalogProgram &Program)
{
    if(auto *ExportEntries = Module.getAuxData<gtirb::schema::ExportEntries>())
    {
        Program.insert("export_entry", *ExportEntries);
    }

    if(auto *ImportEntries = Module.getAuxData<gtirb::schema::ImportEntries>())
    {
        Program.insert("import_entry", *ImportEntries);
    }
}

void PeDataDirectoryLoader(const gtirb::Module &Module, DatalogProgram &Program)
{
    if(auto *DataDirectories = Module.getAuxData<gtirb::schema::PeDataDirectories>())
    {
        Program.insert("data_directory", *DataDirectories);
    }
    if(auto *DebugData = Module.getAuxData<gtirb::schema::PeDebugData>())
    {
        Program.insert("pe_debug_data", *DebugData);
    }
}

namespace souffle
{
    souffle::tuple &operator<<(souffle::tuple &t, const auxdata::PeExportEntry &ExportEntry)
    {
        auto &[Address, Ordinal, Name] = ExportEntry;
        t << Address << Ordinal << Name;
        return t;
    }

    souffle::tuple &operator<<(souffle::tuple &t, const auxdata::PeImportEntry &ImportEntry)
    {
        auto &[Address, Ordinal, Function, Library] = ImportEntry;
        t << Address << Ordinal << Function << Library;
        return t;
    }

    souffle::tuple &operator<<(souffle::tuple &t, const auxdata::PeDataDirectory &DataDirectory)
    {
        auto &[Type, Address, Size] = DataDirectory;
        t << Type << Address << Size;
        return t;
    }
} // namespace souffle
