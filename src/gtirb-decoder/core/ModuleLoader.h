//===- ModuleLoader.h -------------------------------------------*- C++ -*-===//
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
#ifndef SRC_GTIRB_DECODER_CORE_MODULELOADER_H_
#define SRC_GTIRB_DECODER_CORE_MODULELOADER_H_

#include <gtirb/gtirb.hpp>

#include "../DatalogProgram.h"
#include "../Relations.h"

// Load binary format information: architecture, file format, entry point, etc.
void ModuleLoader(const gtirb::Module& Module, DatalogProgram& Program);

const char* binaryISA(gtirb::ISA Arch);
const char* binaryFormat(const gtirb::FileFormat Format);

#endif // SRC_GTIRB_DECODER_CORE_MODULELOADER_H_
