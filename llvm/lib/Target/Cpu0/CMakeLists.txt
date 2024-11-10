add_llvm_component_group(Cpu0)

set(LLVM_TARGET_DEFINITIONS Cpu0Other.td)

tablegen(LLVM Cpu0GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM Cpu0GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM Cpu0GenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM Cpu0GenMCPseudoLowering.inc -gen-pseudo-lowering)

add_public_tablegen_target(Cpu0CommonTableGen)

add_llvm_target(Cpu0CodeGen
    Cpu0TargetMachine.cpp

    LINK_COMPONENTS
    Analysis
    AsmPrinter
    CodeGen
    Core
    MC
    Cpu0Desc
    Cpu0Info
    SelectionDAG
    Support
    Target
    GlobalISel

    ADD_TO_COMPONENT
    Cpu0
)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
