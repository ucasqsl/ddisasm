//===- Arm32Loader.h --------------------------------------------*- C++ -*-===//
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
#ifndef SRC_GTIRB_DECODER_ARCH_ARM32DECODER_H_
#define SRC_GTIRB_DECODER_ARCH_ARM32DECODER_H_

#include <map>
#include <string>

#include <capstone/capstone.h>

#include "../Relations.h"
#include "../core/InstructionLoader.h"

class Arm32Loader : public InstructionLoader
{
public:
    Arm32Loader() : InstructionLoader(4)
    {
        // Setup Capstone engine.
        [[maybe_unused]] cs_err Err = cs_open(CS_ARCH_ARM, CS_MODE_ARM, &CsHandle);
        assert(Err == CS_ERR_OK && "Failed to initialize ARM disassembler.");
        cs_option(CsHandle, CS_OPT_DETAIL, CS_OPT_ON);

        // Call cs_close when the last Arm32Loader is destroyed.
        CloseHandle.reset(new csh(CsHandle), cs_close);
    }

    void operator()(const gtirb::Module& Module, DatalogProgram& Program) override;

protected:
    using InstructionLoader::load;
    void load(const gtirb::ByteInterval& ByteInterval) override;
    void load(const gtirb::ByteInterval& ByteInterval, bool Thumb);

    void decode(const uint8_t* Bytes, uint64_t Size, uint64_t Addr) override;

private:
    size_t Increment;

    struct Arm32Facts;
    std::shared_ptr<Arm32Facts> Facts;

    std::optional<relations::Operand> build(const cs_arm_op& CsOp);
    std::optional<relations::Instruction> build(const cs_insn& CsInstruction);

    std::shared_ptr<csh> CloseHandle;
    csh CsHandle = CS_ERR_ARCH;
};

#endif // SRC_GTIRB_DECODER_ARCH_ARM32DECODER_H_
