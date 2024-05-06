import platform
import unittest
from disassemble_reassemble_check import compile, cd, disassemble
from pathlib import Path
import gtirb

from snippets import parse_souffle_output

ex_arm_asm_dir = Path("./examples/arm_asm_examples")


class ArmMiscTest(unittest.TestCase):
    @unittest.skipUnless(
        platform.system() == "Linux", "This test is linux only."
    )
    def test_arm_invalid_ldr(self):
        """
        Test ARM32 invalid load instructions are not disassembled as code.
        """
        binary = Path("ex")
        adder_dir = ex_arm_asm_dir / "ex_ldr"
        with cd(adder_dir):
            self.assertTrue(
                compile(
                    "arm-linux-gnueabihf-gcc",
                    "arm-linux-gnueabihf-g++",
                    "-O0",
                    [],
                    "qemu-arm -L /usr/arm-linux-gnueabihf",
                )
            )

            # collect the invalid symbols
            ir_library = disassemble(binary).ir()
            m = ir_library.modules[0]
            invalid_syms = [
                sym.name
                for sym in m.symbols
                if sym.name.startswith(".INVALID")
            ]
            extra_strip_flags = ["--keep-symbol=main"]
            for sym in invalid_syms:
                extra_strip_flags.append("--keep-symbol={}".format(sym))

            ir_library = disassemble(
                binary,
                Path("ex_stripped.gtirb"),
                strip_exe="arm-linux-gnueabihf-strip",
                strip=True,
                extra_strip_flags=extra_strip_flags,
            ).ir()
            m = ir_library.modules[0]

            main_first_block = None
            blocks_are_data = []
            for sym in m.symbols:
                if sym.name == "main":
                    main_first_block = sym.referent
                    continue
                if not sym.name.startswith(".INVALID"):
                    continue

                blocks_are_data.append(
                    isinstance(sym.referent, gtirb.DataBlock)
                )

            self.assertTrue(isinstance(main_first_block, gtirb.CodeBlock))
            self.assertTrue(all(blocks_are_data))
            self.assertEqual(len(blocks_are_data), len(invalid_syms))

    @unittest.skipUnless(
        platform.system() == "Linux", "This test is linux only."
    )
    def test_thumb_at_section_start(self):
        """
        Test that Thumb code is discovered at the start of a section
        """
        binary = Path("ex")
        with cd(ex_arm_asm_dir / "ex_thumb_at_section_start"):
            self.assertTrue(
                compile(
                    "arm-linux-gnueabihf-gcc",
                    "arm-linux-gnueabihf-g++",
                    "-O0",
                    [],
                    "qemu-arm -L /usr/arm-linux-gnueabihf",
                )
            )

            ir_library = disassemble(
                binary,
                strip_exe="arm-linux-gnueabihf-strip",
                strip=True,
            ).ir()

            m = ir_library.modules[0]

            section = next(s for s in m.sections if s.name == ".text")
            self.assertEqual(len(list(m.code_blocks_on(section.address))), 1)

    @unittest.skipUnless(
        platform.system() == "Linux", "This test is linux only."
    )
    def test_arm_ldcl(self):
        """
        Test that ldcl instructions, valid only on ARMv7 and earlier, decode
        correctly.
        """
        binary = Path("ex")
        adder_dir = ex_arm_asm_dir / "ex_ldcl"
        with cd(adder_dir):
            self.assertTrue(
                compile(
                    "arm-linux-gnueabihf-gcc",
                    "arm-linux-gnueabihf-g++",
                    "-O0",
                    [],
                    "qemu-arm -L /usr/arm-linux-gnueabihf",
                )
            )

            ir_library = disassemble(binary).ir()
            m = ir_library.modules[0]

            for sym in m.symbols:
                if sym.name == "main":
                    block = sym.referent
                    break

            self.assertIsInstance(block, gtirb.block.CodeBlock)
            # ensure main decoded as ARM, not Thumb.
            self.assertEqual(
                block.decode_mode, gtirb.CodeBlock.DecodeMode.Default
            )

            # Check auxdata
            archInfo = m.aux_data["archInfo"].data
            self.assertIn("Arch", archInfo)
            self.assertIn("Profile", archInfo)
            self.assertEqual(archInfo["Arch"], "v7")
            self.assertEqual(archInfo["Profile"], "Application")

    @unittest.skipUnless(
        platform.system() == "Linux", "This test is linux only."
    )
    def test_arm_dead_code_after_literal_pool(self):
        """
        Test that unreferenced ARM32 code after a literal pool is code.

        This is important in cases where there could be an indirect jump to the
        code that we did not detect.
        """

        binary = Path("ex")
        dir = ex_arm_asm_dir / "ex_code_after_literal_pool"
        with cd(dir):
            self.assertTrue(
                compile(
                    "arm-linux-gnueabihf-gcc",
                    "arm-linux-gnueabihf-g++",
                    "",
                    [],
                    "qemu-arm -L /usr/arm-linux-gnueabihf",
                )
            )

            ir_library = disassemble(
                binary,
                strip_exe="arm-linux-gnueabihf-strip",
                strip=True,
            ).ir()

            module = ir_library.modules[0]

            # ensure we have a block with the dead code:
            #   mov r1, r0          00 10 a0 e1
            #   ldr r0, [r1, #10]   0a 00 91 e5
            #   bx lr               1e ff 2f e1
            for block in module.code_blocks:
                block_contents = block.byte_interval.contents[
                    block.offset : block.offset + block.size
                ]
                if (
                    block_contents
                    == b"\x00\x10\xa0\xe1\x0a\x00\x91\xe5\x1e\xff\x2f\xe1"
                ):
                    break
            else:
                self.fail("Expected block not found")

    @unittest.skipUnless(
        platform.system() == "Linux", "This test is linux only."
    )
    def test_arm_known_block(self):
        """
        Ensure $a and $t symbols create known_block on ARM.
        """
        binary = Path("ex")
        dir = ex_arm_asm_dir / "ex_bx_pc"
        with cd(dir):
            self.assertTrue(
                compile(
                    "arm-linux-gnueabihf-gcc",
                    "arm-linux-gnueabihf-g++",
                    "",
                    [],
                    "qemu-arm -L /usr/arm-linux-gnueabihf",
                )
            )

            ir_library = disassemble(
                binary,
                extra_args=["--with-souffle-relations"],
            ).ir()
            module = ir_library.modules[0]

            known_blocks = parse_souffle_output(module, "known_block")

            reasons = set(kb[3] for kb in known_blocks)
            self.assertIn("$a", reasons)
            self.assertIn("$t", reasons)


if __name__ == "__main__":
    unittest.main()
